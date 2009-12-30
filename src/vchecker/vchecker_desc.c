/*	OSC-CC, a generic open source vision library depending on the Oscar
	sofware framework.
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*******************************************************************************
* BA09_FS_THAM_2                                                               *
* 02/2009-05/2009                                                              *
* Martin Kurmann, Sandro Gort                                                  *
*                                                                              *
* Beschreibung: Deskriptoren um Objekte zu beschreiben                         *
*******************************************************************************/  

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vchecker/vchecker_desc.h>
/* Depends on vchecker_labeling. */
#include <vchecker/vchecker_labeling.h>


#define FFT_SIZE                    512
#define FFT_SUBSAMPLING_FACTOR      2
#define HOLE_BORDER                 5


uint8 contour[OSC_CAM_MAX_IMAGE_HEIGHT * OSC_CAM_MAX_IMAGE_WIDTH];

/*******************************************************************************
 * berechnet die Flaeche eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw             Bildausscnitt
 * 					width			Breite des Bildauschnittes
 * 					height			Hoehe des Bildausschnittes
 *                  bb              Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->area     Flaeche
 ******************************************************************************/
void getArea(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int i, j;
    long sum = 0;

    /* Flaeche bereits berechnet */
    if (descr->area) return;

    /* alle Vordergrundpixel aufsummieren */
    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            /* Vordergrundpixel aufsummieren */
            if (pBw[i * width + j]) ++sum;
        }
    }

    descr->area = sum;
}
/******************************************************************************/

/*******************************************************************************
 * berechnet den Umfang eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->perimeter    Umfang
 ******************************************************************************/
void getPerimeter(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    uint16 i, j;
    long perimeter, area, normal, diagonal;
    uint8 lut[] = {1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1};
    uint8 pattern;


    /* Umfang bereits berechnet */
    if (descr->perimeter) return;

    /* Zielbild initialisieren */
    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            contour[i * width + j] = 0;
        }
    }

    /* Erosion mit Maske:*/
    /* 0 1 0 */
    /* 1 X 1 */
    /* 0 1 0 */
    for (i = bb->y + 1; i < bb->y + bb->height - 1; ++i) {
        for (j = bb->x + 1; j < bb->x + bb->width - 1; ++j) {
            if (pBw[i * width + j]) {
                contour[(i - 1) * width + j] = 1;
                contour[i * width + j - 1] = 1;
                contour[i * width + j] = 1;
                contour[i * width + j + 1] = 1;
                contour[(i + 1) * width + j] = 1;
            }
        }
    }

    /* Erodiertes Bild minus Originalbild = Umfang */
    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            contour[i * width + j] -= pBw[i * width + j];
        }
    }

    /* Wenn diagonaler Pixelverlauf moeglich Eckpixel mit Lookup-Table loeschen */
    for (i = bb->y + 1; i < bb->y + bb->height - 1; ++i) {
        for (j = bb->x + 1; j < bb->x + bb->width - 1; ++j) {
            pattern = contour[(i - 1) * width + j] +
                    (contour[i * width + j - 1]*2) +
                    (contour[i * width + j + 1]*4) +
                    (contour[(i + 1) * width + j]*8);

            contour[i * width + j] = contour[i * width + j] * lut[pattern];
        }
    }

    /* Diagonale Verbindungen zaehlen */
    diagonal = 0;
    for (i = bb->y + 1; i < bb->y + bb->height - 1; ++i) {
        for (j = bb->x + 1; j < bb->x + bb->width - 1; ++j) {
            if (contour[i * width + j] == 1) {
                diagonal = diagonal + contour[(i - 1) * width + j - 1] +
                        contour[(i - 1) * width + j + 1] +
                        contour[(i + 1) * width + j - 1] +
                        contour[(i + 1) * width + j + 1];
            }
        }
    }

    /* pro Diagonalverbindung wurde 2 gezaehlt deshalb / 2 */
    diagonal = diagonal >> 1;

    /* Anzahl Pixel von Umfang berechnen */
    area = 0;
    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            if (contour[i * width + j]) ++area;
        }
    }
    /* Anzahl direkt und nicht diagonal verbundene Pixel bestimmen */
    normal = area - diagonal;

    /* Umfang berechnen, *14/10 ungefaehr Wurzel 2 */
    perimeter = normal + ((diagonal * 14) / 10);

    descr->perimeter = perimeter;
}
/******************************************************************************/

