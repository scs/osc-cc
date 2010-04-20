
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

void setPixel(IplImage* img, int x, int y, const char* color) {
	memcpy(img->imageData + 
		(img->depth & ~IPL_DEPTH_SIGN)/8*img->nChannels*(y*img->width+x), color
		, (img->depth & ~IPL_DEPTH_SIGN)/8*img->nChannels);
}

void lcvDrawCentroidMarkers(IplImage* img_in
		, const struct LCV_REGIONS *regions, const char* color) {
	
	if(!img_in || !regions || !color) lcvError("Parameter is NULL");
	
	
	uint16 i;
	/* Draw red crosses */
	for (i = 0; i < regions->noOfObjects; i++)
	{
		setPixel(img_in, regions->objects[i].centroidX,regions->objects[i].centroidY, color);
		setPixel(img_in, regions->objects[i].centroidX-1, regions->objects[i].centroidY, color);
		setPixel(img_in, regions->objects[i].centroidX+1, regions->objects[i].centroidY, color);
		setPixel(img_in, regions->objects[i].centroidX, regions->objects[i].centroidY+1, color);
		setPixel(img_in, regions->objects[i].centroidX, regions->objects[i].centroidY-1, color);	
	}
}

void lcvDrawBoundingBox(IplImage* img_in
		, const struct LCV_REGIONS *regions, const char* color)
{
	
	if(!img_in || !regions || !color) lcvError("Parameter is NULL");
	
	uint16 i, o;
	for(o = 0; o < regions->noOfObjects; o++)
	{
		
		/* Draw the horizontal lines. */
		for (i = regions->objects[o].bboxLeft; i < regions->objects[o].bboxRight; i += 1)
		{
			setPixel(img_in, i, regions->objects[o].bboxTop, color);
			setPixel(img_in, i, regions->objects[o].bboxBottom, color);
		}
		
		/* Draw the vertical lines. */
		
		for (i = regions->objects[o].bboxTop; i < regions->objects[o].bboxBottom-1; i += 1)
		{
			setPixel(img_in, regions->objects[o].bboxLeft, i, color);
			setPixel(img_in, regions->objects[o].bboxRight, i, color);
		}
	}
}


