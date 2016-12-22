// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, February 24, 2011</lastedit>
// ===================================
#pragma once
#include "ImageRegion.h"
#include "Trigger.h"
#include <string>
#include <vector>
#include "Utils.h"
#include <math.h>
#include "AcquisitionChannel.h"
#include "Selection.h"
#include <string>
#include "CImg.h"
class Scan;

struct LiveViewData{
	int x;
	int y;
	int width;
	int height;
	std::vector<double>* scaleFactor;
	std::vector<double>* centerX;
	std::vector<double>* centerY;
	double* zoom;
	double* centerXEff;
	double* centerYEff;
	int camNum;
	LiveViewData(int x,int y, int width, int height, std::vector<double>* scaleFactor,std::vector<double>* centerX,	std::vector<double>* centerY,double* zoom, double* centerXEff,double* centerYEff,int camNum):x(x),y(y),width(width),height(height),scaleFactor(scaleFactor),centerX(centerX),centerY(centerY),zoom(zoom),centerXEff(centerXEff),centerYEff(centerYEff),camNum(camNum){}
};

struct LiveViewArg{
	Camera* a;
	LiveViewData lvd;
	LiveViewArg(Camera* a,LiveViewData lvd):a(a),lvd(lvd){}
};

class Camera{

protected:
	HANDLE getAbortEvent(Scan *s);
	//Orientation:
	//  0- normal like left port using Andor iXon DU885
	//  1- flip image vertically compared to option 0 e.g. transmitted right port of AxioObserver with dualCam using Andor iXon DU885
	//  2- flip image horizontally compared to option 1 (vertical and horizontal compared to option 0) e.g. reflected right port of AxioObserver with dualCam using Andor 
	Camera(Trigger* t,std::vector<int> triggerLines,int orientation=0);
	
	std::vector<double> scaleFactor;//for live view zooming feature
	std::vector<double> centerX;//for live view zooming feature between -.5 and .5
	std::vector<double> centerY;//for live view zooming feature between -.5 and .5
	double zoom,centerXEff,centerYEff;

	static double getZoom(std::vector<double>& scaleFactor);
	static void getCenter(LiveViewData& lvd);
	static cimg_library::CImg<unsigned short> getAdjustedImage(cimg_library::CImg<unsigned short>& original, double scale, double centerX, double centerY, int width, int height,double& finalScale,int& beforeX,int& afterX, int& beforeY, int& afterY);
	static cimg_library::CImg<unsigned short> getFinalImage(LiveViewArg& lva, cimg_library::CImg<unsigned short>& copyImageDisp,double zoom,double mag);

public:
	
	int orientation;
	//virtual void terminateSeries()=0;
	virtual void abort()=0;
	virtual float getPixelSize()=0;
	virtual int getBitDepth()=0;
	std::vector<std::string> name;
	Trigger* t;
	std::vector<int> triggerLines;//lines corresponding to camera triggers
	//real methods
	void trigger();
	//virtual methods
	virtual ~Camera()=0;
	//camNum is -1 for single camera acquisitions i.e. not multicam
	virtual HANDLE startSeries(int numImages,int seriesNum,Scan* s, int camNum=-1)=0;//exposure times determined by external triggers and must be checked by isTriggerable. Will use a temporary folder to save files to.  The folder will be deleted before each acquistion  
	virtual HANDLE startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan* s)=0;//exposure times determined internally (frees up daq board to do other things), still needs start trigger;
	virtual double getTemp()=0;
	virtual void waitTemp()=0;
	virtual void startCooling(){};
	virtual void stopCooling(){};
	//virtual void saveSeries(int num, string fileName);//This will save the appropriate file to the corresponding file name as a tiff
	virtual double startFocusSeries(int num,AcquisitionParameters ap)=0;//needs trigger to start
	virtual double startStepwiseFocusSeries(int num, AcquisitionParameters ap)=0;//needs trigger before each image
	virtual int getBestFocus()=0;//return index to best image
	virtual LONGLONG takePicture(AcquisitionChannel* ac,std::string fileName="temp")=0;
	//virtual void beginTakePicture(AcquisitionChannel* ac,std::string fileName="temp")=0;
	virtual void takeAccumulation(AcquisitionChannel* ac, int num,std::string fileName)=0;
	
	virtual void waitLiveViewStart()=0;
	virtual void saveLiveImage(std::string fileName="temp")=0;
	virtual void stopLiveView()=0;
	virtual bool isLiveView()=0;//is live view on?
	virtual double getImageWidth(AcquisitionParameters &ap)=0;//return image width in microns
	virtual double getImageHeight(AcquisitionParameters &ap)=0;//return image height in microns
	virtual void wait()=0;
	virtual void adjustIntensity(AcquisitionChannel& ac, double percentSaturation=0.9)=0;

	//check gain , bin, image region and exposure for compatibility with this camera
	virtual bool validate(AcquisitionParameters& ap)=0;//verify these acquisition parameters can be achieved with this camera
	virtual AcquisitionParameters getDefaultAcquisitionParameters(){return AcquisitionParameters();}
	//check if the minimum trigger time for a kinetic series is compatible with these acquisition parameters...if not need to use takePicture
	virtual bool isTriggerable(AcquisitionParameters& ap)=0;
	
	void startLiveView();//only called on top level cameras (e.g. dual cam)
	virtual void startLiveView(LiveViewData lvd)=0; //called on low level cameras like cam1 and cam2 of the dual cam

	//std::string toString(int n=-1);
};


