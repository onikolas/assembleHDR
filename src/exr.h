#ifndef EXR_H_
#define EXR_H_

#include <string>
using namespace std;

struct fRGB
{
	fRGB() {r=0;g=0;b=0;}
	fRGB(float i,float j,float k) {r=i;g=j;b=k;}
	float r,g,b;
};

struct dRGB
{
	dRGB() {r=0;g=0;b=0;}
	dRGB(double i,double j,double k) {r=i;g=j;b=k;}
	double r,g,b;
};

class Exr
{
	public:
	Exr();
	Exr(string filename);
	Exr(const Exr &a);
	~Exr();

	fRGB At(int index){ return HDRimg[index]; }
	fRGB At(int w, int h){ return HDRimg[h*width+w]; }
	void Set(int index, fRGB a) {HDRimg[index]=a;}
	int Width(){return width;}
	int Height(){return height;}
	int Size(){return width*height;}
	void FromArray(fRGB *a, int width, int height);
	void Init(int w, int h, fRGB c);
	bool IsEmpty();

	int ToneMapEXR(float i, float max=1);
	float ReadEXR(string filename, float adjust);
	//fRGB * ScaleEXR(fRGB *HDRimg, int ow, int oh, int nw, int nh);
	bool WriteEXR(string filename);

	private:
	fRGB *HDRimg;
	int width, height;
};

#endif /* EXR_H_ */
