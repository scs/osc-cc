
#include <leancv.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


void lcvError(const char* fmt, ...) {
	
	va_list va;
	char buf[1024], buf2[1024];
	
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	
	sprintf(buf2, "LeanCV Error: %s in %s, file %s, line %d\n"
		, buf, __FUNCTION__, __FILE__, __LINE__);
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








