/*	
	Copyright (C) 2008 Reto Bättig
	
	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*!@file leanXoverlay.c
 * @Write text on top of an image
 */

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <oscar.h>
#include <leanXoverlay.h>

ov_font_t OV_FONT_SYSTEM = { NULL, "Font_System.bmp", 6, 12, 32, 127 };

uint16 sin8[256];
uint16 cos8[256];
bool  init_done = false;

#define INIT(_pPic) \
	unsigned char *_data = _pPic->data; \
	uint16 _coldepth = OSC_PICTURE_TYPE_COLOR_DEPTH(_pPic->type)/8;\
	uint16 _width = _pPic->width; 

#define SETPIX(_x, _y, _col)  \
	if (_coldepth == 1) \
 		_data[(_x+((_y)*_width))*_coldepth] = _col; \
	else { \
 		_data[(_x+((_y)*_width))*_coldepth]   = (_col & 0xFF0000)>>16; \
 		_data[(_x+((_y)*_width))*_coldepth+1] = (_col & 0x00FF00)>>8; \
 		_data[(_x+((_y)*_width))*_coldepth+2] = _col & 0x0000FF; \
	}


#define ORDER(_x1, _x2, _tmp) \
	if (_x1>_x2) { \
		_tmp = _x1; \
		_x1  = _x2; \
		_x2  = _tmp; \
	} o

int Occ_ov_init() {
	ov_font_t *f = &OV_FONT_SYSTEM;
	uint16 i;
	OSC_ERR err;
	
	if (init_done)
		return 0;

	f->p = malloc(sizeof(struct OSC_PICTURE));
	f->p->width = 0;
	f->p->height = 0;
	f->p->data = NULL;
	err = OscBmpRead(f->p, f->filename);
	if (err != SUCCESS) {
		printf("Could not read font %s\n", f->filename);
		f->p = NULL;
		return -1;
	}

	for (i=0; i<256; i++) {
		sin8[i]=256*sin(M_PI*2/256*i);
		cos8[i]=256*cos(M_PI*2/256*i);
	}

	init_done = true;
	return 0;

}

void blob(struct OSC_PICTURE *pPic, int x, int y, int size, uint32 color) 
{
	INIT(pPic);
	int a, b;
	for (a=0; a<size; a++) {
		for (b=0; b<size; b++) {
			SETPIX(x+b,(y+a), color);
		}
	}
}

void ov_text(struct OSC_PICTURE *pPic, int x, int y, int size, char *text, uint32 col_fg, uint32 col_bg) 
{
	int i;
	int a,b;
	uint8 num;
	uint8 pix;
	ov_font_t *f = &OV_FONT_SYSTEM;
	uint8 *f_data;
	
	
	f_data=f->p->data;

	for (i=0; text[i]!=0; i++) {
		if (text[i]>f->to)
			continue;
		if (text[i]<f->from)
			continue;

		num = text[i] - f->from;
		for (a=0; a<f->width; a++) {
			for (b=0; b<f->height; b++) {
				pix = f_data[b*f->p->width+num*f->width+a];
				if (pix)
					blob(pPic, x+a*size, y+b*size, size, col_fg);
				else
					blob(pPic, x+a*size, y+b*size, size, col_bg);
			}
		}
		x = x + f->width*size;
	}
}

void ov_line(struct OSC_PICTURE *pPic, int x1, int y1, int x2, int y2, uint32 col) 
{
	INIT(pPic);
	int16 x, y, dx, dy;
	int32 a;
	
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	if (dx==0 && dy==0)
		return;

	if (dx > dy) {
		a = (y2-y1)*64/dx;
		if (x2>x1) {
			for (x=0; x<=dx; x++) {
				y = y1+((x*a)>>6);
				SETPIX(x1+x, y, col)
			}
		} else {		
			for (x=0; x<=dx; x++) {
				y = y1+((x*a)>>6);
				SETPIX(x1-x, y, col)
			}
		}
	} else {
		a = (x2-x1)*64/dy;
		if (y2>y1) {
			for (y=0; y<=dy; y++) {
				x = x1+((y*a)>>6);
				SETPIX(x, y1+y, col)		
			}
		} else {
			for (y=0; y<=dy; y++) {
				x = x1+((y*a)>>6);
				SETPIX(x, y1-y, col)		
			}
		}
	}
}

void ov_box(struct OSC_PICTURE *pPic, int x1, int y1, int x2, int y2, uint32 col, bool fill)
{
	int y;
	int tmp;

	if (fill) {
		if (y1>y2) {
			tmp=x1; x1=x2; x2=tmp;
			tmp=y1; y1=y2; y2=tmp;
		}
		for (y=y1; y<=y2; y++)
			ov_line(pPic, x1, y, x2, y, col);
	} else {
		ov_line(pPic, x1, y1, x2, y1, col);
		ov_line(pPic, x1, y1, x1, y2, col);
		ov_line(pPic, x2, y1, x2, y2, col);
		ov_line(pPic, x1, y2, x2, y2, col);
	}
}

