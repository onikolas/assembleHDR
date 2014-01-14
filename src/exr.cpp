
#include <string>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <half.h>



#include "exr.h"

using namespace Imf;
using namespace Imath;


Exr::Exr()
{
	HDRimg =NULL;
	width = height = 0;
}

Exr::Exr(string filename)
{
	HDRimg = NULL;
	ReadEXR(filename, 1);
}

Exr::Exr(const Exr &a)
{
	HDRimg = new fRGB[a.width*a.height];
	for(int i=0; i<a.width*a.height; i++)
		HDRimg[i]=a.HDRimg[i];
	width = a.width; height = a.height;
}

Exr::~Exr()
{
	if(HDRimg) delete [] HDRimg;
}

int Exr::ToneMapEXR(float i, float max)
{
	int r = (i/max)*255;
	if(r<0)   r=0;
	if(r>255) r=255;
	return r;
}

void Exr::FromArray(fRGB *a, int w, int h)
{
	if(HDRimg) delete [] HDRimg;
	HDRimg = new fRGB[w*h];
	for(int i=0; i<w*h; i++) HDRimg[i] = a[i];
	width = w; height = h;
}

bool Exr::IsEmpty()
{
	if(HDRimg) return false;
	return true;
}

void Exr::Init(int w, int h, fRGB c)
{
	HDRimg = new fRGB[w*h];
	for(int i=0; i<w*h; i++)
	{
		HDRimg[i].r = c.r;
		HDRimg[i].g = c.g;
		HDRimg[i].b = c.b;
	}
	width=w; height=h;
}

//TODO We could sample or average the area that is reduced to one pixel
//     instead of just getting one pixel (the top left)
/*fRGB *ScaleEXR(fRGB *HDRimg, int ow, int oh, int nw, int nh)
{
	if((ow < nw) || (oh < nh))
		return 0;

	float multw = ow/(float)nw;
	float multh = oh/(float)nh;

	fRGB * newimg = new fRGB[nw*nh];

	for(int a=0 ; a<(nw*nh); a++)
	{
		int i = a / nw;
		int j = a % nw;

		int io = i * multh;
		int jo = j * multw;

		newimg[a] = HDRimg[io*ow + jo];
	}

	return newimg;
}*/

float Exr::ReadEXR(string filename, float adjust)
{
	InputFile file( filename.c_str() );
	Box2i dw = file.header().dataWindow();
	width  = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;

	half *rgb = new half[3 * width * height];

	FrameBuffer frameBuffer;
	frameBuffer.insert("R", Slice(HALF, (char *)rgb,
		3*sizeof(half), width * 3 * sizeof(half), 1, 1, 0.0));
	frameBuffer.insert("G", Slice(HALF, (char *)rgb+sizeof(half),
		3*sizeof(half), width * 3 * sizeof(half), 1, 1, 0.0));
	frameBuffer.insert("B", Slice(HALF, (char *)rgb+2*sizeof(half),
		3*sizeof(half), width * 3 * sizeof(half), 1, 1, 0.0));

	file.setFrameBuffer(frameBuffer);
	file.readPixels(dw.min.y, dw.max.y);

	float max=0;

	if(HDRimg)
		delete [] HDRimg;
	HDRimg = new fRGB[width * height];

	for (int i = 0; i < width * height; ++i)
	{
		float frgb[3] = { rgb[3*i], rgb[3*i+1], rgb[3*i+2] };
		HDRimg[i].r = frgb[0]/adjust; if(HDRimg[i].r > max) max = HDRimg[i].r;
		HDRimg[i].g = frgb[1]/adjust; if(HDRimg[i].g > max) max = HDRimg[i].g;
		HDRimg[i].b = frgb[2]/adjust; if(HDRimg[i].b > max) max = HDRimg[i].b;
	}


	delete[] rgb;
	return max;
}

bool Exr::WriteEXR(string filename)
{
	int w = width; int h = height;
	Header header(w, h);
	header.channels().insert("R", Channel(HALF));
	header.channels().insert("G", Channel(HALF));
	header.channels().insert("B", Channel(HALF));

	OutputFile file(filename.c_str(), header);

	FrameBuffer frameBuffer;

	//split img int 3 buffers
	half *r = new half[w*h];
	half *g = new half[w*h];
	half *b = new half[w*h];

	for(int i=0; i<w*h; i++)
	{
		r[i] = HDRimg[i].r;
		g[i] = HDRimg[i].g;
		b[i] = HDRimg[i].b;
	}

	frameBuffer.insert("R", Slice(HALF, (char *)r, sizeof(*r), sizeof(*r)*w));
	frameBuffer.insert("G", Slice(HALF, (char *)g, sizeof(*g), sizeof(*g)*w));
	frameBuffer.insert("B", Slice(HALF, (char *)b, sizeof(*b), sizeof(*b)*w));

	file.setFrameBuffer(frameBuffer);
	file.writePixels(h);

	delete [] r;
	delete [] g;
	delete [] b;

	return true;
}



