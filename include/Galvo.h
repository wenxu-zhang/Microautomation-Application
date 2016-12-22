// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, June 20, 2011</lastedit>
// ===================================
#pragma once
#include "Trigger.h"
#include "Laser.h"
class Galvo:public Changer, public Laser{
public:
//	static const int MAXPOWER=-2;
//	static const int MINPOWER=-3;
//	static const int MEDPOWER=-4;
	Galvo(Trigger* t,std::string paramsFile,int GALVOPOSIN=-1,int GALVOVELIN=-1);
	void set(double pos);
	double get();
	//void wait();
	bool analogIn;//does the daq board support reading voltage from galvo?

	//implementation of abstract LightSource class
	//void on(int pos, double intensity);
	//void off();
	//void enterRingBuffer(AcquisitionGroup& ag){}
	//void exitRingBuffer(){}
//	ConstParams loadParams(std::string name);
//	double mW2Volts(double mW);
//	double volts2mW(double volts);
//	bool calibrate(std::vector<double> &x,std::vector<double> &y);
//	std::string intensityToString(double intensity);
//	double getIntensity(double intensity=-1,std::string intensityUnit="");
	//bool isValidIntensity(double intensity){if (intensity==DGFULL || intensity==DGHALF || intensity==DGTHIRD) return true; else return false;}	
private:
	const int GALVOPOSIN;
	const int GALVOVELIN;
	static const double VELTHRESH;
	static const double POSTHRESH;
	static const double LINEARITY;

};