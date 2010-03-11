
/*	LeanCV, a computer vision library for the LeanXcam and IndXcam.
	Copyright (C) 2010 Supercomputing Systems AG
	
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

/*! @file
 * @brief API definition for LeanCV Library
 * 
 */

#ifndef LEANCV_INCLUDE_LEANCV_H_
#define LEANCV_INCLUDE_LEANCV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include the opencv header for image type */
#include <opencv/cv.h>

#define SKIP_INT64_TYPEDEF
#include <oscar.h>


void lcvError(const char* fmt, ...);


/* Creates IPL image (header and data) */
IplImage*  lcvCreateImage( CvSize size, int depth, int channels );


/* Allocates and initializes IplImage header */
IplImage*  lcvCreateImageHeader( CvSize size, int depth, int channels );

/* Releases IPL image header and data */
void  lcvReleaseImage( IplImage** image );

/* Releases (i.e. deallocates) IPL image header */
void  lcvReleaseImageHeader( IplImage** image );


/* Calculates an image derivative using generalized Sobel
   (aperture_size = 1,3,5,7) or Scharr (aperture_size = -1) operator.
   Scharr can be used only for the first dx or dy derivative */
void lcvSobel( const CvArr* src, CvArr* dst,
                    int xorder, int yorder,
                    int aperture_size CV_DEFAULT(3));





#ifdef __cplusplus
}
#endif

#endif /* LEANCV_INCLUDE_LEANCV_H_ */







