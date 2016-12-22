// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, November 15, 2011</created>
// <lastedit>Thursday, April 05, 2012</lastedit>
// ===================================
#pragma once
#include <string>
class Pump;
class Valve{
public:
	Valve(Pump* pmp,int devNum):pmp(pmp),devNum(devNum),pos(1){}
	Valve():isPresent(false),pos(1){}
	//Valve():isInitialized(false){}
	int devNum;
	int pos;//cannot query position of valve on rocket pump. on smart valve the command ?0 would do it, but we will just keep track of the valve position so that it works for both the rocket pump and the smart valve
	virtual void select(int valvePos,double msWait=1000,int dir=0);
	virtual void wait();
	Pump* pmp;
	bool isPresent;
	virtual void init();
	virtual std::string toString();
	
	
};