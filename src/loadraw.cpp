/*
 * loadraw.cpp
 *
 *  Created on: Jun 22, 2012
 *      Author: nikolas
 */


#include <libraw/libraw.h>

#include "loadraw.h"

ushort *loadraw(const char *filename, int *w, int *h)
{
	LibRaw rawwwr;
	rawwwr.open_file(filename);

	*w = rawwwr.imgdata.sizes.iwidth;
	*h = rawwwr.imgdata.sizes.iheight;

	rawwwr.unpack();


	/* This creates a bitmap of the raw data (8-bit)
	raw.dcraw_process();
	libraw_processed_image_t* img = raw.dcraw_make_mem_image();
	for(int i=0; i<img->height * img->width * img->colors; i++)
	{
		ret[i] = img->data[i];
	}*/

	rawwwr.raw2image();
	ushort  *ret = new ushort[(*w) * (*h) * 4];

	for(int i = 0; i < rawwwr.imgdata.sizes.iwidth * rawwwr.imgdata.sizes.iheight; i++)
	{
		ret[i*4+0] = rawwwr.imgdata.image[i][0];
		ret[i*4+1] = rawwwr.imgdata.image[i][1];
		ret[i*4+2] = rawwwr.imgdata.image[i][2];
		ret[i*4+3] = rawwwr.imgdata.image[i][3];
	}

	//LibRaw::dcraw_clear_mem(img);
	rawwwr.recycle();
	return ret;
}



