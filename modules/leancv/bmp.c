
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
#include <lcv_bmp.h>

#include <nih.h> /* oscar */



void lcvImgReverseRowOrder(IplImage* img) {
	
	void        *pTempRow;
	uint32      curFIndex, curBIndex;
	uint8       *pData = (uint8*)img->imageData;
	
	
	pTempRow = malloc(img->widthStep);
	
	for(int i = 0; i < img->height/2; i++)
	{
		/* Swap the rows */
		curFIndex = i * img->widthStep;
		curBIndex = (img->height - i - 1) * img->widthStep;
		memcpy(pTempRow, &pData[curFIndex], img->widthStep);
		memcpy(&pData[curFIndex], &pData[curBIndex], img->widthStep);
		memcpy(&pData[curBIndex], pTempRow, img->widthStep);
	}
	
	free(pTempRow);
}



static inline void lcvBmpReadHdrInfo(const uint8 *pHdr,
		int32 * pWidth, int32 * pHeight, int32 *pDataOffset,
		int16 * pColorDepth)
{
#ifdef CPU_LITTLE_ENDIAN
	*pDataOffset = LD_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET]);
	*pWidth = LD_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH]);
	*pHeight = LD_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT]);
	*pColorDepth = LD_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH]);
	/* BMP uses little endian format */
#else /* CPU_LITTLE_ENDIAN */
	/* For a big endian CPU we need to swap endianness */
	*pDataOffset =
		ENDIAN_SWAP_32(LD_INT32((&pHdr[BMP_HEADER_FIELD_DATA_OFFSET])));
	*pWidth =
		ENDIAN_SWAP_32(LD_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH]));
	*pHeight =
		ENDIAN_SWAP_32(LD_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT]));
	*pColorDepth =
		ENDIAN_SWAP_16(LD_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH]));
#endif /* CPU_LITTLE_ENDIAN */
}

static inline void lcvBmpWriteHdrInfo(uint8 *pHdr,
		const int32 width, const int32 height, const int16 colorDepth,
		const int32 headerSize)
{
	int32           imageSize, dataOffset, fileSize;
	
	imageSize = (((int32)width*(colorDepth/8) + 3)/4)*4*height;
	fileSize = imageSize + headerSize;
	dataOffset = headerSize;
	
	/* BMP uses little endian format */
#ifdef CPU_LITTLE_ENDIAN
	/* Data is already lying correctly in memory */
	ST_INT32(&pHdr[BMP_HEADER_FIELD_FILE_SIZE], fileSize);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET], dataOffset);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH], width);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT], height);
	ST_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH], colorDepth);
	ST_INT32(&pHdr[BMP_HEADER_FIELD_IMAGE_SIZE], imageSize);
#else /* CPU_LITTLE_ENDIAN */
	/* For a big endian CPU we need to swap endianness */
	ST_INT32(&pHdr[BMP_HEADER_FIELD_FILE_SIZE],
			ENDIAN_SWAP_32(fileSize));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_DATA_OFFSET],
			ENDIAN_SWAP_32(dataOffset));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_WIDTH],
			ENDIAN_SWAP_32(width));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_HEIGHT],
			ENDIAN_SWAP_32(height));
	ST_INT16(&pHdr[BMP_HEADER_FIELD_COLOR_DEPTH],
			ENDIAN_SWAP_16(colorDepth));
	ST_INT32(&pHdr[BMP_HEADER_FIELD_IMAGE_SIZE],
			ENDIAN_SWAP_32(imageSize));
#endif /* CPU_LITTLE_ENDIAN */
}



