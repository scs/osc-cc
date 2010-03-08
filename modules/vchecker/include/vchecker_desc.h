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

#ifndef DESCR_H
#define DESCR_H

#include "vchecker_global.h"

/* Struktur um Deskriptoren Werte zu speichern */
struct G_DESCRIPTOR {
    /* Flaeche des Objekts*/
    long area;
    /* Umfang des Objekts*/
    long perimeter;
    /* Kompaktheit des Objekts*/
    long compactness;
    /* Schwerpunkt des Objekts*/
    long balancePoint[2];
    /* Schwerpunkt der Objektkontur*/
    long contourBalancePoint[2];
    /* Anzahl Loecher im Objekt */
    long numberOfHoles;
    /* mittlere Distanz zum Schwerpunkt */
    long meanDistance2BP;
    /* Fourier Betragsspektrum der Kontur */
    long fourierMagnitude[FOURIER_M];
};

/* public functions */
/*******************************************************************************
 * berechnet die Flaeche eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw             Bildausscnitt
 * 					width			Breite des Bildauschnittes
 * 					height			Hoehe des Bildausschnittes
 *                  bb              Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->area     Flaeche
 ******************************************************************************/
void getPerimeter(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * berechnet den Umfang eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->perimeter    Umfang
 ******************************************************************************/
void getArea(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * berechnet die Kompaktheit eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->compactness  Kompaktheit
 ******************************************************************************/
void getCompactness(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * berechnet den Schwerpunkt eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->balancePoint Koordinaten des Schwerpunktes
 ******************************************************************************/
void getBalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * berechnet den Schwerpunkt der Kontur eines Objekts in der uebergebenen Region
 *
 * Parameter:       pBw                 Bildausscnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                  Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->balancePoint Koordinaten des Schwerpunktes
 ******************************************************************************/
void getContourBalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * Berechnet das Betragsspektrum eines Objektes
 * Alle Koordinaten der Kontur werden al komplexe Zahl Dargestellt (x + j*y)
 * Dann wird die komplexe FFT berechnet
 * als letztes werden 32 Betragspektren berechnet (die tiefsten 16 und die hoechsten
 * 16)  
 *
 * Parameter:       pBw                  Bildausschnitt
 * 					width				Breite des Bildauschnittes
 * 					height				Hoehe des Bildausschnittes
 *                  bb                   Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  
 ******************************************************************************/
void getFourier(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * bestimmt wieviele Loecher ein Objekt enthaelt
 *
 * Parameter:       pBw                  Bildausschnitt
 * 					width				 Breite des Bildauschnittes
 * 					height				 Hoehe des Bildausschnittes
 *                  bb                   Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->numberOfHoles Anzahl Loecher im Objekt
 ******************************************************************************/
void numberOfHoles(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);


/*******************************************************************************
 * berechnet den mittleren Abstand zum Schwerpunkt
 *
 * Parameter:       pBw                     Bildausschnitt
 * 					width					Breite des Bildauschnittes
 * 					height					Hoehe des Bildausschnittes
 *                  bb                      Boundingbox in dem sich Teil befindet
 * Rueckgabewerte:  descr->meanDistance2BP  mittlerer Abstand zum Schwerpunkt
 ******************************************************************************/
void meanDistance2BalancePoint(uint8 *pBw, uint32 width, uint32 height, struct G_DESCRIPTOR *descr, struct G_BOUNDINGBOX *bb);
#endif 
 
