// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Saturday, April 07, 2012</lastedit>
// ===================================
#include "Solution.h"
#include "Utils.h"
#include "Pump.h"
#include "Controller.h"
#include "Chamber.h"
#include "FlowChannel.h"
#include <iostream>
using namespace std;

extern Controller cont;
extern Record logFile;
extern customProtocol scan;
extern HANDLE abortEvent;
Solution::Solution(SolutionData& sd):sd(sd)
{
	if (sd.name=="tempNOTVALID" || sd.name=="")
		throw exception(string("Solution "+sd.name+" creation error: must have a valid name").c_str());
	if (sd.inletTubingVol<0)
		throw exception(string("Solution "+sd.name+" creation error: inletTubingVol must be >= 0").c_str());
	if (sd.uLps<=0)
		throw exception(string("Solution "+sd.name+" creation error: flow rate must be > 0").c_str());
	if (sd.loadFactor<= 0)
		throw exception(string("Solution "+sd.name+" creation error: load factor must be > 0").c_str());
	}

PullSolution::PullSolution(SolutionData& sd):Solution(sd){
	if (sd.valveNum==-1 && !sd.dv.isValid(2,sd.valvePos))
		sd.valveNum=1;//only if there is no ambiguity about valve number
	if (!sd.dv.isValid(sd.valveNum,sd.valvePos))
		throw exception(string("Solution "+sd.name+" creation error: daisy valve does not support specified valve number/valve pos combination").c_str());

}

PushSolution::PushSolution(SolutionData& sd):Solution(sd){
	if (sd.s==NULL || sd.syringePort==-1)
		throw exception(string("Solution "+sd.name+" creation error: syringe and syringe port required").c_str());
	if (sd.valveNum==-1 && (sd.s==NULL || sd.syringePort==-1))
		throw exception(string("Solution "+sd.name+" creation error: syringe and syringe port required for solution on syringe").c_str());
	if (sd.valveNum!=-1)
		if (!sd.dv.isValid(sd.valveNum,sd.valvePos))
			throw exception(string("Solution "+sd.name+" creation error: daisy valve does not support specified valve number/valve pos combination").c_str());
	if (sd.dv.injectTubingVol<0)
		throw exception(string("Push Daisy Valve requires injectVolume").c_str());
	if (sd.dv.outPort==-1)
		throw exception(string("Push Daisy Valve requires outPort").c_str());
}

bool Solution::conflict(Solution& s){
	if (sd.valveNum==1)
		if (s.sd.valveNum==1){
			if (sd.dv.v1==s.sd.dv.v1 && !sd.dv.v1->pmp->isDummyValve(sd.dv.v1) && sd.valvePos>0 && sd.valvePos==s.sd.valvePos){
				return true;
			}
		else if (s.sd.valveNum==2){
			if (sd.dv.v1==s.sd.dv.v2 && !sd.dv.v1->pmp->isDummyValve(sd.dv.v1) && sd.valvePos>0 && sd.valvePos==s.sd.valvePos){
				return true;
			}
		}
	}
	if (sd.valveNum==2)
		if (s.sd.valveNum==1){
			if (sd.dv.v2==s.sd.dv.v1 && !sd.dv.v1->pmp->isDummyValve(sd.dv.v1) && sd.valvePos>0 && sd.valvePos==s.sd.valvePos){
				return true;
			}
		else if (s.sd.valveNum==2){
			if (sd.dv.v2==s.sd.dv.v2 && sd.valvePos>0 && sd.valvePos==s.sd.valvePos){
				return true;
			}
		}
	}
	if (sd.name==s.sd.name)
		return true;
	return false;
}

void Solution::setFlowVelocity(double mmps,Chamber* c){
	double CSA=c->channelWidth*c->channelHeight;//(cross-sectional area in mm squared)
	sd.uLps= mmps*CSA;
}


void Solution::valveSelect(){
	sd.dv.select(this);
}

void PushSolution::valveSelect(){
	if (sd.valveNum==-1)
		return;
	sd.dv.select(this);
}


