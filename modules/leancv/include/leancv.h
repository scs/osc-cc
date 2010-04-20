
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
 * 
 * 
 * Supported Image Formats/Image Conditions:
 * ----------------------------------------
 * 
 * depth:		IPL_DEPTH_8U
 * 				IPL_DEPTH_16FRACT
 * 
 * width: 		multiple of LCV_WIDTH_ALIGN
 * 
 * channels:	1 or 3 (not all functions support 3 channels)
 * 
 * 
 */

#ifndef LEANCV_INCLUDE_LEANCV_H_
#define LEANCV_INCLUDE_LEANCV_H_


/* include the opencv header for image type */
#include <opencv/cxtypes.h>

#define SKIP_INT64_TYPEDEF
#include <oscar.h>
#include <cam.h>

#define IPL_DEPTH_16FRACT IPL_DEPTH_16S

#define LCV_WIDTH_ALIGN 4 /* to be compatible with OpenCV */
#define LCV_IMG_ALIGN 16 /* align created images. Default: CV_MALLOC_ALIGN=16 */


#ifdef __cplusplus
extern "C" {
#endif



void lcvError_m(const char* func, const char* file, int line, const char* fmt, ...);

/*! @brief outputs an error to stderr and quits the application */
#ifdef __cplusplus
#define lcvError(fmt, ...) lcvError_m(__FUNCTION__, __FILE__, __LINE__, fmt, ## __VA_ARGS__)
#else
#define lcvError(fmt, args ...) lcvError_m(__FUNCTION__, __FILE__, __LINE__, fmt, ## args)
#endif



//---------------------------------------------------------------------------
///  Image Allocation & Deallocation
//---------------------------------------------------------------------------


/*! @brief Creates IPL image (header and data) */
IplImage*  lcvCreateImage( CvSize size, int depth, int channels );

/*! @brief Allocates and initializes IplImage header */
IplImage*  lcvCreateImageHeader( CvSize size, int depth, int channels );

/*! @brief Align an image to LCV_IMG_ALIGN */
char* lcvAlignImage(char* img);

/*! @brief Releases IPL image header and data */
void  lcvReleaseImage( IplImage** image );

/*! @brief Releases (i.e. deallocates) IPL image header */
void  lcvReleaseImageHeader( IplImage** image );



//---------------------------------------------------------------------------
///  Debayering
//---------------------------------------------------------------------------

/*! @brief Debayer a captured image to BGR color 
 * the output format is BGR interleaved
 * 
 * @param pTmp 	Temporary memory for intermediate calculations
 *  			(size: raw_img->width x 4)
 */
void lcvDebayerBilinear(const IplImage* raw_img, enum EnBayerOrder order
		, uint8* pTmp, IplImage* output);

/*! @brief Debayer a captured image (captured with a color sensor) to gray color 
 *  the output image must have half the width & height of the input image
 */
void lcvDebayerGray(const IplImage* raw_img, enum EnBayerOrder order, IplImage* output);



//---------------------------------------------------------------------------
///  Image Formats
//---------------------------------------------------------------------------

/*! @brief write a bmp image to a file */
OSC_ERR lcvBmpWrite(const IplImage* img, const char* file_name);


/*! @brief get a bmp header.
 *  @param img			pointer to the image
 *  @param header_out	will point to the header on success
 *  @return 			size of the header, < 0 on error
  */
int lcvBmpHeader(const IplImage* img, char** header_out);

/*! @brief read a bmp image from a file. if channels == 3, image format is BGR
 * @return: created image on success, NULL on error
 */
IplImage* lcvBmpRead(const char* file_name);

/*! @brief Reverse the row order of an image. Used for bmp's because they are
 *  stored upside down
 */
void lcvImgReverseRowOrder(IplImage* img);



//---------------------------------------------------------------------------
///  Image Scaling & Conversion
//---------------------------------------------------------------------------


/*! @brief Convert one image type to another.
 *  	currently, IPL_DEPTH_16FRACT  <-> IPL_DEPTH_8U conversion is supported
 */
void lcvConvertImage(const IplImage* img_in, IplImage* img_out);


/*! @brief Convert an image to a binary
 *  	img_in can be the same as img_out
 */
void lcvConvertImageBinary(const IplImage* img_in, IplImage* img_out, int threshold);


//---------------------------------------------------------------------------
///  Filters & Object detection
//---------------------------------------------------------------------------


#define MAX_RUN_COUNT 8192
#define MAX_OBJECT_COUNT 1024

/*! @brief Structure representing a run used in connected components labeling based on run-length-encoding (RLE). */
struct LCV_REGIONS_RUN {
	uint16 row;								/*!< @brief The row in which the run was detected */
	uint16 startColumn;						/*!< @brief The start column of the detected run */
	uint16 endColumn;						/*!< @brief The end column of the detected run */
	struct LCV_REGIONS_RUN *parent;			/*!< @brief Pointer to the parent run */
	struct LCV_REGIONS_RUN *next;			/*!< @brief Pointer to the next run in the linked list of runs */
	uint16 label;							/*!< @brief Label number of the run */
};

/*! @brief Structure representing an object (=region) in the binary image. Used in connected components labeling based on run-length-encoding (RLE). */
struct LCV_REGIONS_OBJECT {
	struct LCV_REGIONS_RUN *root;			/*!< @brief Pointer to the root run representing the object */
	uint16 area;							/*!< @brief Property entry for object area */
	uint16 perimeter;						/*!< @brief Property entry for object perimter (not implemented yet) */
	uint16 centroidX, centroidY;			/*!< @brief Property entry for object centroid pixel coordinates */
	uint16 bboxTop, bboxBottom, bboxLeft, bboxRight; /*!< @brief Property entry for object bounding box coordinates */
};

/*! @brief Structure representing a binary image as connected sets of runs grouped into objects (=regions). It is the result/output
 * of the connected components labeling algorithm based on run-length-encoding (RLE). */
struct LCV_REGIONS {
	uint16 noOfRuns;											/*!< @brief Number of detected runs */
	uint16 noOfObjects;											/*!< @brief Number of detected objects/regions */
	struct LCV_REGIONS_RUN runs[MAX_RUN_COUNT];					/*!< @brief The array of all detected runs */
	struct LCV_REGIONS_OBJECT objects[MAX_OBJECT_COUNT];		/*!< @brief Array of the detected objects */
};


/*!
 * @brief Label Binary Image
 * 
 * This function labels the binary image by checking for connected-components based on
 * run-length encoding. This function outputs a representation of the binary image based on connected
 * sets of runs. The sets of runs are refered to as 'objects' (=regions of foreground pixels).
 * 
 * @param img	pointer to the image
 * @param regions Pointer to the regions struct
 */
void lcvLabelBinary(const IplImage* img_in, struct LCV_REGIONS* regions);

/*!
 * @brief Extract Properties of the Regions (=labeled binary Image)
 * 
 * This function fills out the internal properties data fields (area,
 * centroid, bounding box) of the regions.
 * 
 * @param regions Pointer to the regions struct
 */
void lcvGetRegionProperties(struct LCV_REGIONS* regions);



//---------------------------------------------------------------------------
///  Drawing
//---------------------------------------------------------------------------


/*!
 * @brief Draw Centroid Markers
 * 
 * This function draws the centroids of the Regions as small crosses. 
 * 
 * @param img_in Pointer to the input color picture struct.
 * @param regions Pointer to the regions struct
 * @param color drawing color. size must be img_in->nChannels * image depth/8
 */
void lcvDrawCentroidMarkers(IplImage* img_in
		, const struct LCV_REGIONS *regions, const char* color);

/*!
 * @brief Draw Bounding Boxes
 * 
 * This function draws the bounding boxes of the Regions as rectangles. 
 * 
 * @param img_in Pointer to the input color picture struct.
 * @param regions Pointer to the regions struct
 * @param color drawing color. size must be img_in->nChannels * image depth/8
 */
void lcvDrawBoundingBox(IplImage* img_in
		, const struct LCV_REGIONS *regions, const char* color);




#ifdef __cplusplus
}
#endif



#endif /* LEANCV_INCLUDE_LEANCV_H_ */







