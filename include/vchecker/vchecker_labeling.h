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


#ifndef REGION_LABELING_H
#define REGION_LABELING_H

#include "vchecker_global.h"

/* Definitionen */
#define RL_MAX_COLLISION_VECTOR_LEN     512 /* maximale Laenge des Kollisionsvektors */

#define MIN_BOUNDINGBOX_AREA            500

#define BB_BORDER_SIZE_PIX      5

/* public functions */
/*******************************************************************************
 * Regionen in Binaerbild finden und das Bounding-Rectangle darum bestimmen
 *
 * Parameter:       pBw    S/W-Bild
 * 					width			Breite des Bildauschnittes
 * 					height			Hoehe des Bildausschnittes
 *                  *bb    Pointer auf struct Bounding Box
 * Rueckgabewerte:  die gefundenen Boundingboxen werden in bb gespeichert
 ******************************************************************************/
void regionLabeling(uint8 *pBw, uint32 width, uint32 height, struct G_BOUNDINGBOX *bb);

#endif 
