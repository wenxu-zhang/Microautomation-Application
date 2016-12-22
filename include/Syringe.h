// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, December 19, 2011</lastedit>
// ===================================
/*
Syringe: Rocket Pump 9-port syringe pump
		-Constructor: initialize syringe position
			Input: 	Device number
			            Volume
				Max step
		-pull: bring syringe down desired volume.  May need to expel to waste for larger volumes
			Input:  volume in microliters
				speed in millimeters per second through the chamber
			Output: string command
		-push: bring syringe up desired volume.  Should never need to bring more solution into syringe
			Input:  volume in microliters
				speed in millimeters per second through the chamber
			Output: string command
		-waste: expel contents into waste line
			Output: string command

*/
#pragma once
#include <atlbase.h>
#include "Solution.h"
class Pump;
#include <string>

class Syringe{
public:
	//class variables
	int devNum;
	double volume;//in uL
	int maxStep;
	int maxStepNormal;
	int maxStepFine;
	int wastePos;
	bool isPresent;
	bool isInitialized;
	Pump* pmp;
	CComBSTR b;//so we don't have to allocate it every time
	//class methods
	Syringe():isPresent(false){}
	Syringe(Pump* pmp,int deviceNum,double volume,int maxStepNormal,int maxStepFine, int wastePos);
	void pull(int valvePos,double volume, double uLps,bool wait=true,double msPause=2000);//for rocket pump
	void push(int valvePos,double volume, double uLps,bool wait=true,double msPause=2000);//for rocket pump
	std::string _pull(int valvePos,double volume, double uLps, double* time=NULL,double msPause=2000);//for rocket pump 
	std::string _push(int valvePos,double volume, double uLps,double msPause=2000);//for rocket pump
	std::string moveValve(int num);
	std::string _waste();
	void waste(bool wait=true);
	void stop();
	void wait();
	int getPosition();
	void enableMicrostepping();
	void disableMicrostepping();
	double steps2vol(int steps);
	 
		int getStartVelocity();
		int getCutoffVelocity();
		int getTopVelocity(int Hz);
		void setSlope(int val);
		int slopeCode;
		double fineControl(int syringePort,double uLps);
	//class variables

	//class methods
	void init();
	std::string toString();
	void sendCommand(std::string st);
	std::string speed2Hz(double mmps);
	std::string flow2Hz(double uLps);
	std::string vol2steps(double uL);
	int vol2stepsInt(double uL);
	int flow2HzInt(double uLps);
private:
		double calcTime(double uLps,double vol);
	
};
