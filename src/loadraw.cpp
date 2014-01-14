/*
 * loadraw.cpp
 *
 *  Created on: Jun 22, 2012
 *      Author: nikolas
 */

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

	r.unpack();

	unsigned int size = r.imgdata.rawdata.sizes.width * r.imgdata.rawdata.sizes.height;
	ushort *ret = new ushort[size];

	for(int i=tmargin; i<fullh; i++)
		for(int j=lmargin; j<fullw; j++)
		{
			unsigned int raw_ndx = i*fullw+j;
			unsigned int new_ndx = (i-tmargin)*(*w)+(j-lmargin);
			ret[new_ndx] = r.imgdata.rawdata.raw_image[raw_ndx];
		}

	r.recycle();
	return ret;
}



