// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#pragma once
class AcquisitionGroup;//#include "AcquisitionGroup.h"
#include "Trigger.h"//#include "Trigger.h"
#include "Record.h"
class Channel;
#include "nr.h"
extern Record logFile;
class ConstParams{
public:
	int analogTriggerLine;
	int triggerLine;
	int triggerOption;
	double delay;
	double zeroIntensity;
	std::string calChan;
	double wavelength;
	ConstParams(){}
	ConstParams(int triggerLine, double delay, int analogTriggerLine,int triggerOption,double zeroIntensity,double wavelength=500,std::string calChan="None"):
				triggerLine(triggerLine),delay(delay),analogTriggerLine(analogTriggerLine),
																									triggerOption(triggerOption),zeroIntensity(zeroIntensity),calChan(calChan){}
};

class NonConstParams{
public:
	double defaultIntensity;
	string units;
	NonConstParams(){}
	NonConstParams(double defaultIntensity):defaultIntensity(defaultIntensity){}
};

struct Params{
	const ConstParams * cp;
	NonConstParams* ncp;
	Params(const ConstParams* cp,NonConstParams* ncp):cp(cp),ncp(ncp){}
};
class Objective;
class LightSource{
protected://constructors are protected to prevent someone from creating a LightSource directly. Why would you need to do this? Just have a nonpresent lightsource
	//constructor must make sure the triggerLine is valid
	LightSource(Trigger* t=NULL,string name="",int triggerLine=0, double defaultIntensity=0,double delay=0, int analogTriggerLine=-1,int triggerOption=0,double zeroIntensity=0);
	LightSource(Trigger* t, string name,Params& p);
public:
	virtual ~LightSource();
	
		void trigger();
	
	virtual void on(int pos, double intens);
	virtual void off();
	virtual void wait();
	//This function should take in an AcquisitionGroup and prepare positions for those channels that use this light source while inserting off/blank positions for those that do not have this light source
	//if the ring buffer method is not supported by the light source (e.g. lasers) then this will be checked by the supportsRingBuffer method and the appropriate trigger will be sent 
	virtual void enterRingBuffer(AcquisitionGroup& ag);
	virtual bool supportsRingBuffer();
	virtual void exitRingBuffer();//exit ring buffer if necessary
	virtual double getIntensity(double intensity=-1,std::string intensityUnit="",Objective* obj=NULL)=0;//passing no parameters will give default intensity for this light source default intensityUnit "" is chosen by the LightSource "V" for halogen "mW" for laser  and "f" (fraction from 0 to 1) for DG, "%" should also be supported.
	virtual std::string intensityToString(double intensity,Objective* obj=NULL){return "UnknownIntensity";}
	virtual double getPower(double intensity, string& units,Objective* obj=NULL){"UnknownPowerUnits";return 0;}
	//virtual bool isValidIntensity(double intensity=-1, std::string intensityUnit="")=0;
	bool isTriggerable();
	bool operator==(LightSource& right);
	virtual double getObjEff(Objective* obj){return 1;}
	//use the power meter to calibrate this light source based on the input channel (the input channel light source must be the same as "this" light source") and save the results to a text file if necessary (i.e. for lasers)
	virtual void calibrate();//full calibration, step all points 
	virtual void quickCalibrate();//get max power and scale all the calibration points accordingly, not as accurate as full calibration
	virtual double checkMaxPower(){return 1;}//return ratio of current max power to the max power after last alignment. if less than 80% an alignment should be performed
	virtual void onAlign(){return;}
	virtual bool prepareCalChan(){return false;}
	virtual void endCalChan(){return;}
	static LightSource* select();

	/*
		const int analogTriggerLine;
	const int triggerLine;//digital
	const int triggerOption;//0 is digital, 1 is analog, 2 is both and -1 is reserved for a pause during FT kinetics scans
	const double delay;//default delay before exposure starts in ms

	const std::string calChan;
	const double wavelength;//excitation wavelength for calibration
	double defaultIntensity;
	const double zeroIntensity;//might not be zero e.g. galvo   //can only be set in the constructor for this lightsource
	*/
	bool isPresent;
	Trigger * t;
	std::string name;
	const ConstParams* const cp;
	double defaultIntensity(Objective* obj){return getIntensity(ncp->defaultIntensity,ncp->units,obj);}
protected:
	NonConstParams* ncp;
	/*
	LightSource operator=(LightSource right){
		return right;
	}
	*/
};

