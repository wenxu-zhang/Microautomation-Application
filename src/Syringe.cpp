// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, April 05, 2012</lastedit>
// ===================================
#include "Syringe.h"
#include "Controller.h"
#include <conio.h>
extern Controller cont;
extern Record logFile;
extern customProtocol scan;
using namespace std;

Syringe::Syringe(Pump* pmp,int deviceNum,double volume,int maxStepNormal, int maxStepFine,int wastePos)
:pmp(pmp),maxStepNormal(maxStepNormal),maxStepFine(maxStepFine),maxStep(maxStepNormal),devNum(deviceNum),volume(volume),wastePos(wastePos),b(255),slopeCode(7),isPresent(false){

	//add to constructor...
	//enableMicrostepping();   put in constructor for syringe
	//disableMicrostepping();
	cont.pmp.wait(devNum);
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("?"),devNum, &(b.m_str)));
	std::string s;
	wstring ws(b);
	s.assign(ws.begin(), ws.end()); 
	int i=toInt(s);
	if (i==8142 || i==65136) {
		logFile.write("Pump not initialized.",true);
		isInitialized=false;
	}
	else {
		logFile.write("Pump has already been initialized.",true);
		isInitialized=true;
	}	
	//enable microstepping
	pmp->PumpErrChk(pmp->pumpServer->PumpSendCommand(CComBSTR("N1R"),devNum,&(b.m_str)));
	this->maxStep=maxStepFine;
	isPresent=true;
	//this->enableMicrostepping();//dont call this because it will waste syringe
				
}


double Syringe::fineControl(int syringePort, double uLps){
	int c;
	int start=	this->getPosition();
	while(true){
		cout<<"Welcome to Fine Pump Control. Press e to exit (Chamber volume is "<<::toString(scan.cham.getChannelVolume())<<" uL)"<<endl;
		cout<<"     Up    arrow: "<<this->steps2vol(1)<<"uL increments (+1 step)"<<endl;
		cout<<"     Down  arrow: "<<this->steps2vol(1)<<"uL decrements (-1 step)"<<endl;
		cout<<"     Right arrow: "<<this->steps2vol(10)<<"uL increments (+10 steps)"<<endl;
		cout<<"     Left  arrow: "<<this->steps2vol(10)<<"uL decrements (-10steps)"<<endl;
		cout<<"     d key:       "<<this->steps2vol(100)<<"uL increments (+100 steps)"<<endl;
		cout<<"     u key:       "<<this->steps2vol(100)<<"uL decrements (-100 steps)"<<endl;
		cout<<"     > key:       "<<this->steps2vol(1000)<<"uL increments (+1000 steps)"<<endl;
		cout<<"     < key:       "<<this->steps2vol(1000)<<"uL decrements (-1000 steps)"<<endl;
		cout<<"     f key:       Pull fixed volume"<<endl;
		cout<<"		s key:		 Push fixed volume"<<endl;
		cout<<"     e            Exit"<<endl<<endl;
		cout<<"Make a selection..."<<endl;
		c=getch();
		switch(c){
		case 224://arrow pressed
			c=getch();
			switch(c){
			case 72://up
				cout<<"pulling 1 step"<<endl;
				this->pull(syringePort,this->steps2vol(1),uLps/10,true,0);
				break;
			case 80://down
				cout<<"pushing 1 step"<<endl;
				this->push(syringePort,this->steps2vol(1),uLps/10,true,0);
				break;
			case 75://left
				cout<<"pushing 10 steps"<<endl;
				this->push(syringePort,this->steps2vol(10),uLps/4,true,0);
				break;
			case 77://right
				cout<<"pulling 10 steps"<<endl;
				this->pull(syringePort,this->steps2vol(10),uLps/4,true,0);
				break;
			}
			break;
		case 'e':{
			int end=this->getPosition();
			return this->steps2vol(abs(start-end));
				 }
		case 'd':
			cout<<"pulling 100 steps"<<endl;
			this->pull(syringePort,this->steps2vol(100),uLps,true,0);
			break;
		case 'u':
			cout<<"pushing 100 steps"<<endl;
			this->push(syringePort,this->steps2vol(100),uLps,true,0);
			break;
		case '.':
			cout<<"pulling 1000 steps"<<endl;
			this->pull(syringePort,this->steps2vol(1000),uLps,true,0);
			break;
		case ',':
			cout<<"pushing 1000 steps"<<endl;
			this->push(syringePort,this->steps2vol(1000),uLps,true,0);
			break;
		case 'f':{
			cout<<"enter volume (uL) to pull"<<endl;
			double vol=getDouble();
			this->pull(syringePort,vol,uLps,true,0);
			break;}
				 case 's':{
			cout<<"enter volume (uL) to push"<<endl;
			double vol=getDouble();
			this->push(syringePort,vol,uLps,true,0);
			break;}
		default:
			cout<<c<<" is not a valid option"<<endl<<endl;
			break;
		}
		this->wait();
		cout<<"...selection executed"<<endl;
		int end=this->getPosition();
		if (end-start>=0){
			cout<<"pulled "<<end-start<<" steps so far ("<<::toString(this->steps2vol(end-start))<<" uL)"<<endl;
		}else{
			cout<<"pushed "<<start-end<<" steps so far ("<<::toString(this->steps2vol(start-end))<<" uL)"<<endl;
		}
	}
}