/*******************************************************************************
 * berechnet die Kompaktheit eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->compactness  Kompaktheit
 ******************************************************************************/
void getCompactness(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    long compactness, perimeter;

    if (!descr->area) getArea(pBw, width, height, descr, bb);
    if (!descr->perimeter) getPerimeter(pBw, width, height, descr, bb);

    perimeter = descr->perimeter * descr->perimeter; /* Umfang quadrieren */
    compactness = perimeter / descr->area;

    descr->compactness = compactness;
}
/******************************************************************************/

/*******************************************************************************
 * berechnet den Schwerpunkt eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->balancePoint Koordinaten des Schwerpunktes
 ******************************************************************************/
void getBalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int i, j;
    uint16 pixVal = 0;
    long xPos = 0, yPos = 0;

    if (descr->balancePoint[0] || descr->balancePoint[1]) return;

    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            if (pBw[i * width + j]) {
                /* alle Koordinaten der Vordergrundpixel aufsummieren */
                xPos += (j - bb->x);
                yPos += (i - bb->y);
                pixVal++; /* Pixel zaehlen */
            }
        }
    }

    xPos /= pixVal;
    yPos /= pixVal;

    /* Boundingboxposition addieren */
    xPos = xPos + bb->x;
    yPos = yPos + bb->y;

    descr->balancePoint[0] = xPos;
    descr->balancePoint[1] = yPos;
}
/******************************************************************************/

/*******************************************************************************
 * berechnet den Schwerpunkt der Kontur eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->balancePoint Koordinaten des Schwerpunktes
 ******************************************************************************/
void getContourBalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int i, j;
    uint16 pixVal = 0;
    long xPos = 0, yPos = 0;

    /* Konturschwerpunkt bereits berechnet */
    if (descr->contourBalancePoint[0] || descr->contourBalancePoint[1]) return;

    /* Kontur erzeugen */
    getPerimeter(pBw, width, height, descr, bb);


    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            if (contour[i * width + j]) {
                /* alle Koordinaten der Vordergrundpixel aufsummieren */
                xPos += (j - bb->x);
                yPos += (i - bb->y);
                pixVal++; /* Pixel zaehlen */
            }
        }
    }

    xPos /= pixVal;
    yPos /= pixVal;

    /* Boundingboxposition addieren */
    xPos = xPos + bb->x;
    yPos = yPos + bb->y;

    descr->contourBalancePoint[0] = xPos;
    descr->contourBalancePoint[1] = yPos;
}
/******************************************************************************/

/*******************************************************************************
 * Berechnet das Betragsspektrum eines Objektes
 * Alle Koordinaten der Kontur werden al komplexe Zahl Dargestellt (x + j*y)
 * Dann wird die komplexe FFT berechnet
 * als letztes werden 8 Betragspektren berechnet (die tiefsten 4 und die hoechsten
 * 4 (negative Frequenzen))  
 *
 * Parameter:       pBw                     Bildausschnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                      Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->fourierMagnitude Betragspektrum
 ******************************************************************************/
