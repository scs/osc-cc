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
* Beschreibung: Regionen im Bild markieren                                     *
*******************************************************************************/


#include <vchecker/vchecker_labeling.h>
#include <stdio.h>

/*******************************************************************************
 * sichert die Kollisionen zwischen Labels und sichert sie in einer Liste
 *
 * Parameter:       c1      Label 1
 *                  c2      Label 2   
 *                  cVec    Kollisionarray (Liste)
 * Rueckgabewerte:  cVec wird veraendert
 ******************************************************************************/
void collision(uint16 c1, uint16 c2, uint16 cVec[RL_MAX_COLLISION_VECTOR_LEN][2]) {
    int i;
    uint16 temp;
    /* Kollisionswerte nach groesse ordnen */
    /* kleinerer Wert = Ziellabel */
    /* groesserer Wert = Sourcelabel */
    if (c2 < c1) {
        temp = c1;
        c1 = c2;
        c2 = temp;
    }

    for (i = 0; i < RL_MAX_COLLISION_VECTOR_LEN; ++i) {
        if ((cVec[i][0] == c1) && (cVec[i][1] == c2)) return; /* Eintrag vorhanden */
        else if (cVec[i][1] == c1) c1 = cVec[i][0]; /*Eintrag mit kleinerem Ziellabel vorhanden */
        else if (cVec[i][1] == c2) /* Sourcelabel bereits vorhanden */ {
            if (cVec[i][0] >= c1) /* gesichertes Ziellabel groesser als neues Ziellabel */ {
                cVec[i][0] = c1; /* kleineres Ziellabel sichern */
            } else /* gesichertes Ziellabel kleiner als neues Ziellabel */ {
                /* registrieren, dass c2 auch auf bereits gesichertes kleineres
                 * Ziellabel abgebildet wird, moegliche Fehler dadurch werden
                 * durch Funktion cleaningCollisionVector(...) aufgeloest */
                c2 = c1;
                c1 = cVec[i][0];
            }
        }

        if (cVec[i][0] == 0) /* neuer Eintrag in Kollisionstabelle */ {
            cVec[i][0] = c1;
            cVec[i][1] = c2;
            return;
        }
    }
}
/******************************************************************************/

/*******************************************************************************
 * ordnet die Liste mit den gefundenen Kollisionen
 *
 * Parameter:       cVec    Kollisionarray
 * Rueckgabewerte:  cVec wird veraendert
 ******************************************************************************/
void cleaningCollisionVector(uint16 cVec[RL_MAX_COLLISION_VECTOR_LEN][2]) {
    int i, j;
    uint16 s[2]; /* Sucharray */

    for (i = 0; i < RL_MAX_COLLISION_VECTOR_LEN; ++i) {
        s[0] = cVec[i][0];
        s[1] = cVec[i][1];
        for (j = 0; j < RL_MAX_COLLISION_VECTOR_LEN; ++j) {
            if (cVec[j][0] == s[1]) cVec[j][0] = s[0]; /* kleineres Ziellabel gefunden */
            if (cVec[j][0] == 0) break; /* Ende */
        }
        if (cVec[i][0] == 0) break; /* Ende */
    }
}
/******************************************************************************/

/*******************************************************************************
 * Erster Schritt der Regionenmarkierung. Die Regionen werden temporaer markiert
 * und Kollisionen zwischen Regionen die zusammengehoeren werden gespeichert
 * und koennen spaeter aufgeloest werden
 *
 * Parameter:       pBw                 S/W-Bild
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  collisionVector     Array in dem Kollisionen zwischen den Labels festgehalten werden
 * Rueckgabewerte:  collisionVector wird veraendert
 ******************************************************************************/
