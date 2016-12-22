// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Thursday, November 17, 2011</created>
// <lastedit>Tuesday, December 13, 2011</lastedit>
// ===================================
#include "FlowChannel.h"
#include "Valve.h"
#include "Syringe.h"
#include "Pump.h"
#include "Controller.h"
#include "Record.h"
extern Controller cont;
extern Record logFile;
using namespace std;
void FlowChannel::wait(){
	vPull->wait();
	s->wait();
}

FlowChannel::FlowChannel(Syringe* s,Chamber cham,Valve* vPull,Valve* vPush):name("tempNOTVALID"),s(s),cham(cham),injectTubingVolumePull(-1),injectTubingVolumePush(-1),syringePos(-1),vPullPort(0),vPushPort(0),vPull(vPull),vPush(vPush){

}

string FlowChannel::toString(){
	return "FlowChannel "+name;
}

FlowChannel::FlowChannel(std::string name,Syringe* s,int syringePos,Chamber cham,double injectTubingVolumePull,Valve* vPull,int vPullPort,double injectTubingVolumePush,Valve* vPush,int vPushPort,DaisyValve dvPull, DaisyValve dvPush):name(name),injectTubingVolumePull(injectTubingVolumePull),injectTubingVolumePush(injectTubingVolumePush),s(s),syringePos(syringePos),vPull(vPull),vPush(vPush),vPullPort(vPullPort),vPushPort(vPushPort),cham(cham),dvPull(dvPull),dvPush(dvPush){
	if (s==NULL)
		throw exception(string("FlowChannel "+name+" creation error: must have a valid syringe").c_str());
	if (name=="tempNOTVALID")
		throw exception(string("FlowChannel "+name+" creation error: must have a valid name").c_str());
	if (injectTubingVolumePull<=0)
		throw exception(string("FlowChannel "+name+" creation error: injectTubingVol must be >0").c_str());
	if (this->s->wastePos==this->syringePos || syringePos<1 || syringePos>9)
		throw exception(string("FlowChannel "+name+" creation error: flow channel syringe position cannot be the same as the waste position on that syringe").c_str());
	if (vPull!=NULL && (vPullPort<1 || vPullPort>9))
		throw exception(string("FlowChannel "+name+" creation error: a valid channel multiplexing valve is present. must specify a valid vPullPort").c_str());
	if (vPull==NULL && vPullPort!=0)
		throw exception(string("FlowChannel "+name+" creation error: vPullPort cannot be specified without a valid valve").c_str());
	if (dvPush.v1!=NULL && !cont.pmp.isDummyValve(dvPush.v1) && dvPull.v1 && !cont.pmp.isDummyValve(dvPull.v1) && dvPull.outPort==-1)
		throw exception(string("When Push Daisy Valve is present, Pull Daisy Valve requires outport").c_str());
}

bool FlowChannel::conflict(FlowChannel& fc){
	if (name==fc.name)
		return true;
	if (!vPush && s==fc.s && !s->pmp->isDummySyringe(s) && fc.syringePos==syringePos)
		return true;
	if (vPull && vPull==fc.vPull && vPullPort==fc.vPullPort)
		return true;
	if (vPush && vPush==fc.vPush && vPushPort==fc.vPushPort)
		return true;
	return false;
}

void FlowChannel::push(double uL,double uLps,double msPause){
	dvPull.selectOut();
	dvPush.selectOut();
	select();
	this->s->push(this->syringePos,uL,uLps,true,msPause);
}
void FlowChannel::pull(double uL,double uLps,double msPause){
	dvPush.selectOut();
	select();
	this->s->pull(this->syringePos,uL,uLps,true, msPause);
}

void FlowChannel::waste(){
	s->waste();
}

double FlowChannel::getSyringeVolume(FlowChannel* chan){
	int deviceNum=chan->s->devNum;
	//if(cont.pmp.getSyringe(deviceNum)!=&dummySryinge
	return cont.pmp.getSyringe(deviceNum)->volume;
	}
FlowChannel FlowChannel::parseLine(string line){
	stringstream ss;
	ss<<line;
	//Channel name
	getline(ss,name,',');
	removeWhite(name);
	try {
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//syring valve position specified
			this->syringePos=toInt(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//syringe dev number specified
			this->s=cont.pmp.getSyringe(toInt(line));
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//valve Port number specified
			this->vPullPort=toDouble(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//valve device number specified
			this->vPull=cont.pmp.getValve(toInt(line));
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//valve Port number specified
			this->vPushPort=toDouble(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//valve device number specified
			this->vPush=cont.pmp.getValve(toInt(line));
		
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//injectTubingVolume specified
			this->injectTubingVolumePull=toDouble(line);
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!="")
			//injectTubingVolume specified
			this->injectTubingVolumePush=toDouble(line);
		
	}catch(std::ios_base::failure e){
		throw exception(string("FlowChannel "+name+" parse error: could not convert input to numeric value").c_str());
	}

	getline(ss,line,',');
	if (!ss.fail())
		throw exception(string("FlowChannel "+this->name+" parse error: too many input parameters").c_str());
	if (!cont.pmp.isDummySyringe(s) && cont.pmp.isDummyValve(vPull)){
		this->vPull=NULL;
		this->vPullPort=0;
	}
	if (!cont.pmp.isDummySyringe(s) && cont.pmp.isDummyValve(vPush)){
		this->vPush=NULL;
		this->vPushPort=0;
	}
	if (cont.pmp.isDummyValve(vPull) && this->vPullPort==0)
		vPull=NULL;
	if (cont.pmp.isDummyValve(vPull) && this->vPullPort==0)
		vPush=NULL;

	return FlowChannel(name,this->s,this->syringePos,this->cham,this->injectTubingVolumePull,this->vPull,this->vPullPort,this->injectTubingVolumePush,this->vPush,this->vPushPort,dvPull,dvPush);
}

void FlowChannel::select(){
	if (vPull!=NULL){
		vPull->select(vPullPort);
	}
	if (vPush!=NULL){
		vPush->select(vPushPort);
	}
	if (vPull!=NULL){
		vPull->wait();
	}
	if (vPush!=NULL){
		vPush->wait();
	}
}