void getFourier(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int x, y, i, x0 = 0, y0 = 0;
    uint16 idx = 0;
    uint16 startPosX = 0xFFFF, startPosY = 0xFFFF, lastPosX = 0xFFFF, lastPosY = 0xFFFF;
    bool bufferFull = FALSE;
    int block_exp = 1; /* dummy variable, wird nicht gebraucht */
    uint8 supsamplingCounter = 0;
    static bool twiddleFlag = FALSE;

    static complex_fract16 twiddle_table[FFT_SIZE >> 1];
    complex_fract16 input[FFT_SIZE];
    complex_fract16 output[FFT_SIZE];


    /* wenn schon berechnet abbruch */
    for (i = 0; i < FOURIER_M; ++i) {
        if (descr->fourierMagnitude[i]) return;
    }

    /* Schwerpunkt berechnen */
    getBalancePoint(pBw, width, height, descr, bb);
    x0 = (int) descr->balancePoint[0];
    y0 = (int) descr->balancePoint[1];

    /* Kontur erzeugen */
    getPerimeter(pBw, width, height, descr, bb);

    /* Input Werte abfuellen */
    for (y = bb->y + 1; y < bb->y + bb->height - 1; ++y) {
        for (x = bb->x + 1; x < bb->x + bb->width - 1; ++x) {
            if (contour[y * width + x] && !bufferFull) {
                /* Startposition speichern damit nach einer Umrundung der Kontur
                 * abgebrochen werden kann */
                startPosX = x;
                startPosY = y;

                /* Buffer fuellen bis voll oder einaml Kontur umrunden */
                while (!bufferFull) {
                    /* Subsampling */
                    ++supsamplingCounter;
                    if (supsamplingCounter >= FFT_SUBSAMPLING_FACTOR) {
                        supsamplingCounter = 0;
                        /* Realteil ist Abstand in x-Richtung zum Mittelpunkt 
                           shift um 7 -> Q7.8 Format */
                        input[idx].re = (fract16) ((x - x0) << 7);
                        /* Imaginaerteil ist Abstand in y-Richtung zum Mittelpunkt 
                           shift um 7 -> Q7.8 Format */
                        input[idx].im = (fract16) ((y - y0) << 7);
                        ++idx;
                    }

                    /* N7 N8 N1 */
                    /* N6 xx N2 */
                    /* N5 N4 N3 */
                    if (contour[(y - 1) * width + x + 1] && !((lastPosX == x + 1) && (lastPosY == y - 1))) /* N1 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x + 1;
                        y = y - 1;
                    } else if (contour[y * width + x + 1] && !((lastPosX == x + 1) && (lastPosY == y))) /* N2 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x + 1;
                    }
                    else if (contour[(y - 1) * width + x] && !((lastPosX == x) && (lastPosY == y - 1))) /* N8 */ {
                        lastPosX = x;
                        lastPosY = y;
                        y = y - 1;
                    }
                    else if (contour[(y - 1) * width + x - 1] && !((lastPosX == x - 1) && (lastPosY == y - 1))) /* N7 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x - 1;
                        y = y - 1;
                    }
                    else if (contour[(y + 1) * width + x + 1] && !((lastPosX == x + 1) && (lastPosY == y + 1))) /* N3 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x + 1;
                        y = y + 1;
                    }
                    else if (contour[(y + 1) * width + x] && !((lastPosX == x) && (lastPosY == y + 1))) /* N4 */ {
                        lastPosX = x;
                        lastPosY = y;
                        y = y + 1;
                    }
                    else if (contour[(y + 1) * width + x - 1] && !((lastPosX == x - 1) && (lastPosY == y + 1))) /* N5 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x - 1;
                        y = y + 1;
                    }
                    else if (contour[y * width + x - 1] && !((lastPosX == x - 1) && (lastPosY == y))) /* N6 */ {
                        lastPosX = x;
                        lastPosY = y;
                        x = x - 1;
                    }
                    else {
                        bufferFull = TRUE;
                    }

                    if (((startPosX == x) && (startPosY == y)) || (idx >= FFT_SIZE)) {
                        bufferFull = TRUE;
                    }
                }
                if (bufferFull) {
                    for (i = idx; i < FFT_SIZE; ++i) {
                        input[i].re = 0;
                        input[i].im = 0;
                    }
                    break;
                }
            }
            if (bufferFull) break;
        }
        if (bufferFull) break;
    }

    /* Das erste Mal Twiddle Table fuer FFT berechnen */
    if (!twiddleFlag) OscDspl_twidfftrad2_fr16(twiddle_table, FFT_SIZE);

    /* FFT berechnen */
    OscDspl_cfft_fr16(input, output, twiddle_table, 1, (int) FFT_SIZE, &block_exp, 1);

    /* Betrag der Komplexen-Fouriertransformierten berechnen */
    for (x = 0; x < (FOURIER_M >> 1); ++x) {
        descr->fourierMagnitude[x] = (long) (OscDspl_cabs_fr16(output[FFT_SIZE - (FOURIER_M >> 1) + x]) & 0x0000FFFF); /* negative Frequenzen */
        descr->fourierMagnitude[(FOURIER_M >> 1) + x] = (long) (OscDspl_cabs_fr16(output[x + 1]) & 0x0000FFFF);
    }
}
/******************************************************************************/

