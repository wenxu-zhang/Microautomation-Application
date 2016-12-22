// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, November 15, 2011</created>
// <lastedit>Thursday, April 05, 2012</lastedit>
// ===================================
#include "Valve.h"
#include "Pump.h"
#include "Definitions.h"
#include "Utils.h"
#include "Record.h"
extern Record logFile;
void Valve::init(){
	CheckExists()
	pmp->sendCommand("Z",devNum);
	pos=1;
}

string Valve::toString(){
	CheckExists("DummyValve")
	return "DevNum"+::toString(devNum);
}

//dir=0 use fastest  dir=1 is cw and dir -1 is ccw
void Valve::select(int valvePos,double msPause, int dir){
	CheckExists()
	wait();
	if (dir!=0 && dir!=1 && dir!=-1){
		throw exception(string("Valve select error: direction not valid. Must be 0 , 1 or -1.").c_str());
		return;
	}
	if (dir==0)
		pmp->sendCommand("B"+::toString(valvePos)+"M"+::toString(int(msPause)),devNum);
	else if (dir==1)
		pmp->sendCommand("O"+::toString(valvePos)+"M"+::toString(int(msPause)),devNum);
	else//dir==-1
		pmp->sendCommand("I"+::toString(valvePos)+"M"+::toString(int(msPause)),devNum);
	pos=valvePos;
	wait();//can never be too safe.
}
void Valve::wait(){
	CheckExists()
	pmp->wait(devNum);
}