OSC_ERR lcvBmpWrite(const IplImage* img, const char* file_name) {

	FILE            *pPicFile;
	int16           colorDepth;
	uint8           *aryBmpHead;
	uint32          bmpHeadSize;
	uint32          zero = 0;
	int32          row, rowLen, padLen;
	uint8           *pData;
	
	/* Input validation */
	if(img == NULL || img->imageData == NULL ||
			file_name == NULL || file_name[0] == '\0') {
		lcvError("Invalid Parameter");
	}
	
	if(img->depth != IPL_DEPTH_8U) lcvError("Image depth must be %i", IPL_DEPTH_8U);
	
	
	if(img->nChannels == 1) {
		aryBmpHead = aryBmpHeadGrey;
		bmpHeadSize = sizeof(aryBmpHeadGrey);
		colorDepth = 8;
	} else if(img->nChannels == 3) {
		aryBmpHead = aryBmpHeadRGB;
		bmpHeadSize = sizeof(aryBmpHeadRGB);
		colorDepth = 24;
	} else {
		lcvError("Image Channel count must be 1 or 3");
	}
	
			
	/* Initialize the header */
	lcvBmpWriteHdrInfo(aryBmpHead,
			(int32)img->width,
			(int32)img->height,
			colorDepth,
			bmpHeadSize);
	
	pPicFile = fopen(file_name, "wb");
	if(pPicFile == NULL) {
		return -EUNABLE_TO_OPEN_FILE;
	}
	
	/* Write header and data to file */
	fwrite(aryBmpHead, 1, bmpHeadSize, pPicFile);
	
	/* Pad to 4 bytes. Write 'reversed'. Bmps are stored 'reversed'
	 * in the file. */
	rowLen = (int32)img->width*colorDepth/8;
	padLen = ((rowLen + 3)/4)*4 - rowLen;
	pData = (uint8*)img->imageData;
	for(row =  img->height - 1; row >= 0; row--)
	{
		/* Row data. */
		fwrite(&pData[row*img->width*colorDepth/8], 1,
			rowLen, pPicFile);
		if(padLen != 0)
		{
			/* Row padding. */
			fwrite(&zero, 1, padLen, pPicFile);
		}
	}
	fflush(pPicFile);
	fclose(pPicFile);
	
	return SUCCESS;
	
}


IplImage* lcvBmpRead(const char* file_name) {
	
	if(!file_name) return(NULL);
	
	IplImage* img=NULL;

	FILE            *pPicFile;
	/* Temporary buffer to store the header */
	unsigned char   *pHeader[sizeof(aryBmpHeadRGB)];
	int32           dataOffset, width, height;
	int16           colorDepth;
	uint32          imgSize;
	int             bIsReversed = FALSE;
	uint16          row, rowLen;
	uint8           *pData;
	
	
	pPicFile = fopen(file_name, "rb");
	if(pPicFile == NULL) return(NULL);
	
	
	/* Read in the header and extract the interesting fields */
	if(fread(pHeader, 1, sizeof(aryBmpHeadRGB), pPicFile) != sizeof(aryBmpHeadRGB)) {
		fclose(pPicFile);
		return(NULL);
	}
	
	lcvBmpReadHdrInfo((uint8*)pHeader,
			&width,
			&height,
			&dataOffset,
			&colorDepth);
	
	if(height > 0)
	{
		/* The row order is reversed (mirrored on y axis). This is
		 * the default way that bitmaps are stored (reversed, so to
		 * speak). */
		bIsReversed = TRUE;
	} else {
		height = height * (-1);
	}
	
	/* Check the header for validity */
	if(unlikely(colorDepth != 24 && colorDepth != 8))
	{
		fclose(pPicFile);
		return(NULL);
	}
	if(unlikely(dataOffset != sizeof(aryBmpHeadRGB) &&
			dataOffset != sizeof(aryBmpHeadGrey)))
	{
		fclose(pPicFile);
		return(NULL);
	}
	
	imgSize = width*height*(colorDepth/8);
	
	int width_aligned=((((uint32)width) + 3)/4)*4;
	/* image width should be a multiple of 4 */
	
	img=lcvCreateImage(cvSize(width_aligned, height), IPL_DEPTH_8U, colorDepth/8);
	
		
	/* Seek the pixel data portion of the file and read it in */
	fseek(pPicFile, dataOffset, SEEK_SET);
	
	/* Padded to 4 bytes. */
	rowLen = ((((uint32)img->width*(colorDepth/8)) + 3)/4)*4;
	pData = (uint8*)img->imageData;
	for(row = 0; row < img->height; row++)
	{
	  if(fread(&pData[row*img->width*colorDepth/8], 1,
		   rowLen, pPicFile) != rowLen)
		{
		  fclose(pPicFile);
		  lcvReleaseImage(&img);
		  return(NULL);
		}
	}
	
	fclose(pPicFile);
	
	if(bIsReversed)
	{
		/* The row order is reversed */
		lcvImgReverseRowOrder(img);
	}
	
	return(img);
}






