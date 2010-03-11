
#include <leancv.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


void lcvError(const char* fmt, ...) {
	
	va_list va;
	char buf[1024], buf2[1024];
	
	sprintf(buf, fmt, va);
	
	sprintf(buf2, "LeanCV Error: %s in %s, file %s, line %d\n"
		, buf, __FUNCTION__, __FILE__, __LINE__);
	fprintf(stderr, "%s\n", buf2);
	fflush(stderr);
	
	abort();
}


IplImage*  lcvCreateImage( CvSize size, int depth, int channels ) {
	
	IplImage* image = malloc(sizeof(IplImage));
	 
	memset( image, 0, sizeof( *image ));
	image->nSize = sizeof( *image );
	
	 
}


IplImage*  lcvCreateImageHeader( CvSize size, int depth, int channels ) {
	
}


void  lcvReleaseImage( IplImage** image ) {
	
}

void  lcvReleaseImageHeader( IplImage** image ) {


}