void tempMarking(uint8 *pBw, uint32 width, uint32 height, uint16 collisionVector[RL_MAX_COLLISION_VECTOR_LEN][2]) {
    int i, j;
    uint16 idx = 2; /* label Index */

    /* Maske :  */
    /* N2 N3 N4 */
    /* N1 XX -- */
    /* -- -- -- */
    for (i = 1; i < height; ++i) {
        for (j = 1; j < width - 1; ++j) {
            if (pBw[i * width + j] > 0) /* Vordergrund Pixel */ {
                if (pBw[(i - 1) * width + j + 1] > 1) /* N4 bereits markiert */ {
                    pBw[i * width + j] = pBw[(i - 1) * width + j + 1];
                } else if (pBw[(i - 1) * width + j] > 1) /* N3 bereits markiert */ {
                    pBw[i * width + j] = pBw[(i - 1) * width + j];
                } else if (pBw[(i - 1) * width + j - 1] > 1) /* N2 bereits markiert */ {
                    pBw[i * width + j] = pBw[(i - 1) * width + j - 1];
                } else if (pBw[i * width + j - 1] > 1) /* N1 bereits markiert */ {
                    pBw[i * width + j] = pBw[i * width + j - 1];
                } else /* neues Label */ {
                    pBw[i * width + j] = idx;
                    idx++;
                }

                /* Kollision dedektieren */
                if ((pBw[i * width + j - 1] > 0) && (pBw[i * width + j - 1] != pBw[i * width + j])) {
                    collision(pBw[i * width + j - 1], pBw[i * width + j], collisionVector);
                }
            }
        }
    }
    /* Kollisionsvektor aufraeumen */
    cleaningCollisionVector(collisionVector);
}
/******************************************************************************/

/*******************************************************************************
 * Lookup-Table generieren die festlegt welche Quellabel auf welches Ziellabel
 * abgebildet wird
 *
 * Parameter:       cVec    Kollisionsvektor
 *                  lut     Lookup-Table Array
 * Rueckgabewerte:  lut     Lookuptabelle Resultat
 ******************************************************************************/
void generateLookUpTable(uint16 cVec[RL_MAX_COLLISION_VECTOR_LEN][2], uint16 lut[RL_MAX_COLLISION_VECTOR_LEN]) {
    int i;

    for (i = 0; i < RL_MAX_COLLISION_VECTOR_LEN; ++i) {
        lut[i] = i; /* bildet sich auf sich selber ab */
    }

    for (i = 0; i < RL_MAX_COLLISION_VECTOR_LEN; ++i) {
        if (cVec[i][0] > 0) /* bis Listenende */ {
            lut[cVec[i][1]] = cVec[i][0]; /* wird auf Ziellabel abgebildet */
        } else {
            lut[1] = 0; /* wenn noch 1en vorhanden auf Null abbilden */
            return;
        }
    }
}
/******************************************************************************/

/*******************************************************************************
 * Zweiter Schritt des Region Labeling. Mit Hilfe einer Lookup-Tabelle werden
 * Kollisionen zwischen den Regionen aufgeloest.
 *
 * Parameter:       pBw             Bild mit den im ersten durchlauf "gelabelten" Regionen
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  collisionVector Array in dem Kollisionen aus erstem durchlauf gespeichert wurden
 *                  *bb             Pointer auf struct Bounding Box
 * Rueckgabewerte:  bb              Bounding Boxen werden in bb gesichert
 ******************************************************************************/
void solveCollisions(uint8 *pBw, uint32 width, uint32 height,
        uint16 collisionVector[RL_MAX_COLLISION_VECTOR_LEN][2],
        struct G_BOUNDINGBOX *bb) {
    int i, j, k;
    uint16 lut[RL_MAX_COLLISION_VECTOR_LEN]; /* Lookup-Tabelle */
    uint16 x2Temp = 0, y2Temp = 0;

    generateLookUpTable(collisionVector, lut); /* Lookup-Tabelle generieren */

    /* Kollisionen aufloesen, jeder Pixel wird auf den in der Lookup-Tabelle
     * festgehaltenen Wert abgebildet, gleichzeitig werden die Koordinaten
     * der Bounding Box bestimmt;*/
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            pBw[i * width + j] = lut[pBw[i * width + j]]; /* Wert auf Zielwert abbilden */
            if (pBw[i * width + j] > 1) {
                for (k = 0; k < G_MAX_NO_OF_BOUNDING_BOXES; ++k) {
                    if (bb[k].label == pBw[i * width + j]) { /* Bounding Box besteht schon, pruefen ob groesse geaendert werden muss */
                        if (j < bb[k].x) bb[k].x = j; /* neue x-Position ? */
                        if (i < bb[k].y) bb[k].y = i; /* neue y-Position ? */
                        if (j > x2Temp) x2Temp = j;
                        if (i > y2Temp) y2Temp = i;
                        break;
                    } else if (bb[k].label == 0) /* Beginn neue Bounding Box */ {
                        bb[k].label = pBw[i * width + j];
                        bb[k].x = j;
                        bb[k].y = i;
                        bb[k].width = 0;
                        bb[k].height = 0;
                        break;
                    }
                } /* k */
            } /* if(pBw[i*w+j] > 1) */
        } /* j */
    } /* i */

    for (k = 0; k < G_MAX_NO_OF_BOUNDING_BOXES; ++k) {
        /* Laenge und Hoehe berechnen */
        if (bb[k].label) {
            bb[k].width = x2Temp - bb[k].x;
            bb[k].height = y2Temp - bb[k].y;
        }
    }
}
/******************************************************************************/

