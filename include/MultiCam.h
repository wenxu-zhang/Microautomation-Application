// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once

#include "Warn.h"

#include "Camera.h"
#include <vector>

class MultiCam:public Camera{
public:
	MultiCam(std::vector<Camera*> cameras);
	~MultiCam();
	void abort();
//abstract camera class
	float getPixelSize();
	int getBitDepth();
	std::vector<HANDLE> startSeriesThreads;
	std::vector<HANDLE> saveThreads;
	//void terminateSeries();
	HANDLE startSeries(int numImages,int seriesNum, Scan* scan,int camNum=-1);
	HANDLE startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan *s);
	double getTemp();
	void waitTemp();
	//void saveSeries(int num,string fileName,const AcquisitionChannel &ac){this->dat2Tiff(num,fileName,ac);}
	double startFocusSeries(int num,AcquisitionParameters ap);
	double startStepwiseFocusSeries(int num, AcquisitionParameters ap);
	int getBestFocus();
	LONGLONG takePicture(AcquisitionChannel* ac, std::string fileName);
	void takeAccumulation(AcquisitionChannel* ac, int num, std::string fileName);
	
	void waitLiveViewStart();
	void saveLiveImage(std::string fileName);
	void stopLiveView();
	bool isLiveView();
	double getImageWidth(AcquisitionParameters& ap);
	double getImageHeight(AcquisitionParameters& ap);
	void wait();
	AcquisitionParameters getDefaultAcquisitionParameters();
	bool validate(AcquisitionParameters& ap);
	void adjustIntensity(AcquisitionChannel& a,double percentSaturation=0.9);
	bool isTriggerable(AcquisitionParameters& ap);

protected:
	void startLiveView(LiveViewData lvd);

private:
	static unsigned __stdcall startSeriesThread(void* param);
	static unsigned __stdcall masterThread(void* param);
	static unsigned __stdcall masterThread2(void* param);
	std::vector<Camera*> cameras;
};