void ov_ellipse(struct OSC_PICTURE *pPic, int x1, int y1, int x2, int y2, uint32 col, bool fill)
{
	int tmp;
	uint8 phi;
	uint16 s,c;
	int w,h,y;
	uint16 x0, y0, x1a, x2a, y1a, y2a;
	uint16 ox1a=0, ox2a=0, oy1a=0, oy2a=0;

	if (y1>y2) {
		tmp=y1; y1=y2; y2=tmp;
	}
	if (x1>x2) {
		tmp=x1; x1=x2; x2=tmp;
	}

	w=(x2-x1)>>1;
	h=(y2-y1)>>1;
	x0 = x1 + w;
	y0 = y1 + h;

	for (phi = 0; phi <= 64; phi+=1) {
		s = (int32)w*sin8[phi]>>8;
		c = (int32)h*cos8[phi]>>8;
		x1a = x0+s;
		x2a = x0-s;
		y1a = y0+c;
		y2a = y0-c;
		if (phi != 0) {
			ov_line(pPic, ox1a, oy1a, x1a, y1a, col);
			ov_line(pPic, ox2a, oy1a, x2a, y1a, col);
			ov_line(pPic, ox1a, oy2a, x1a, y2a, col);
			ov_line(pPic, ox2a, oy2a, x2a, y2a, col);
		}
		if ((phi != 0) && fill) {
			for (y=oy2a; y<=y2a; y++) {
				ov_line(pPic, ox1a, y, ox2a, y, col);
			}
			for (y=y1a; y<=oy1a; y++) {
				ov_line(pPic, ox1a, y, ox2a, y, col);
			}
		}
		ox1a = x1a; 
		ox2a = x2a; 
		oy1a = y1a; 
		oy2a = y2a; 
	}
}	

void ov_pixel(struct OSC_PICTURE *pPic, int x, int y, uint32 col)
{
	INIT(pPic);
	SETPIX(x, y, col)		
}

void ov_plot_vector(struct OSC_PICTURE *pic, int x0, int y0, int8 *v, int len, uint32 col)
{
	int i;
	for (i=1; i<len; i++)  {
		ov_line(pic, x0+i-1, y0+v[i-1], x0+i, y0+v[i], col);
	}
	ov_line(pic, x0, y0, x0+len, y0, col);
}

void ov_plot_vector_scale8(struct OSC_PICTURE *pic, int x1, int y1, int height, int8 *vect, int16 len, uint32 color) 
{
	int8 v[len];
	int x;
	float scale = 0;
	int8 min = 0;
	int8 max = 0;

	for (x=0; x<len; x++)
	{
		if (vect[x] > max)
			max = vect[x];
		if (vect[x] < min)
			min = vect[x];
	}
	if (max > (-min))
		scale = (float)height / -2 / max;
	else
		scale = (float)height / -2 / (-min);

	for (x=0; x<len; x++)
		v[x] = vect[x]*scale;

	ov_plot_vector(pic, x1, y1+height/2, v, len, color);
}

void ov_plot_vector_scale16(struct OSC_PICTURE *pic, int x1, int y1, int height, int16 *vect, int16 len, uint32 color) 
{
	int8 v[len];
	int x;
	float scale = 0;
	int16 min = 0;
	int16 max = 0;

	for (x=0; x<len; x++)
	{
		if (vect[x] > max)
			max = vect[x];
		if (vect[x] < min)
			min = vect[x];
	}
	if (max > (-min))
		scale = (float)height / -2 / max;
	else
		scale = (float)height / -2 / (-min);

	for (x=0; x<len; x++)
		v[x] = vect[x]*scale;

	ov_plot_vector(pic, x1, y1+height/2, v, len, color);
}

void ov_plot_vector_scale32(struct OSC_PICTURE *pic, int x1, int y1, int height, int32 *vect, int16 len, uint32 color) 
{
	int8 v[len];
	int x;
	float scale = 0;
	int32 min = 0;
	int32 max = 0;

	for (x=0; x<len; x++)
	{
		if (vect[x] > max)
			max = vect[x];
		if (vect[x] < min)
			min = vect[x];
	}
	if (max > (-min))
		scale = (float)height / -2 / max;
	else
		scale = (float)height / -2 / (-min);

	for (x=0; x<len; x++)
		v[x] = vect[x]*scale;

	ov_plot_vector(pic, x1, y1+height/2, v, len, color);
}

void ov_bar_vert(struct OSC_PICTURE *pic, int x1, int y1, int x2, int y2, int percentage, uint32 color) 
{
	int height, tmp;

	if (y1>y2) {
		tmp=y1; y1=y2; y2=tmp;
	}
	if (x1>x2) {
		tmp=x1; x1=x2; x2=tmp;
	}

	if (percentage > 100)
		percentage = 100;
	if (percentage < 0)
		percentage = 0;
	height = (y2-y1)*percentage/100;
	
	ov_box(pic, x1, y2-height, x2, y2, color, true);
	ov_box(pic, x1, y1, x2, y2, color, false);
}

