// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#pragma once
#include <string.h>
class Objective;
class RS232;
using namespace std;
class AcquisitionChannel;
class PowerMeter{
	friend class Controller;
public:
	bool isPresent;
	bool isDarkCurrentSet;
	PowerMeter(int COM,double zpos, Objective* obj);//open comm communications and establish default values (sensor type, degrees celsius etc.)
	~PowerMeter();//close comm communications
	void abort();
	void setWavelength(int nm);
	double getWavelength();
	
	double getPower(AcquisitionChannel* ac);
	double readPower();
	void setObjective();
	string checkError();

	//gui
	void powerMeterControl();
private:
	static const int numReadingsDark=10;
	static const int numReadingsPower=10;
	RS232* pm100;
	double zpos;
	Objective* obj;
	void setDarkCurrent();
};