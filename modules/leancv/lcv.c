
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

#include <leancv.h>

#include <dspl.h> //from oscar

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


void lcvError_m(const char* func, const char* file, int line, const char* fmt, ...) {
	
	va_list va;
	char buf[1024], buf2[1024];
	
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	
	sprintf(buf2, "LeanCV Error: %s in %s, file %s, line %d\n"
		, buf, func, file, line);
	fprintf(stderr, "%s\n", buf2);
	fflush(stderr);
	
	abort();
}


IplImage*  lcvCreateImage( CvSize size, int depth, int channels ) {
	
	IplImage *img = lcvCreateImageHeader( size, depth, channels );
	
	img->imageDataOrigin = (char*)malloc(img->imageSize+LCV_IMG_ALIGN);
	img->imageData=lcvAlignImage(img->imageDataOrigin);
	
	return(img);
}


IplImage*  lcvCreateImageHeader( CvSize size, int depth, int channels ) {
	
	if( size.width < 0 || size.height < 0 ) lcvError("Image Size < 0");
	if(size.width % LCV_WIDTH_ALIGN != 0) lcvError("Image Width must be a multiple of %i", LCV_WIDTH_ALIGN);
	
	IplImage* image = malloc(sizeof(IplImage));
	 
	memset( image, 0, sizeof( *image ));
	image->nSize = sizeof( *image );
	
	
    if( (depth != (int)IPL_DEPTH_8U &&
         depth != (int)IPL_DEPTH_16FRACT) ||
         channels < 0 )
    	lcvError("Unsupported format" );
	
	
	image->width = size.width;
	image->height = size.height;
	
	image->nChannels = MAX( channels, 1 );
	image->depth = depth;
	image->align = LCV_WIDTH_ALIGN;
	image->widthStep = (((image->width * image->nChannels *
		 (image->depth & ~IPL_DEPTH_SIGN) + 7)/8)+ image->align - 1) & (~(image->align - 1));
	
	image->imageSize = image->widthStep * image->height;
	
	return(image);
}


char* lcvAlignImage(char* img) {
	return((char*)(((uint32)img + LCV_IMG_ALIGN-1) & -LCV_IMG_ALIGN));
}


void  lcvReleaseImage( IplImage** image ) {
	
	if(image && *image && (*image)->imageDataOrigin) {
		free((*image)->imageDataOrigin);
	}
	
	lcvReleaseImageHeader(image);
}

void  lcvReleaseImageHeader( IplImage** image ) {
	
	if(image && *image) {
        IplImage* img = *image;
        *image = NULL;
        
        free(img);
	}
}



void lcvConvertImage(const IplImage* img_in, IplImage* img_out) {
	
	if(!img_in || !img_out) lcvError("Image parameter is NULL");
	
	if(img_in->width!=img_out->width || img_in->height!=img_out->height) 
		lcvError("Image sizes must be the same");
	
	if(img_in->nChannels!=img_out->nChannels) lcvError("Channel count must be the same");
	
	int16* data;
	
	switch(img_in->depth) {
	case IPL_DEPTH_8U:
		switch(img_out->depth) {
		case IPL_DEPTH_16FRACT:
			//IPL_DEPTH_8U -> IPL_DEPTH_16FRACT
			data=(int16*)img_out->imageData;
			for(int i=0; i<img_in->imageSize; ++i) {
				data[i]=(int16)((int16)(img_in->imageData[i])<<7)-0x7FFF;
			}
			break;
		default:
			lcvError("Unsupported Image Depth");
		}
		break;
		
	case IPL_DEPTH_16FRACT:
		switch(img_out->depth) {
		case IPL_DEPTH_8U:
			//IPL_DEPTH_16FRACT -> IPL_DEPTH_8U
			data=(fract16*)img_in->imageData;
			for(int i=0; i<img_out->imageSize; ++i) {
				img_out->imageData[i]=(char)((data[i]>>8)-128);
			}
			break;
		default:
			lcvError("Unsupported Image Depth");
		}
		break;
		
	default:
		lcvError("Unsupported Image Depth");
	}
	
	
}








