// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, November 15, 2011</created>
// <lastedit>Thursday, April 05, 2012</lastedit>
// ===================================
#include <string>
using namespace std;
#include "DaisyValve.h"
#include "Utils.h"
#include "Solution.h"
#include "Syringe.h"
#include "Valve.h"
#include "Record.h"
#include "Controller.h"
extern Record logFile;
extern Controller cont;
DaisyValve::DaisyValve(Valve* v1,Valve* v2,int daisyPort,double daisyInjectTubingVol,int outPort,double injectTubingVol):v1(v1),v2(v2),daisyInjectTubingVol(daisyInjectTubingVol),daisyPort(daisyPort),outPort(outPort),injectTubingVol(injectTubingVol){
		if (v1==NULL && v2!=NULL)
			throw exception(string("Daisy Valve creation error: attempt to create daisy valve with manual first valve and computer controlled 2nd valve").c_str());
		if (v1==NULL && daisyPort!=-1)
			throw exception(string("Daisy Valve creation error: cannot specify daisy port on a fully manual daisy valve").c_str());
		if (v1!=NULL && daisyPort!=-1 && (daisyPort<1 || daisyPort>9))
			throw exception(string("Daisy Valve creation error: invalid daisy port").c_str());
		if (v1!=NULL && daisyPort!=-1 && daisyInjectTubingVol<=0)
			throw exception(string("Daisy Valve creation error: manual 2nd valve requires daisyInjectTubingVol>0").c_str());
}
/*
bool DaisyValve::operator==(const DaisyValve& dv){
	return v1==dv.v1 && v2==dv.v2 && daisyPort==
}*/

DaisyValve::DaisyValve(Valve* v1,Valve* v2):v1(v1),v2(v2),daisyInjectTubingVol(-1),daisyPort(-1),injectTubingVol(-1),outPort(-1){
}

bool DaisyValve::isValid(int valveNum,int valvePos){
	if (cont.pmp.isDummyValve(v1))
		return true;
	if (valveNum==1){
		//if real valve then valvePos must be valid
		if (v1 && valvePos<1)
			return false;
		if (v1 && valvePos>9)
			return false;
		return true;
	}
	else if (valveNum==2){
		//daisyPort must be specified and if real valve then valvePos must be valid
		if (daisyPort==-1)
			return false;
		if (v2 && valvePos==-1)
			return false;
		if (v2 && valvePos<1)
			return false;
		if (v2 && valvePos>9)
			return false;
		return true;
	}else{
		if (v1)
			return false;
		else
			return true;
	}
}

double DaisyValve::getInjectTubingVolPull(int valveNum){
	switch(valveNum){
		case 1:
			return 0;
			break;
		case 2:
			return daisyInjectTubingVol;
			break;
		default:
			logFile.write("Solution Error: That valve number is not supported");
			return 0;
	}
}

double DaisyValve::getInjectTubingVolPush(int valveNum){
	switch(valveNum){
		case 2:
			return 0;
			break;
		case 1:
			return daisyInjectTubingVol;
			break;
		default:
			logFile.write("Solution Error: That valve number is not supported");
			return 0;
	}
}

void DaisyValve::select(Solution* s){
	int dir=0;
	try{
	if (!isValid(s->sd.valveNum,s->sd.valvePos))
		logFile.write("Solution: error, valve"+::toString(s->sd.valveNum)+"/pos"+::toString(s->sd.valvePos)+" is not supported");
	switch(s->sd.valveNum){
		case -1:
		case 1:
			if (v1==NULL){
				logFile.write("Put tubing into Solution "+s->sd.name+" (press enter to continue)",true);
				getString();
				return;
			}
			if (v1->pos==9)//daisy out
				if (s->sd.valvePos<5)
					dir=1;//cw to get there
				else 
					dir=-1;//ccw to get there
			v1->select(s->sd.valvePos,1000,dir);
			v1->wait();
			break;
		case 2:
			if (v1==NULL){
				logFile.write("Put tubing into Solution "+s->sd.name+" (press enter to continue)",true);
				getString();
				return;
			}
			if (v1->pos<5)
				v1->select(daisyPort,1000,-1);//ccw to get there
			else 
				v1->select(daisyPort,1000,1);//cw to get there
			v1->wait();
			if (v2->pos==9|| v2->pos==8){//outport || waste
				if (s->sd.valvePos<5)
					dir=1;//cw to get there 
				else 
					dir=-1;//ccw to get there
			
			}else if (s->sd.valvePos==8) {//waste
				if (v2->pos<5)
					dir=-1;//ccw to get there 
				else 
					dir=1;//cw to get there 
			}
			v2->select(s->sd.valvePos,1000,dir);
			v2->wait();
			break;
		default:
			logFile.write("This should not be possible",true);
	}
	}catch(std::ios_base::failure e)
			    {
					logFile.write("Pump Control Menu Exception: "+string(e.what()),true);
				}
}

void DaisyValve::selectOut(){
	if (v1==NULL)
		return;
	else if (v2==NULL && outPort!=-1){
		if (v1->pos<5)
			v1->select(outPort,1000,-1);//ccw to get there
		else
			v1->select(outPort,1000,1);//cw to get there
		v1->wait();
		return;
	}
	else{
		if (daisyPort!=-1){
			if (v1->pos<5)
				v1->select(daisyPort,1000,-1);//ccw to get there
			else
				v1->select(daisyPort,1000,1);//cw to get there
			v1->wait();
		}
		if (outPort!=-1){
			if (v2->pos<5)
				v2->select(outPort,1000,-1);//ccw to get there
			else
				v2->select(outPort,1000,1);//cw to get there
			v2->wait();
		}
	}
		
}

void DaisyValve::parse(string line){
	//daisyvalve is specified
	stringstream ss;
	ss<<line;
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisyPort specified
		daisyPort=toInt(line);
	}
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisyInjectTubingVol specified
		daisyInjectTubingVol=toDouble(line);
	}
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisy out port specified
		outPort=toInt(line);
	}
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisy valve 1 dev num specified
		v1=cont.pmp.getValve(toInt(line));
		if (cont.pmp.isDummyValve(v1)){
			logFile.write(string("DaisyValve Valve 1 not found, using dummy valve"),true);
		}
	}
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisy valve 2 dev num specified
		v2=cont.pmp.getValve(toInt(line));
		if (cont.pmp.isDummyValve(v2)){
			logFile.write(string("DaisyValve Valve 2 not found, using dummy valve"),true);
		}
	}
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!=""){
		//daisy valve inject volume
		injectTubingVol=toDouble(line);
	}
	getline(ss,line,',');
	if (!ss.fail()){
		throw exception(string("Daisy Valve parse error: too many parameters").c_str());
	}
	*this=DaisyValve(v1,v2,daisyPort,daisyInjectTubingVol,outPort,injectTubingVol);
}