#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

#include "loadraw.h"
#include "rgbe.h"

//min and max depend on the camera
void pixweight(ushort pix[3], float w[3], float val[3], float min=0.0f, float max=255.0f)
{
	for(int i=0; i<3 ; i++)
	{
		val[i] = ((float)(pix[i]-min)) / (max-min);
		if(val[i]<0.0) val[i]=0.0;
		if(val[i]>1.0) val[i]=1.0;
		w[i] = 1-pow(2*val[i]-1, 12);
	}
}

void rawpixel(ushort *rawLDR, int i, int j, int raww, int rawh, ushort pix[3])
{
	int ndx = (i*raww + j)*4;

	unsigned int r,g,b;

	//Very simple demosaicing using a 2x2 block:
	// R  G
	// G2 B
	//All 4x4 pixel blocks have the same value

	if(i%2) //odd lines
	{
		if(j%2) //B
		{
			r = rawLDR[ndx-raww*4-4];
			g = rawLDR[ndx-4+3];
			b = rawLDR[ndx+2];
		}
		else  //G2
		{
			r = rawLDR[ndx-raww*4];
			g = rawLDR[ndx+3];
			b = rawLDR[ndx+4+2];
		}
	}
	else //even lines
	{
		if(j%2) //G
		{
			r = rawLDR[ndx-4];
			g = rawLDR[ndx+1];
			//this is to handle odd height images
			((ndx+raww*4+2)<(raww*rawh*4))? b = rawLDR[ndx+raww*4+2] : b=0;
		}
		else //R
		{
			r = rawLDR[ndx];
			g = rawLDR[ndx+5];
			//TODO: b=0 is wrong but only affects last line in image
			((ndx+raww*4+6) < (raww*rawh*4))? b = rawLDR[ndx+raww*4+6] : b=0;
		}
	}

	pix[0]=r; pix[1]=g; pix[2]=b;
}

int main(int argc, char **argv)
{
	if(argc<2)
	{
		cout << "Assemble LDR images into HDR\n ";
		cout << "Usage: asmHDR [image list]\n" ;
		cout << "[image list] has one line for each LDR image. Each line is of the format:\n";
		cout << "[filename] [aperture] [exposure time]\n";

		return 404;
	}

	//count images
	FILE *f=fopen(argv[1],"rb");
	int c=0,b;  while ((b=fgetc(f))!=EOF) c+=(b==10)?1:0;
	fclose(f);

	cout << "files:"<<c<<endl;

	//place to store data
	ushort **LDRs = new ushort*[c];
	float *Avs = new float[c];
	float *Tvs = new float[c];
	int w, h;

	//load all input images
	ifstream filelist;
	filelist.open(argv[1], fstream::in);
	char filename[256];

	int l=0;
	while(filelist.good())
	{
		filelist >> filename;
		filelist >> Avs[l];    //aperture value
		filelist >> Tvs[l];    //exposure time
		LDRs[l] = loadraw(filename, &w, &h);
		cout << "Loaded "<< filename << " " << Avs[l] << " " << Tvs[l] <<endl;
		l++;
		if(l>c) break;
	}

	//Debevec method:

	float tmpresult[3];
	float sumweights[3];
	float *imaResult = new float[w*h*4]; //HDR image

	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			for(int t=0; t<3; t++){ tmpresult[t]=0; sumweights[t]=0; }

			//loop over the images and calculate the HDR for current pixel
			for(int k=0; k<c ; k++)
			{
				ushort pix[3];
				float weights[3];      //weights
				float floatvals[3];    //pixel values normalized to [0,1]

				rawpixel(LDRs[k], i, j, w, h, pix);
				pixweight(pix, weights, floatvals, 2030, 13586); //values for canon EOS10D

				for(int t=0; t<3 ; t++)
				{
					tmpresult[t] += weights[t]*floatvals[t] / Tvs[k];
					sumweights[t] += weights[t];
				}
			}

			for(int t=0; t<3 ; t++)
			{
				imaResult[ (i*w+j)*4+t ] = tmpresult[t] / sumweights[t];
			}
			imaResult[ (i*w+j)*4+3 ] = 0.0;
		}
	}

	FILE* fp;
	if(argc<3)
		fp = fopen("a.hdr","w+b");
	else
		fp = fopen(argv[2],"w+b");

	// Writing of the .hdr header
	RGBE_WriteHeader(fp, w, h, NULL);

	// Writing of the pixels of the final image in the .hdr
	RGBE_WritePixels(fp,imaResult,w * h);

	fclose(fp);

	for(int i=0; i<c; i++) delete [] LDRs[i];
	delete [] LDRs;
	delete [] Tvs;
	delete [] Avs;
	delete [] imaResult;


	return 0;
}
