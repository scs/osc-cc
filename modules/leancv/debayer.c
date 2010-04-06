
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
#include <lcv_debayer.h>


void lcvDebayerBilinear(const IplImage* raw_img, enum EnBayerOrder order
		, uint8* pTmp, IplImage* output) {
	
	
	if(!raw_img || !output) lcvError("NULL Parameter");
	if(raw_img->width != output->width || raw_img->height != output->height) 
		lcvError("Size must be the same");
	if(raw_img->nChannels != 1 || output->nChannels != 3) 
		lcvError("raw_img must have 1 channel and output must have 3 channels");
	if(raw_img->depth != IPL_DEPTH_8U || output->depth != IPL_DEPTH_8U) 
		lcvError("image depth must be %i", IPL_DEPTH_8U);
	
	if(order != ROW_BGBG && order != ROW_GRGR)
		lcvError("Unsupported Bayer order");
	
	
	_DebayerBilinearBGR((uint8*)output->imageData, (uint8*)raw_img->imageData, 
			raw_img->width, raw_img->height, pTmp, order);
	
}


void lcvDebayerGray(const IplImage* raw_img, enum EnBayerOrder order, IplImage* output) {

	if(!raw_img || !output) lcvError("NULL Parameter");
	if(raw_img->width != output->width*2 || raw_img->height != output->height*2) 
		lcvError("output width and height must be half of the input image");
	if(raw_img->nChannels != 1 || output->nChannels != 1) 
		lcvError("both image channels must be 1");
	if(raw_img->depth != IPL_DEPTH_8U || output->depth != IPL_DEPTH_8U) 
		lcvError("image depth must be %i", IPL_DEPTH_8U);
	
	uint8* pOut=(uint8*)output->imageData;
	uint8* pRaw=(uint8*)raw_img->imageData;
	uint16 width=raw_img->width;
	uint16 height=raw_img->height;
 	
 	bool bTopLeftIsGreen;
	uint16 ix, iy, outWidth = width / 2, outHeight = height / 2;
 	
	
	bTopLeftIsGreen = (order == ROW_GBGB) || (order == ROW_GRGR);

	if (bTopLeftIsGreen)
		for(iy = 0; iy < outHeight; iy += 1)
		{
			uint16 iyRaw = iy * 2;
			for(ix = 0; ix < outWidth; ix += 1)
			{
				uint8 cellRed, cellGreen1, cellGreen2, cellBlue;
				uint16 ixRaw = ix * 2;
				uint16 grey;
				
				cellGreen1 = pRaw[iyRaw * width + ixRaw];
				cellRed = pRaw[iyRaw * width + ixRaw + 1];
				cellBlue = pRaw[(iyRaw + 1) * width + ixRaw];
				cellGreen2 = pRaw[(iyRaw + 1) * width + ixRaw + 1];
				
				grey = (uint16)cellRed * 2 + (uint16)cellGreen1 + (uint16)cellGreen2 + (uint16)cellBlue * 2;
				pOut[iy * outWidth + ix] = grey / 6;
			}
		}
	else
		for(iy = 0; iy < outHeight; iy += 1)
		{
			uint16 iyRaw = iy * 2;
			for(ix = 0; ix < outWidth; ix += 1)
			{
				uint8 cellRed, cellGreen1, cellGreen2, cellBlue;
				uint16 ixRaw = ix * 2;
				uint16 grey;;
				
				cellRed = pRaw[iyRaw * width + ixRaw];
				cellGreen1 = pRaw[iyRaw * width + ixRaw + 1];
				cellGreen2 = pRaw[(iyRaw + 1) * width + ixRaw];
				cellBlue = pRaw[(iyRaw + 1) * width + ixRaw + 1];
				
				grey = (uint16)cellRed * 2 + (uint16)cellGreen1 + (uint16)cellGreen2 + (uint16)cellBlue * 2;
				pOut[iy * outWidth + ix] = grey / 6;
			}
		}
}






	
