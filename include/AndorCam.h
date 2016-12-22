// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, November 09, 2011</lastedit>
// ===================================
#pragma once

#include "Warn.h"
//#define cimg_imagemagick_path "C:\\ImageMagick\\convert.exe"
#define cimg_use_tiff

#include "Camera.h"
#include "CImg.h"
#include <cstdlib>
#include <process.h>
#include <vector>
#include "Utils.h"
#include "Scan.h"
#include <string>
#include "ThreadList.h"
//I know an image object may exist when the destructor is called
	//but this should be at the end of this program so we don't care about
	//this memory leak
class AcquisitionChannel;

class AndorCam:public Camera{
public:

	
	AndorCam(Trigger* t,int triggerLine, int deviceNum=0,int orientation=0);
	static int init();
	long camHandle;
	~AndorCam();//close and deinitialize AndorCam

//abstract camera class
	int deviceNum;
	float getPixelSize();
	int getBitDepth();
	HANDLE startSeries(int numImages,int seriesNum, Scan* scan, int camNum=-1);
	HANDLE startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan *s);
	double getTemp();
	void waitTemp();
	void startCooling();
	void stopCooling();
	//void saveSeries(int num,string fileName,const AcquisitionChannel &ac){this->dat2Tiff(num,fileName,ac);}
	double startFocusSeries(int num,AcquisitionParameters ap);
	double startStepwiseFocusSeries(int num, AcquisitionParameters ap);
	int getBestFocus();
	LONGLONG takePicture(AcquisitionChannel* ac, string fileName);
	void takeAccumulation(AcquisitionChannel* ac, int num, string fileName);
	void startLiveView(LiveViewData lvd);
	void waitLiveViewStart();
	void saveLiveImage(string fileName);
	void stopLiveView();
	bool isLiveView();
	double getImageWidth(AcquisitionParameters& ap);
	double getImageHeight(AcquisitionParameters& ap);
	void wait();
	bool validate(AcquisitionParameters& ap);
	void adjustIntensity(AcquisitionChannel& a,double percentSaturation=0.9);
	bool isTriggerable(AcquisitionParameters& ap);
	float getMinTriggerTime(AcquisitionParameters& ap);//exposure doesn't matter
	float standardMinTriggerTime;
//start actual implementation
	void terminateSeries();
	void abort();
	void getNumPixels(const AcquisitionParameters &ap,int& numXpixels,int& numYpixels);
	//int getNumXpixels(AcquisitionParameters ap);
	//int getNumYpixels(AcquisitionParameters ap);
	//void getImageRegion(int& minXpixel,int& minYpixel,int& maxXpixel,int& maxYpixel);
	HANDLE setup;
	static unsigned __stdcall setupThread(void* param);
	void waitSetup();
	static unsigned __stdcall dat2Tiff(void* param);//int num,string fileName,const AcquisitionChannel& ac);
	static unsigned __stdcall saveAccum(void* param);
	bool isPresent;
	float temp; //temperature to cool the AndorCam down to
	float exp;
	int gain;
	int bin;//binning factor
	bool first;
	bool takepic;
	LONGLONG freq;
	float minTrigTime;
	LARGE_INTEGER start;
	HANDLE focus;
	HANDLE live;
	HANDLE buttonWait;
	HANDLE liveViewStart;
	bool _isLiveView;
	HANDLE newImage;
	HANDLE CLEANONE;
	HANDLE triggerReady;
	int XPixels,YPixels;
	int minXPixel,maxXPixel,minYPixel,maxYPixel;
	//cimg_library::CImgDisplay liveDisp;//display window for live view
	string fileName;
	
	void closeAll();//close all open windows
	void setExposure(float exp);
	float getExposure();
	void setGain(int gain);
	int getGain();
	int getBin();
	void setBin(int bin);
	void takePicture(float exp=-1,int gain=-1, int bin=-1);//take a picture with the provided exp, gain and binning factor;
	//takePicture returns only after the exposure is complete
	void getImageRegion(const AcquisitionParameters& ap,int& minX,int& minY,int& maxX,int& maxY);
	//void setImageRegion(int opt, int percentX=100,int percentY=100); //0 will be full image,1=quadrant1, 2=quadrant2, 3=quadrant3,4=quadrant4, 5=middle quadrant
	static void showImage(cimg_library::CImg<unsigned short>* c,string title="Current Image");
	long spoolProgress();
	long totalNum();
	
	float getCoolTemp();
	bool isIdle();
	void waitIdle();
	static void showFile(string fileName);
	static unsigned __stdcall FileThread(void* param);
	static unsigned __stdcall ImageThread(void* param);
	static unsigned __stdcall LiveThread(void* param);
	static unsigned __stdcall ButtonWaitThread(void* param);
	static unsigned __stdcall FocusThread(void* param);
	static unsigned __stdcall SaveAccum(void* param);
	static void saveTiff(std::string fileName, cimg_library::CImg<unsigned short>* c, std::string descr="", bool asEightBit=false, int bitDepth=14);//save an array of pixel intensities to a tiff file with the given name
	static void saveTiff(cimg_library::CImg<unsigned short>* c,ImageProperties& ip);
	LONGLONG Kinetics();
	float startKineticsFTExtStart(string scanName, int num, float exp, int bin=-1, int gain=-1);//Frame Transfer Acquisition for scanning
	float startKineticsFTExtTrig(string scanName, int num, int bin=-1, int gain=-1);//Frame Transfer Acquisition for scanning
	float startFTKinetics(int num,float exp=-1,int bin=-1, int gain=-1);//Frame Transfer Acquisition for autofocus
	float startInternalFTKinetics(string scanName,int num, float exp, int bin=-1,int gain=-1);
	float startNonFTKineticsExtTrig(int num, float exp,int bin,int gain);//used for stepwise autofocusing
	cimg_library::CImg<unsigned short>* getNewPic();//read out the data from the buffer into memory
	cimg_library::CImg<unsigned short>* getOldest();
	static void getScore(CImg<unsigned short>* Cimage,LONGLONG* sum1);
	static unsigned __stdcall getScore(void* param);//cimg_library::CImg<unsigned short>* Cimage,LONGLONG* score);
	static unsigned __stdcall threadGroup(void* param);
	bool focusing;
	int bitDepth;
	int focusNum;

	void testMultipleCameras();
	void testStop();
	CImg<unsigned short>* testGetPic();
private:
	float pixelSize;
	int minGain,maxGain;
	//refresh rate for live view in ms
	void coolDown();//switch cooler on and set AndorCam temperature to global temp variable
	void heatUp();//switch off cooler and wait for temperature to go above 0 deg C.

	//this next line may not be needed if we have the Cimage object
	//unsigned short image[1024*1344];//actually think of this as a double array [1024][1344]	

	void getError(unsigned int);
	void adjustOrientation(CImg<unsigned short>* img);
};
struct Score{
	cimg_library::CImg<unsigned short>** images;
	int ind;
	LONGLONG* result;
	double* times;
};


struct ThreadGroup{
	int num;
	HANDLE threads[MAXIMUM_WAIT_OBJECTS];
};

struct Accum{
	int num;
	string fileName;
	AndorCam* c;
	AcquisitionChannel* ac;
};