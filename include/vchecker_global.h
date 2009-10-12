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
#ifndef VCHECKER_GLOBAL_H_
#define VCHECKER_GLOBAL_H_

#include <oscar.h>

struct G_BOUNDINGBOX {
    uint16 label;
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
};

/* Anzahl Fourierkoeffizienten Amplitudenspektrum fuer Kontourbeschreibung*/
#define FOURIER_M                       4

#define G_MAX_NO_OF_BOUNDING_BOXES      10

#endif /* VCHECKER_GLOBAL_H_ */
