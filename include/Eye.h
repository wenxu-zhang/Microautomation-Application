// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Camera.h"
//the default camera is our eye
class Eye:public Camera{
public:
	int num;
	void terminateSeries(){}
	void abort(){}
	Eye():Camera(NULL,vector<int>(1,0)){}
	void adjustIntensity(AcquisitionChannel &,double){}
	HANDLE startSeries(int numImages,int seriesNum, Scan* s,int camNum=-1){return 0;}
	HANDLE startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan* s){return 0;}
	double startFocusSeries(int num,AcquisitionParameters ap){this->num=num;return ap.exp;}//needs trigger to start
	double startStepwiseFocusSeries(int num, AcquisitionParameters ap){this->num=num;return ap.exp;}
	int getBestFocus(){return num/2;}//return index to best image
	LONGLONG takePicture(AcquisitionChannel* , string fileName){return 0;}
	void takeAccumulation(AcquisitionChannel* ac,int num,string fileName){}
	void startLiveView(LiveViewData lvd){}
	void waitLiveViewStart(){}
	void saveLiveImage(string fileName){}
	void stopLiveView(){}
	bool isLiveView(){return false;}
	double getImageWidth(AcquisitionParameters& ap){return 18000.0/sqrt((double)2);}//18mmFOV diameter
	double getImageHeight(AcquisitionParameters& ap){return 18000.0/sqrt((double)2);}//18mmFOV diameter
	void wait(){return;}
	bool validate(AcquisitionParameters &ap){return true;}
	bool isTriggerable(AcquisitionParameters &ap){return false;}
	double getTemp(){return 20;}
	void waitTemp(){}
	float getPixelSize(){return 78;}//iPhone 4 retina display
	int getBitDepth(){return 8;}//only ~200:1 at any given time but it can quickly adjust
};