/*******************************************************************************
 * Regionen in Binaerbild finden und das Bounding-Rectangle darum bestimmen
 *
 * Parameter:       pBw    S/W-Bild
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  *bb   				Pointer auf struct Bounding Box
 * Rueckgabewerte:  die gefundenen Boundingboxen werden in bb gespeichert
 ******************************************************************************/
void regionLabeling(uint8 *pBw, uint32 width, uint32 height, struct G_BOUNDINGBOX *bb) {
    int i, j, k;
    /* Kollisionsvektor, pro Zeile zwei labels di kollidiert sind */
    uint16 collisionVector[RL_MAX_COLLISION_VECTOR_LEN][2];

    /* Kollisionsvektor initialisieren */
    for (i = 0; i < RL_MAX_COLLISION_VECTOR_LEN; ++i) {
        collisionVector[i][0] = 0;
        collisionVector[i][1] = 0;
    }

    /* boundingbox leeren */
    for (i = 0; i < G_MAX_NO_OF_BOUNDING_BOXES; ++i) {
        bb[i].label = 0;
    }

    /* Schritt 1: Vorlaeufige Markierung der Regionen */
    tempMarking(pBw, width, height, collisionVector);

    /* Schritt 2: Aufloesung der Kollisione */
    solveCollisions(pBw, width, height, collisionVector, bb);

    /* Rahmen um Bounding Box setzten */
    for (i = 0; i < G_MAX_NO_OF_BOUNDING_BOXES; ++i) {
        if (bb[i].label) {
            /* x Position anpassen */
            if (bb[i].x > BB_BORDER_SIZE_PIX) bb[i].x = bb[i].x - BB_BORDER_SIZE_PIX;
            else bb[i].x = 0;
            /* y Position anpassen */
            if (bb[i].y > BB_BORDER_SIZE_PIX) bb[i].y = bb[i].y - BB_BORDER_SIZE_PIX;
            else bb[i].y = 0;
            /* Breite anpassen */
            if (bb[i].x + bb[i].width + 2 * BB_BORDER_SIZE_PIX < width) bb[i].width = bb[i].width + 2 * BB_BORDER_SIZE_PIX;
            else bb[i].width = width - bb[i].x;
            /* Hoehe anpassen */
            if (bb[i].y + bb[i].height + 2 * BB_BORDER_SIZE_PIX < height) bb[i].height = bb[i].height + 2 * BB_BORDER_SIZE_PIX;
            else bb[i].height = height - bb[i].y;
        }
    }

    for (k = 0; k < G_MAX_NO_OF_BOUNDING_BOXES; ++k) {
        /* minimale Flaeche das ein Objekt haben muss um als Objekt erkannt zu werden */
        if (bb[k].label && ((bb[k].width * bb[k].height) < MIN_BOUNDINGBOX_AREA)) {
            bb[k].label = 0;
        } 
        else if (bb[k].label) {
            for (i = bb[k].y; i < bb[k].y + bb[k].height; ++i) {
                for (j = bb[k].x; j < bb[k].x + bb[k].width; ++j) {
                    if (pBw[i * width + j]) {
                        pBw[i * width + j] = 1;
                    } else {
                        pBw[i * width + j] = 0;
                    }
                }
            }
        }
    }

    /* Debug Meldung der gefundenen Boundingboxen */
#if(RL_DEBUG_MSG == ON)
    for (i = 0; i < G_MAX_NO_OF_BOUNDING_BOXES; ++i) {
        if (bb[i].label) {
            fprintf(stdout, "Bounding Box %d: \n x = %d\n y = %d\n width = %d\n height = %d\n",
                    i, bb[i].x, bb[i].y, bb[i].width, bb[i].height);
        }
    }
#endif
}
/******************************************************************************/

