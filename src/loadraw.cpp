/*
 * loadraw.cpp
 *
 *  Created on: Jun 22, 2012
 *      Author: nikolas
 */

#include <iostream>
#include <algorithm>
#include <libraw/libraw.h>
#include "loadraw.h"


ushort *loadraw(const char *filename, int *w, int *h, float *av, float *tv)
{
	LibRaw r;
	r.open_file(filename);

	int fullw = r.imgdata.rawdata.sizes.raw_width;
	int fullh = r.imgdata.rawdata.sizes.raw_height;
	*h = r.imgdata.rawdata.sizes.height;
	*w = r.imgdata.rawdata.sizes.width;
	int lmargin = r.imgdata.rawdata.sizes.left_margin;
	int tmargin = r.imgdata.rawdata.sizes.top_margin;

	*av = r.imgdata.other.aperture;
	*tv = r.imgdata.other.shutter;

	int err = r.unpack();
	if(err != LIBRAW_SUCCESS)
	{
		exit(-1);
	}

	unsigned int size = r.imgdata.rawdata.sizes.width * r.imgdata.rawdata.sizes.height;
	ushort *ret = new ushort[size];

	for(int i=tmargin; i<fullh; i+=1)
		for(int j=lmargin; j<fullw; j+=1)
		{
			unsigned int raw_ndx = i*fullw+j;
			unsigned int new_ndx = (i-tmargin)*(*w)+(j-lmargin);
			ret[new_ndx] = r.imgdata.rawdata.raw_image[raw_ndx];
		}

	r.recycle();
	return ret;
}


ushort *loadsraw(const char *filename, int *w, int *h, float *av, float *tv)
{
	LibRaw r;
	r.open_file(filename);

	int fullw = r.imgdata.rawdata.sizes.raw_width;
	int fullh = r.imgdata.rawdata.sizes.raw_height;
	*h = r.imgdata.rawdata.sizes.height;
	*w = r.imgdata.rawdata.sizes.width;

	*av = r.imgdata.other.aperture;
	*tv = r.imgdata.other.shutter;

	int err = r.unpack();
	if(err != LIBRAW_SUCCESS)
	{
		exit(-1);
	}

	unsigned int size = r.imgdata.rawdata.sizes.width * r.imgdata.rawdata.sizes.height;
	ushort *ret = new ushort[size*3];

	uint imgmin = 999999;
	uint imgmax = 0;

	for(int i=0; i<fullh; i+=1)
		for(int j=0; j<fullw; j+=1)
		{
			unsigned int raw_ndx = i*fullw+j;
			unsigned int new_ndx = raw_ndx*3;

			for(int k = 0; k < 3; ++k)
			{
				ret[new_ndx +k] = r.imgdata.rawdata.color4_image[raw_ndx][k];
				if(ret[new_ndx +k] > imgmax) imgmax = ret[new_ndx +k];
				if(ret[new_ndx +k] < imgmin) imgmin = ret[new_ndx +k];
			}
		}

	std::cout << "min: " << imgmin << " max: " << imgmax << std::endl;

	r.recycle();
	return ret;
}