/*******************************************************************************
 * bestimmt wieviele Loecher ein Objekt enthaelt
 *
 * Parameter:       pBw                  Bildausschnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                   Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->numberOfHoles Anzahl Loecher im Objekt
 ******************************************************************************/
void numberOfHoles(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int i, j;
    uint8 p[OSC_CAM_MAX_IMAGE_HEIGHT * OSC_CAM_MAX_IMAGE_WIDTH];
    long holes = 0;
    struct G_BOUNDINGBOX noHolesBB[G_MAX_NO_OF_BOUNDING_BOXES];

    /* wenn bereits berechnet */
    if (descr->numberOfHoles) return;

    /* Bild invertieren Damit Loecher Wert "1" und Objekt Wert "0" */
    for (i = 0; i < height; ++i) {
        for (j = 0; j < height; ++j) {
            p[i * width + j] = 1 - pBw[i * width + j];
        }
    }
    
    /* Regioenmarkierung im invertierten Bild */
    regionLabeling(p, width, height, noHolesBB);

    for (i = 0; i < G_MAX_NO_OF_BOUNDING_BOXES; ++i) {
        if (bb[i].label) {
            /* ist Loch innerhalb des Objektes */
            if (noHolesBB[i].label) {
                if ((noHolesBB[i].x > (bb->x+HOLE_BORDER) &&
                        (noHolesBB[i].y > (bb->y+HOLE_BORDER)) &&
                        (noHolesBB[i].width < (bb->width-2*HOLE_BORDER)) &&
                        (noHolesBB[i].height < (bb->height-2*HOLE_BORDER)))) {
                    ++holes;
                }
            }
        }
    }

    descr->numberOfHoles = holes;
}
/******************************************************************************/

/*******************************************************************************
 * Berechnet den mittleren Abstand zum Schwerpunkt nach Pythagoras
 * c = sqrt(a^2,b^2)
 * Die wurzel wird jedoch nicht gezogen damit nicht mit Floatingpoint-Zahlen 
 * gerechnet werden muss.
 *
 * Parameter:       pBw                     Bildausschnitt
 * 					width					Breite des Bildauschnittes
 * 					height					Hoehe des Bildausschnittes
 *                  bb                      Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->meanDistance2BP  mittlerer Abstand zum Schwerpunkt
 ******************************************************************************/
void meanDistance2BalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb) {
    int i, j, x0 = 0, y0 = 0;
    uint16 pixVal = 0;
    long mean = 0;

    /* Konturschwerpunkt bereits berechnet */
    if (descr->meanDistance2BP) return;

    /* Kontur erzeugen */
    getPerimeter(pBw, width, height, descr, bb);

    /* Schwerpunkt berechnen */
    getBalancePoint(pBw, width, height, descr, bb);
    x0 = (int) descr->balancePoint[0];
    y0 = (int) descr->balancePoint[1];


    /* Alle Distanzen zum Schwerpunkt aufaddieren */
    for (i = bb->y; i < bb->y + bb->height; ++i) {
        for (j = bb->x; j < bb->x + bb->width; ++j) {
            if (contour[i * width + j]) {
                mean += (j - x0)*(j - x0) + (i - y0)*(i - y0);
                ++pixVal;
            }
        }
    }

    /* durch Anzahl Konturpixel teilen */
    mean /= pixVal;
    descr->meanDistance2BP = mean;
}
/******************************************************************************/
