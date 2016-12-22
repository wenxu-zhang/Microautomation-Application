// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Thursday, November 17, 2011</created>
// <lastedit>Tuesday, December 06, 2011</lastedit>
// ===================================
#pragma once
#include "Chamber.h";
class Syringe;
class Valve;
#include "DaisyValve.h"
#include <string>
class FlowChannel{
public:
	FlowChannel(Syringe* s,Chamber cham,Valve* vPull=NULL,Valve* vPush=NULL);//does not throw exception
	FlowChannel(std::string name,Syringe* s,int syringePos,Chamber cham,double injectTubingVolumePull,Valve* vPull=NULL,int vPullPort=0,double injectTubingVolumePush=0,Valve* vPush=NULL,int vPushPort=0, DaisyValve dvPull=DaisyValve(),DaisyValve dvPush=DaisyValve());//real could throw exception
	Chamber cham;
	std::string name;
	double injectTubingVolumePush;
	double injectTubingVolumePull;
	int syringePos;
	int vPullPort;
	int vPushPort;
	Valve* vPush;
	Valve* vPull;
	DaisyValve dvPull;
	DaisyValve dvPush;
	Syringe* s;
	void select();
	double getInjectTubingVolPush(){return injectTubingVolumePush;}//tubing volume from outPort of Push DaisyValve to chamber entrance 
	double getInjectTubingVolPull(){return injectTubingVolumePull;}//tubing volume from outPort of Push DaisyValve to chamber entrance 
	double getSyringeVolume(FlowChannel* chan);
	void pull(double uL,double uLps,double msPause=2000);
	void push(double uL,double uLps,double msPause=2000);
	void waste();
	void wait();
	bool conflict(FlowChannel& s);
	std::string toString();
	FlowChannel parseLine(std::string s);
};