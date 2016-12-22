// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "AcquisitionChannel.h"

class ZStage;

class Focus{
public:
	AcquisitionChannel a;
	ZStage* z;
	double range;//total search area in um
	double step;//size of each step in um
	double getInitialFocus(bool moveAfterFocus=true,bool wait=true);//ignores range and searches entire space for focus using stepSize provided
	double getFocus(bool moveAfterFocus=true, double center=-1,bool wait=true, int mode=0);//use the provided center (default is current position) 
	double getFocus(bool moveAfterFocus, double center,bool wait,int mode,bool& isInRange);//use the provided center (default is current position) 
	void adjustIntensity(double percentSaturation=0.5);
	//double getInitialTIRangle(bool moveAfter=true,double center=-1,bool wait=true);
	Focus();
	Focus(AcquisitionChannel a,double range, double stepSize);
	Focus(AcquisitionChannel a,double range, int steps);
	Focus(AcquisitionChannel a,double range=20);

	void modify();

private:
	//select correct function
	double getFocus(bool&isInRange,double range, double step, double center, int mode);
	//mode 0
	double getContinuousFocus(bool& isInRange,double range,double step,double center);
	//mode 1
	double getStepwiseFocus(bool& isInRange,double range,double step,double center);
	//mode 2
	double getDefiniteFocus(bool& isInRange,double range,double step,double center);
};
