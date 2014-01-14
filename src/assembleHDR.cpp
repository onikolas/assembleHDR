#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <boost/program_options.hpp>


using namespace std;

#include "loadraw.h"
#include "rgbe.h"
#include "exr.h"

//min and max depend on the camera
void pixweight(ushort pix, float *w, float *val, unsigned int min, unsigned int max, float cutoff)
{

	*val = ((float)(pix-min)) / (max-min);
	if(*val<0.0) *val=0.0;
	if(*val>1.0) *val=1.0;
	*w = 1-pow(2*(*val)-1, 12); //TODO check paper
	if(*w<cutoff) *w=0;
}


//4x4 silly filter
fRGB * demosaic(float *img, int w, int h, fRGB matrix, bool scale)
{
	fRGB *ret;
	if(scale)
		ret = new fRGB[w*h/4];
	else
		ret = new fRGB[w*h];



	for(int a=0; a<h; a+=2)
	{
		for(int c=0; c<w; c+=2)
		{
			int i = a*w+c;

			int r = i;
			int g = i+1;
			int g2 = (i+w)%(w*h);
			int b = (i+w+1)%(w*h);

			float red = img[r]*matrix.r;
			float green = (img[g] + img[g2])*matrix.g/2;
			float blue = img[b]*matrix.b;

			if(scale)
			{
				int ii = (a/2)*(w/2)+(c/2);
				ret[ii].r = red;
				ret[ii].g = green;
				ret[ii].b = blue;
			}
			else
			{
				ret[i].r =  ret[g].r = ret[g2].r = ret[b].r = red;
				ret[i].g =  ret[g].g = ret[g2].g = ret[b].g = green;
				ret[i].b =  ret[g].b = ret[g2].b = ret[b].b = blue;
			}
		}
	}

	return ret;
}

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	po::variables_map vm;
	try
	{
		po::options_description desc("Options");
		desc.add_options()
			("help", "print help message")
			("inputs",   po::value< vector<string> >()->required(),     "input files" )
			("output,o",   po::value<string>()->default_value("out.exr"), "output filename to use")
			("filetype,f", po::value<string>()->default_value("exr"),     "output is .hdr or .exr")
			("black,b",    po::value<int>()->default_value(2040),         "black level")
			("white,w",    po::value<int>()->default_value(13586),        "white level")
			("cutoff,c",   po::value<float>()->default_value(0),          "ignore suaturated values (enter %)")
			("scale,s", 												  "scale to 25%")
		;
		po::positional_options_description p;
		p.add("inputs", -1);

		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		if(vm.count("help"))
		{
			cout << "\nAssemble a set of RAW LDR images into HDR\n\n";
			cout << "Useage: assembleHDR [options] ldr-files\n\n";
			cout << desc << "\n";
			return 1;
		}
		po::notify(vm);
	}
	catch(std::exception& e)
	{
		std::cerr << "Error: " << e.what() << " - Use --help for command line options\n";
		return 1;
	}

	//count images
	std::vector<string> files = vm["inputs"].as< vector<string> >();
	int c = files.size();

	cout << "files:"<<c<<endl;

	//place to store data
	ushort **LDRs = new ushort*[c];
	float *Avs = new float[c];
	float *Tvs = new float[c];
	int w, h;

	//load all input images
	for(int i=0; i<c; i++)
	{
		float av,tv;
		LDRs[i] = loadraw(files[i].c_str(), &w, &h, &av, &tv);
		Avs[i] = av; Tvs[i] = tv;
		//TODO test w,h differences
		cout << "Loaded "<< files[i].c_str() << " Av:" << Avs[i] << " Tv:" << Tvs[i] <<endl;
	}


	//Debevec method:
	cout << "Merging into HDR\n";

	float tmpresult;
	float sumweights;
	float *imaResult = new float[w*h]; //HDR image before demosaic and cropping

	for(int i=0; i<w*h; i++)
	{
		tmpresult=0; sumweights=0;

		//loop over the images and calculate the HDR for current pixel
		for(int k=0; k<c ; k++)
		{
			ushort pix = LDRs[k][i];
			float weight;      //weights
			float floatval;    //pixel values normalized to [0,1]

			int black = vm["black"].as<int>();
			int white = vm["white"].as<int>();
			float cutoff = vm["cutoff"].as<float>();
			pixweight(pix, &weight, &floatval,black, white, cutoff);
			tmpresult += weight*floatval / Tvs[k];
			sumweights += weight;
		}

		imaResult[i] = (sumweights>0)?(tmpresult / sumweights):0;

	}

	if(vm["filetype"].as<string>() != "exr")
	{
		std::cerr << "Only exr output for now\n"; return -1;
	}

	/*FILE* fp;
	if(argc<3)
		fp = fopen("a.hdr","w+b");
	else
		fp = fopen(argv[2],"w+b");

	// Writing of the .hdr header
	RGBE_WriteHeader(fp, w, h, NULL);

	// Writing of the pixels of the final image in the .hdr
	RGBE_WritePixels(fp,imaResult,w * h);

	fclose(fp);
	*/

	cout << "Demosaic ";
	bool scale=false;
	int outw = w;
	int outh = h;
	if(vm.count("scale"))
	{
		cout << "and scaling to 25% of original\n";
		outh=h/2;
		outw=w/2;
		scale = true;
	}
	else
		cout << "\n";

	//demosaicing of imaResult into tmpexr
	fRGB *tmpexr = demosaic(imaResult,w,h,fRGB(1,1,1),scale);

	Exr outfile;
	outfile.FromArray(tmpexr,outw,outh);
	outfile.WriteEXR(vm["output"].as<string>());

	cout << "Wrote: " << vm["output"].as<string>() <<endl;

	for(int i=0; i<c; i++) delete [] LDRs[i];
	//delete [] LDRs;
	delete [] Tvs;
	delete [] Avs;
	//delete [] imaResult;
	delete [] tmpexr;


	return 0;
}
