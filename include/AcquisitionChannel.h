// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, November 14, 2011</lastedit>
// ===================================
#pragma once
#include "AcquisitionParameters.h"
#include "OutPort.h"
//#include "Controller.h"
#include <vector>
class Camera;
class Channel;
class Magnification;

class AcquisitionChannel{
public:
	Channel* chan;
	OutPort* out;//so we can take the same channel with different cameras for comparison
	Magnification* m;
	AcquisitionParameters ap;
	void validate();
	double intensity;
	//not yet  we would like the power to remain constant when switching objectives std::string units;//hopefully calculating actual intensity will not take that long
	double TIRFangle;
	bool showOnScreen;
	bool showScaleBar;
	void on(bool wait=true);
	void off(bool wait=true);
	void wait();
	void adjustIntensity(double percentSaturation=0.9);
	void takePicture(std::string fileName);
	void takeAccumulation(int num, std::string fileName);
	bool isTriggerable();
	static void takeMultiplePics(std::vector<AcquisitionChannel>& channels,std::string prefix="",std::string suffix="");
	static void takeMultipleAccumulations(std::vector<AcquisitionChannel>& channels, int num=2,std::string prefix="",std::string suffix="");
	AcquisitionChannel(const AcquisitionChannel&);
	AcquisitionChannel& operator=(const AcquisitionChannel& ac);
	AcquisitionChannel();
	AcquisitionChannel(Channel* chan, OutPort* out,Magnification* m);
	AcquisitionChannel(Channel* chan, OutPort* out,Magnification* m, AcquisitionParameters& ap,double lightIntensity=-1,std::string intensityUnits="",bool showOnScreen=false,bool showScaleBar=false,double TIRFangle=-20);
	AcquisitionChannel(int channel, int out,int mag,AcquisitionParameters& ap=AcquisitionParameters(),double intensity=-1,std::string intensityUnits="",bool showOnScreen=false, bool showScaleBar=false,double TIRFangle=-20);
	std::string toString();
	double getFOVHeight();
	double getFOVWidth();
	bool operator==(const AcquisitionChannel &a2) const{
		return this->chan==a2.chan && this->out==a2.out && this->m==a2.m && this->ap==a2.ap && this->intensity==a2.intensity && this->showOnScreen==a2.showOnScreen
			&& this->showScaleBar==a2.showScaleBar;
	}
	bool operator!=(AcquisitionChannel &right){
		return (!(this->operator==(right)));
	}
	//gui
	void modify();
	static std::vector<AcquisitionChannel> getAcquisitionChannels();
	static bool identical(std::vector<AcquisitionChannel> vac);
};