FlowChannel* PullSolution::prime(FlowChannel* chan){
	if (chan==NULL)
		chan=scan.fs.selectChannel();
	if (chan==NULL){
		logFile.write("No Channel selected, solution was not primed",true);
		return NULL;
	}
	valveSelect();
	chan->pull(4*(sd.inletTubingVol+this->sd.dv.getInjectTubingVolPull(sd.valveNum)+chan->getInjectTubingVolPull()),sd.uLps);
	chan->wait();
	return chan;
}


FlowChannel* PushSolution::prime(FlowChannel* chan){
	if (sd.valveNum==-1){//wash is on syringe not daisy valve
		sd.s->pull(sd.syringePort,1.2*this->sd.inletTubingVol,sd.uLps*5);
		sd.s->waste();
	}else{//wash is on daisy valve
		valveSelect();
		sd.s->pull(sd.syringePort,1.2*(sd.syringePort,this->sd.inletTubingVol+this->sd.dv.injectTubingVol+this->sd.dv.getInjectTubingVolPull(this->sd.valveNum)),sd.uLps);
		sd.s->waste();
	}
	return chan;
}

//bring into chamber, no trailing solution
void PullSolution::load(FlowChannel* chan,double param, double time){
	//pull in chamber volume of reagent
	valveSelect();
	//pull up to chamber and then load factor (number of chamber volumes to wash with)
	double chamVol=chan->cham.getChannelVolume();
	double dvInjVol=getInjectTubingVolume();
	double chanInjVol=chan->getInjectTubingVolPull();
	chan->pull(dvInjVol+chanInjVol+sd.loadFactor*chamVol,sd.uLps);
}

void PushSolution::pull(double vol,FlowChannel* f,double uLps){
	if (uLps==-1)
		uLps=sd.uLps;
	if (vol==-1)
		vol=sd.s->steps2vol(sd.s->maxStep-sd.s->getPosition());
	valveSelect();sd.s->pull(sd.syringePort,vol,uLps);
}

void PushSolution::push(double vol,FlowChannel* f,double uLps){
	if (uLps==-1)
		uLps=sd.uLps;
	if (vol==-1)
		vol=sd.s->steps2vol(sd.s->getPosition());
	valveSelect();sd.s->push(sd.syringePort,vol,uLps);
}

FlowSol PushSolution::clean(double inletFactor,double uLps, FlowChannel* f,Solution* s){
	FlowSol fs;
	fs.f=f;
	fs.s=s;
	if (s==NULL){
		logFile.write("Error in Push Solution clean: wash solution must be specified. cleaning will not be performed",true);
		return fs;
	}
	if (s->sd.dv.v1!=sd.dv.v1 || s->sd.dv.v2!=sd.dv.v2){
		logFile.write("Error in Push Reagent clean: wash solution is not on the same daisy valve as this reagent. cleaning will not be performed",true);
		return fs;
	}

	double vol=sd.inletTubingVol*inletFactor+abs(s->getInjectTubingVolume()-this->getInjectTubingVolume());
	double pos=sd.s->steps2vol(sd.s->getPosition());
	while(vol>sd.s->volume-pos){
		s->pull(sd.s->volume-pos,fs.f,uLps);
		valveSelect();
		this->sd.s->push(sd.syringePort,sd.s->volume-pos,uLps,true,0);
		vol=vol-(sd.s->volume-pos);
	}
	s->pull(vol,f,uLps);
	valveSelect();
	this->sd.s->push(sd.syringePort,vol,uLps,true,0);
	return fs;
}

