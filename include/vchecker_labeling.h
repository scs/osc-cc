/*******************************************************************************
* BA09_FS_THAM_2                                                               *
* 02/2009-05/2009                                                              *
* Martin Kurmann, Sandro Gort                                                  *
*                                                                              *
* Beschreibung: Regionen im Bild markieren                                     *
*******************************************************************************/ 


#ifndef REGION_LABELING_H
#define REGION_LABELING_H

#include <vchecker_global.h>

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