void Syringe::sendCommand(string st){
	CheckExists()
	if (!this->isInitialized)
		throw exception(string("Syringe Error: syringe is not initialized").c_str());
	wait();
	pmp->sendCommand(st,devNum);
}

string Syringe::moveValve(int pos){
	return "B"+::toString(pos);
}
void Syringe::pull(int valvePos,double uL, double uLps,bool w8,double msPause){
	string cmd=_pull(valvePos,uL,uLps,NULL,msPause);
	wait();
	sendCommand(cmd);
	if (w8)
		wait();
}

void Syringe::stop(){
	pmp->terminate(devNum);
	logFile.write("Syringe plunger move was terminated.  You should reinitialize before using the Syringe",true);
	isPresent=true;
	isInitialized=false;
}

void Syringe::init(){
	CheckExists()
		//move valve to first position cause tecan sucks
	wait();
	pmp->sendCommand("B1",devNum);
	wait();
	pmp->sendCommand("Z",devNum);
	wait();
	this->isInitialized=true;
	if (this->maxStep==maxStepFine)
		enableMicrostepping();
	else
		disableMicrostepping();
}


string Syringe::toString(){
	CheckExists("DummySyringe")
	return "DevNum"+::toString(devNum)+" "+::toString(this->volume/1000,1)+"mL";
}

string Syringe::_pull(int valvePos,double uL, double uLps, double* time, double msPause){
	string out="";
	int vol=toInt(vol2steps(uL));
	int pos=getPosition();
	if (vol<=maxStep){
		if (vol>maxStep-pos && pos>0){
			if (time){
				logFile.write("Pump error: time calculation not valid for multiple plunger movements",false);
				*time=0;
			}
			return out+_waste()+string("B")+::toString(valvePos)+"V"+flow2Hz(uLps)+"P"+::toString(vol)+"M"+::toString(int(msPause));
		}
		else{
			if (time)
				*time=calcTime(uLps,uL);
			return out+string("B")+::toString(valvePos)+"V"+flow2Hz(uLps)+"P"+::toString(vol)+"M"+::toString(int(msPause));
		}
	}
	if (vol>maxStep-pos){
		//we need multiple pull then waste cycles
		int numCycles=1+(vol-(maxStep-pos))/maxStep;
		int numCycles2=1+vol/maxStep;
		if (numCycles==numCycles2){//just waste first since it will be faster
			out=out+_waste()+"gB"+::toString(valvePos)+"V"+flow2Hz(uLps)+"A"+::toString(maxStep)+"M"+::toString(int(msPause))+_waste()+"G"+::toString(numCycles-1);
			vol=vol-(numCycles-1)*maxStep;
		}else{
			out=out+"gB"+::toString(valvePos)+"V"+flow2Hz(uLps)+"A"+::toString(maxStep)+"M"+::toString(int(msPause))+_waste()+"G"+::toString(numCycles);
			vol=vol-(maxStep-pos)-(numCycles-1)*maxStep;
		}
	}
	if (time){
		logFile.write("Pump error: time calculation not valid for multiple plunger movements",true);
		*time=0;
	}
	if (vol!=0) 
		out=out+string("B")+::toString(valvePos)+"V"+flow2Hz(uLps)+"P"+::toString(vol)+"M"+::toString(int(msPause));
	return out;
}




void Syringe::push(int valvePos,double uL, double uLps,bool w8,double msPause){
	CheckExists()
	string cmd=_push(valvePos,uL,uLps,msPause);
	sendCommand(cmd);
	if (w8)
		wait();
}
string Syringe::_push(int valvePos,double uL,double uLps,double msPause){
	CheckExists("")
	string out="";
	int vol=::toInt(vol2steps(uL));
	int pos=getPosition();
	if (vol>pos){
		logFile.write("Error: current syringe position does not allow this push volume. No volume will be pushed.",true,"Make sure you have enough room when this command is executed");
		return string("B")+::toString(valvePos)+"V"+flow2Hz(uLps)+"D"+::toString(0)+"M"+::toString(int(msPause));
	}
	return string("B")+::toString(valvePos)+"V"+flow2Hz(uLps)+"D"+::toString(vol)+"M"+::toString(int(msPause));
}

string Syringe::_waste(){
	if (maxStep==maxStepFine)
		return string("B")+::toString(wastePos)+"S2A0M500V5A40";
	else
		return string("B")+::toString(wastePos)+"S2A0";
}

void Syringe::waste(bool w8){
	sendCommand(_waste()+"M2000");
	if (w8)
		wait();
}