//bring into chamber, no trailing solution. param is number of wash cycles to perform, time is the pause between cycles
void PushSolution::load(FlowChannel* chan,double param, double time){
	if (sd.s!=chan->s){
		throw exception(string("Cannot load "+toString()+" Channel Syringe and DaisyValve Syringe must be the same.").c_str());
		return;
	}
	if (sd.valveNum!=-1 && sd.syringePort!=chan->syringePos){
		throw exception(string("Cannot load "+toString()+" Channel Syringe Port and DaisyValve Syringe Port must be the same for solutions attached to daisy port.").c_str());
		return;
	}
	//pull in chamber volume of reagent
	
	//pull up to chamber and then load factor (number of chamber volumes to wash with)
	double chamVol=chan->cham.getChannelVolume();
	double dvInjVol=getInjectTubingVolume();
	double chanInjVol=chan->getInjectTubingVolPull();
	double volWash=sd.loadFactor*chamVol;
	double vol;
	sd.s->waste();
	if (param==-1){
		param=1;
	}
	if (time==-1){
		time=0;
	}
	double pos;
	Timer t;
	for(int i=0;i<param-1;i++){
		vol=volWash;
		pos=sd.s->steps2vol(sd.s->getPosition());
		while(vol>sd.s->volume-pos){
			valveSelect();
			sd.s->pull(sd.syringePort,sd.s->volume-pos,sd.uLps*5);
			chan->push(sd.s->volume-pos,sd.uLps);
			vol=vol-(sd.s->volume-pos);
		}
		valveSelect();
		sd.s->pull(sd.syringePort,vol,sd.uLps*5);
		chan->push(vol,sd.uLps);
		t.restart();
		//sd.s->waste();
		t.waitTotal(1000*time,abortEvent);

	}
	vol=volWash;
	pos=sd.s->steps2vol(sd.s->getPosition());
	while(vol>sd.s->volume-pos){
		valveSelect();
		sd.s->pull(sd.syringePort,sd.s->volume-pos,sd.uLps*5);
		chan->push(sd.s->volume-pos,sd.uLps);
		vol=vol-(sd.s->volume-pos);
	}
	valveSelect();
	sd.s->pull(sd.syringePort,vol,sd.uLps*5);
	chan->push(vol,sd.uLps);
	sd.s->waste();
	
}

double PullSolution::getInjectTubingVolume(){
	return sd.dv.getInjectTubingVolPull(this->sd.valveNum);
}

double PushSolution::getInjectTubingVolume(){
	if (sd.valveNum==-1)
		return sd.dv.injectTubingVol+sd.dv.daisyInjectTubingVol;
	return sd.dv.getInjectTubingVolPush(this->sd.valveNum);
}


void SolutionData::parsePushSolution(string line){
	string sol=commaSep(line,1,6);
	parseSolution(sol);
	string sPort=commaSep(line,7,7);
	if (removeWhite(sPort)!="")
		syringePort=toInt(sPort);
}

void SolutionData::parseSolution(string line){
	stringstream ss;
	ss<<line;	

	//Solution name
	getline(ss,name,',');
	removeWhite(name);
	try{
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//they specified valvePos
			this->valvePos=toInt(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//they specified valveNumber
			this->valveNum=toInt(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//they specified inlet tubing volume
			this->inletTubingVol=toDouble(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//they specified load factor
			loadFactor=toDouble(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//they specified flow rate
			this->uLps=toDouble(line);
	}catch(std::ios_base::failure e){
		throw exception(string("Solution "+name+" parse error: could not convert input to numeric value").c_str());
	}
	getline(ss,line,',');
	if (!ss.fail()){
		throw exception(string("Solution: "+this->name+" parse error: too many input parameters").c_str());
	}

	if (valveNum==-1 && cont.pmp.isDummyValve(dv.v1))
		valveNum=1;
}

std::string Solution::toString(){
	return "Solution "+toString_();
}
string Solution::toString_(){
	return sd.name;
}


void Solution::setFlowRate(double uLps){this->sd.uLps=uLps;}

Solution* PullSolution::clone(FluidicsSetup& fs){
	return new PullSolution(*this);
}

Solution* PushSolution::clone(FluidicsSetup& fs){
	return new PushSolution(*this);
}
/*
void PullSolution::coarseCalibrateInlet(){
}

void PullSolution::calibrateInlet(){
}

void PushSolution::coarseCalibrateInlet(){
}

void PushSolution::calibrateInlet(){
}

*/