int Syringe::flow2HzInt(double uLps){
	CheckExists(0)
	int hz;
	if (uLps==Pump::MINSPEED)
		hz=5;
	else hz=0.5+2.0*uLps*maxStepNormal/volume;
	if (hz>5800) {
		logFile.write("Pump::error: this speed: "+::toString(uLps)+"mm/sec is too fast",DEBUGPUMP," and cannot be achieved with this syringe/chamber combination. Decrease chamber height or width OR increase syringe size");
	}
	if (hz<5) {
		logFile.write("Pump::error: this speed: "+::toString(uLps)+"mm/sec is too slow",DEBUGPUMP," and cannot be achieved with this syringe/chamber combination. Increase chamber height or width OR decrease syringe size");
	}
	if (hz>5800) hz=5800;
	if (hz<5) hz=5;
	return hz;
}
string Syringe::flow2Hz(double uLps){
	CheckExists("0")
	int hz=flow2HzInt(uLps);
	return ::toString(hz);
}

string Syringe::vol2steps(double uL){
	CheckExists("0")
	int steps=0.5+uL*maxStep/volume;//rounding to nearest integer	
	if (steps<=0) {
		logFile.write("Error. Input volume: "+::toString(uL)+"cannot be negative or is too small",DEBUGPUMP);
		steps=0;
	}
	return ::toString(steps);
}

int Syringe::vol2stepsInt(double uL){
	CheckExists(0)
	int steps=0.5+uL*maxStep/volume;//rounding to nearest integer	
	if (steps<=0) {
		logFile.write("Error. Input volume: "+::toString(uL)+"cannot be negative or is too small",DEBUGPUMP);
	}
	if (steps<=0) steps=0;
	return steps;
}

double Syringe::steps2vol(int steps){
	CheckExists(0)
	double uL=steps*volume/maxStep;
	if (uL<=0) {
		logFile.write("Error. Input step: "+::toString(steps)+"cannot be negative or is zero",DEBUGPUMP);
	}
	if (uL<=0) uL=0;
	return uL;
}

void Syringe::enableMicrostepping(){
	CheckExists()
		cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("N1R"),devNum,&(b.m_str)));
	this->maxStep=maxStepFine;
	if (this->isInitialized)
		waste();
}

void Syringe::disableMicrostepping(){
	CheckExists()
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("N0R"),devNum,&(b.m_str)));
	maxStep=maxStepNormal;
	if (this->isInitialized)
		waste();
}

int Syringe::getPosition(){
	CheckExists(0)
	if (!isInitialized){
		throw exception(string("Pump not initialized").c_str());
		return 0;
	}
	//CComBSTR b(255);//this is preallocated by constructor //BSTR b=SysAllocStringLen(NULL,255);
	wait();
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("?"),devNum, &(b.m_str)));
	std::string s;
	wstring ws(b);
	s.assign(ws.begin(), ws.end()); 
	int ret=toInt(s);
	return ret;
}

double Syringe::calcTime(double uLps,double vol){
	CheckExists(0)
	if (pmp->pendingCommand(devNum)){
		logFile.write("Pump error: cannot calculate time during a pending command.",true);
		return 0;
	}
	int steps=this->vol2stepsInt(vol);
	int hz=this->flow2HzInt(uLps);
	string v="V"+::toString(hz)+"R";
	pmp->PumpErrChk(pmp->pumpServer->PumpSendCommand(CComBSTR(v.c_str()),devNum,&(this->pmp->b.m_str)));
	wait();
	int start=getStartVelocity();
	int cutoff=getCutoffVelocity();
	int top=getTopVelocity(hz);
	double slope=slopeCode*2500;//hz per second or half-steps*sec^-2
	if (top<=start)//no ramp up or ramp down
		return 2.0*steps/top;//.5 because speed is in half steps
	double SU=2.0*0.5*(top*top-start*start)/slope;
	double SD=2.0*0.5*(top*top-cutoff*cutoff)/slope;
	if (SU+SD>steps){//never reach top velocity{
		double vf=sqrt(slope*steps*0.5+0.5*start*start+0.5*cutoff*cutoff);
		return (vf-start)/slope+(vf-cutoff)/slope;
	}
	else//normal case...top velocity is reached
		return (top-start)/slope+(top-cutoff)/slope+0.5*(steps-SU-SD)/top;
}

int Syringe::getStartVelocity(){
	CheckExists(0)
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("?1"),devNum, &(b.m_str)));
	int ret=toInt(wstring(b));
	return ret;
}

int Syringe::getCutoffVelocity(){
	CheckExists(0)
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("?3"),devNum, &(b.m_str)));
	int ret=toInt(wstring(b));
	return ret;
}

int Syringe::getTopVelocity(int Hz){
	CheckExists(0)
	cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("?2"),devNum, &(b.m_str)));
	int ret=toInt(wstring(b));
	return ret;
}

void Syringe::setSlope(int val){
	CheckExists()
	slopeCode=val;
	string v="L"+::toString(val)+"R";
	pmp->PumpErrChk(pmp->pumpServer->PumpSendCommand(CComBSTR(v.c_str()),devNum,&(pmp->b.m_str)));
}

void Syringe::wait(){
	CheckExists()
	cont.pmp.wait(devNum);
//	PumpErrChk(cont.pmp.pumpServer->PumpWaitForDevice(deviceNum));
}

