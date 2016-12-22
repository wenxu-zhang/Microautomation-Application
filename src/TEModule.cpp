// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, March 31, 2011</lastedit>
// ===================================
// TEModule.cpp : Defines the entry point for the console application.
//
#include "Warn.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <process.h>
#include "Controller.h"
#include "TEModule.h"
#include "Timer.h"
#include "comm.h"
//#include "Thread.h"
using namespace std;
extern Record logFile;
extern Controller cont;
extern HANDLE abortEvent;
double TEModule::getMultiplier(){
	return multiplier;
}
template <typename T>
struct vlist_of : public vector<T> {
    vlist_of(const T& t) {
        (*this)(t);
    }
    vlist_of& operator ()(const T& t) {
        this->push_back(t);
        return *this;
    }
};

vector<double> TEModule::getPID(double temp){
	vector<vector<double>>::iterator i;
	vector<vector<double>>::iterator end;
	double diff,diff2;
	if (multiplier==10){//TC24
		if (gainScheduleTC24.empty())
			return vlist_of<double>(0.0)(TE24PROPBAND)(TE24INTGAIN)(TE24DERGAIN)(TE24HEATMULT)(0);
		i= gainScheduleTC24.begin();
		end=gainScheduleTC24.end();
		
	}else{//TC36
		if (gainScheduleTC36.empty())
			return vlist_of<double>(0.0)(TE36PROPBAND)(TE36INTGAIN)(TE36DERGAIN)(TE36HEATMULT)(TE36COOLMULT);
		i= gainScheduleTC36.begin();
		end= gainScheduleTC36.end();
	}
	int j=1;
	int best=0;
	diff=::abs(temp-i->front());
	for(i=i+1;i!=end;i++,j++){
		diff2=abs(temp-i->front());
		if (diff2<diff){
			diff=diff2;
			best=j;
		}
	}
	if (multiplier==10){//TC24
		return gainScheduleTC24.at(best);
	}else{//TC36
		return gainScheduleTC36.at(best);
	}
}

void TEModule::setPID(vector<double> pids){
	setPropBand(pids.at(1));
	setIntegGain(pids.at(2));
	setDerGain(pids.at(3));
	setHeatMult(pids.at(4));
	setColdMult(pids.at(5));
}

TEModule::TEModule(int COMM):multiplier(100.0){	
	
	//TC24
	gainScheduleTC24.push_back(vlist_of<double>(4.0)(6.0/2)(2.3)(.23)(.2)(1));
	gainScheduleTC24.push_back(vlist_of<double>(20.0)(20.0/2)(.2)(.02)(.2)(1));
	gainScheduleTC24.push_back(vlist_of<double>(37)(100.0/2)(.5)(0)(1)(1));
	gainScheduleTC24.push_back(vlist_of<double>(75)(30.0/2)(.5)(.1)(1)(1));

	//TC36
	gainScheduleTC36.push_back(vlist_of<double>(4.0)(6/2)(.2)(0)(1)(1));
	gainScheduleTC36.push_back(vlist_of<double>(37)(60/2)(.2)(0)(1)(1));
	gainScheduleTC36.push_back(vlist_of<double>(75)(30/2)(.5)(.1)(1)(1));
	
	tecom=new comm();
	this->COMM=COMM;
	
	controlType=true;//this will get set to false (PID control) when we call setFixedTemp later in this constructor
	TELogFile=new Record(cont.workingDir+"TELogFile.txt");
	isPresent=true;
	record=false;//don't record
	triggerRecord=false;
	ramp=false;//Indicates end of temperature ramp.
	waitTolerance=.5;
	waitTimeout=10;
	comment="";
	hRampThread=0;
	hPeriodicRThread=0;
	hRecordAccess=CreateMutex(NULL,false,NULL);
	//hCommMutex=CreateEvent(NULL, false, true, NULL);//Mutex to access comm. Set for first function to send command.

	//open com port
	int comstat;
	char tmp[100];
	sprintf(tmp,"COM%i",COMM);
	comstat=tecom->opencom(tmp);
	if (!comstat){
		logFile.write(string("TEModule Controller NOT PRESENT. Could not open COM ")+toString(COMM),true);
		//cout<<"Could not open COM"<<COMM<<endl;
		isPresent=false;
		return;
	}

	/*//The following code is needed for the comm class, it establishes a timing mechanism for communication (don't ask me why this is necessary, in my opinion
	//details such as these should be hidden from the user.  afterall, that's the purpose of abstraction)
	long int x;
	double y=0;
	time_t startTime = time(NULL);
	for(x=0;x<500000000;x++){
		y=y+1.0;
	}
	time_t endTime = time(NULL);
	double elapsedTime = difftime(endTime, startTime);
	//firstimer=elapsedTime.GetSeconds();
	if(elapsedTime > 4)timemult=1000;
	else {
		if(elapsedTime >= 3)timemult=2000;
	else timemult = 3000;
	}
	cout<<"Elapsed Time was:"<<elapsedTime<<endl;
	//end stupid code
*/
	tecom->timemult=1000;
	//turn off power
	if (tecom->setint("2d",0)==-1){
		isPresent=false;
		cout<<"TE Module Controller NOT PRESENT"<<endl;
		tecom->closecom();
		return;
	}
	tecom->setint("",100);
	double val=getColdMult();
	if (val!=1.0){
		logFile.write("Found TC 24-25 Controller",true);
		multiplier=10.0;
		setDerGain(TE24DERGAIN);//.23);
		setIntegGain(TE24INTGAIN);//2.3);
		setPropBand(TE24PROPBAND);//6);
		setHeatMult(TE24HEATMULT);
	}else{
		logFile.write("Found TC 36-25 Controller",true);
		multiplier=100.0;
		setDerGain(TE36DERGAIN);//.23);
		setIntegGain(TE36INTGAIN);//2.3);
		setPropBand(TE36PROPBAND);//6);
		setColdMult(TE36COOLMULT);
		setHeatMult(TE36HEATMULT);
	}
	


	//isPowerOn=false;
	//set to degrees C
	tecom->setint("32",1);

	//set polarity
	tecom->setint("2c",0);

		//set default fixed temp to 20deg C
	setFixedTemp(20,false);
	powerOff();
	cout << "TE Module Controller initialized.\n";
	//cout << "YOU MUST TURN WATER COOLER ON"<<endl;
	//system("pause");

}

TEModule::~TEModule(){
	CheckExists()
		stopRecording();
	stopRamp();
	//WaitForSingleObject(hRampThread,INFINITE);
	//WaitForSingleObject(hPeriodicRThread,INFINITE);
	//WaitForSingleObject(hTriggerRThread,INFINITE);
	powerOff();
	TELogFile->write("File Closed.");
	delete TELogFile;
	tecom->closecom();
}
bool TEModule::changeLogFile(string dir){
	delete TELogFile;
	TELogFile=new Record(dir+"TELogFile.txt",0,logFile.swatch);
	return true;
}
void TEModule::abort(){
	//WaitForSingleObject(this->hCommMutex,INFINITE);
	if (hRampThread) {
		TerminateThread(hRampThread,0);
		CloseHandle(hRampThread);
	}
	if (hPeriodicRThread){
		TerminateThread(hPeriodicRThread,0);
		CloseHandle(hPeriodicRThread);
	}
	//SetEvent(this->hCommMutex);
	powerOff();
}

void TEModule::powerOn(){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	tecom->setint("2d",1);
	isPowerOn=true;
	//SetEvent(hCommMutex);
}

void TEModule::powerOff(){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	tecom->setint("2d",0);
	//SetEvent(hCommMutex);
	isPowerOn=false;
}

void TEModule::setPropBand(int width){
	CheckExists()
	//set controlType to PID
	//WaitForSingleObject(hCommMutex, INFINITE);
	if (controlType){
		controlType=false;
		tecom->setint("2b",1);
	}
	tecom->setint("1d",width*multiplier);
	//SetEvent(hCommMutex);
}

void TEModule::setIntegGain(double rpm){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	if (controlType){
		controlType=false;
		tecom->setint("2b",1);
	}
	tecom->setint("1e",rpm*100);
	//SetEvent(hCommMutex);
}

void TEModule::setDerGain(double cpm){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	if (controlType){
		controlType=false;
		tecom->setint("2b",1);
	}
	tecom->setint("1f",cpm*100);
	//SetEvent(hCommMutex);
}

void TEModule::setHeatMult(double val){
	CheckExists()
	if (val<0 || val>1)
		return logFile.write("Error TE Module: invalid heat multiplier, must be between 0 and 1",true); 
	tecom->setint("0c",val*100);
}

void TEModule::setColdMult(double val){
	CheckExists()
	if (multiplier==10.0)
	//	return logFile.write("Error TE Moduel: this controller does not support cold multiplier",true);
	if (val<0 || val>1)
		return logFile.write("Error TE Module: invalid cool multiplier, must be between 0 and 1",true); 
	tecom->setint("0d",val*100);
}

double TEModule::getColdMult(){
	CheckExists(0)
	return tecom->getcomval("5d")/100.0;
}

void TEModule::clearAlarmLatch(){
	CheckExists()
	tecom->setint("33",0);
}


void TEModule::setOutputPower(float val){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	if (!controlType){
		controlType=true;
		tecom->setint("2b",2);
	}
	if (multiplier==100.0)
		tecom->setint("1c",val*(-5.11)+.5);
	else
		tecom->setint("1c",val*(-1.2)+.5);
	//SetEvent(hCommMutex);
}

void TEModule::setFixedTemp(double temp,bool wait){
	CheckExists()
	//WaitForSingleObject(hCommMutex, INFINITE);
	if (controlType){
		controlType=false;
		tecom->setint("2b",1);
	}
	setPID(getPID(temp));
	tecom->setint("1c",temp*multiplier+.5);
	powerOn();
	if (wait) this->wait();
	//SetEvent(hCommMutex);
}
double TEModule::getFixedTemp(){
	CheckExists(0)
	if (controlType) {
		//cout<<"We aren't in fixed temperature control (PID)"<<endl;
		return 0;
	}
	char temp[100];
	//WaitForSingleObject(hCommMutex, INFINITE);
	long int val=tecom->getcomval("50");
	//SetEvent(hCommMutex);
	double result=val/multiplier;
	return result;
}
double TEModule::getTemp(){
	CheckExists(0)
	char temp[100];
	//WaitForSingleObject(hCommMutex, INFINITE);
	long int val=tecom->getcomval("01");
	//SetEvent(hCommMutex);
	return (double)(val/multiplier);
}
double TEModule::getTemp2(){
	CheckExists(0)
	char temp[100];
	//WaitForSingleObject(hCommMutex, INFINITE);
	long int val=tecom->getcomval("06");
	//SetEvent(hCommMutex);
	return (double)(val/multiplier);
}

double TEModule::getOutputPower(){
	CheckExists(0)
	char temp[100];
	//WaitForSingleObject(hCommMutex, INFINITE);
	long int val=tecom->getcomval("04");
	//SetEvent(hCommMutex);
	double result;
	if (multiplier==100)
		result=val*100/511;
	else
		result=val*100/255;
	return result;
}

void TEModule::displayControlOff(){
	if (multiplier==100)
		tecom->setint("29",0);
}

void TEModule::displayControlOn(){
	if (multiplier==100)
		tecom->setint("29",5);
}

/*This function should be deleted with recordTempThread. Replaced by periodicRecording/recordThread
void TEModule::recordTemps2(double sampleTime, string fileName){
	CheckExists()
	//Create a new thread so that Temperature could be recorded while running other parts of the instrument.
	HANDLE hRecordThread;
	PeriodicRecordParam *rparam=new PeriodicRecordParam();
	rparam->fName=fileName;
	rparam->sTime=sampleTime;
	rparam->tem=this;
	record=true;
	HANDLE thread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::recordTempThread, (void *) rparam, 0, &TErecordThreadID);
	CloseHandle(thread);
}*/

//sample time in seconds
void TEModule::periodicRecording(double sampleTime,bool wait){
	CheckExists()
	//Create a new thread so that Temperature could be recorded while running other parts of the instrument.
	WaitForSingleObject(hRecordAccess,INFINITE);
	TELogFile->write("Start recording temperatures periodically.",true);
	ReleaseMutex(hRecordAccess);
	
	PeriodicRecordParam *rparam=new PeriodicRecordParam;
	rparam->sTime=sampleTime;
	rparam->tem=this;
	if (hPeriodicRThread){
		logFile.write("Cancelling previous record to start new one",true);
		stopRecording();
	}
	record=true;
	hPeriodicRThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::periodicRecordThread, (void *) rparam, 0, &TErecordThreadID);
	if (wait){//to prevent program from waiting forever, make sure record can be set to true in a different thread.
		WaitForSingleObject(hPeriodicRThread, INFINITE);
		CloseHandle(hPeriodicRThread);
		hPeriodicRThread=NULL;
	}
}

unsigned __stdcall TEModule::periodicRecordThread(void *param){
	PeriodicRecordParam* rp=(PeriodicRecordParam*) param;
	if (!rp->tem->isPresent) return 0;
		WaitForSingleObject(rp->tem->hRecordAccess,INFINITE);
	rp->tem->TELogFile->write("Time\tTemperature\tOutput\tFixed Temp\tHeatsink Temp",false);
	ReleaseMutex(rp->tem->hRecordAccess);
	Timer t;
	t.startTimer();
	int i=0;
	while(rp->tem->record){ //keep recording until terminated by some other part of the program
		WaitForSingleObject(rp->tem->hRecordAccess,INFINITE);
		rp->tem->TELogFile->write(toString(t.getTime())+"\t"+toString(rp->tem->getTemp())+"\t"+toString(rp->tem->getOutputPower())+"\t"+toString(rp->tem->getFixedTemp())+"\t"+toString(rp->tem->getTemp2())+"\t");
		ReleaseMutex(rp->tem->hRecordAccess);
		i++;
		t.waitAfterLastStart(i*rp->sTime*1000,abortEvent);
		if (WAIT_OBJECT_0==WaitForSingleObject(abortEvent,0))
			_endthreadex(0);
	}
	return 0;
}

unsigned __stdcall TEModule::rampOutputThread(void *param){
	OutputRampParam* r=(OutputRampParam*) param;
	if (!r->temod->isPresent) return 0;
	time_t iniTime, rmpTime;
	time(&iniTime);
	r->temod->setOutputPower(r->iniVolt);
	r->temod->powerOn();
	r->temod->ramp=true;
	
	//Set conditions:
	//Keep ramping unless 1. the target temperature is reached
	//2. the output is 100% or -100% and cannot increase/decrease.
	//3. haven't timed out yet.
	int prevPower, nextPower;
	nextPower=r->iniVolt;
	prevPower=r->iniVolt;
	bool ramp1, ramp2, IsRampUp;
	if (r->endVolt > 0) IsRampUp=false;
	else IsRampUp=true;
	ramp1=true;
	ramp2=true;
	Timer t;
	t.startTimer();

	//Keep ramping until 100% output or target temperature or timeout is reached.
	while (ramp1 && ramp2 && r->temod->ramp && t.getTime()<r->timeout*1000){
		time(&rmpTime);
		//check to see if above criteria have been met.
		if (IsRampUp && r->temod->getTemp() >= r->targetTemp) ramp1=false;// cout<<"1: "<<r->temod->getTemp()<<endl;}
		else if (!IsRampUp && r->temod->getTemp() <= r->targetTemp) ramp1=false;//cout<<"3: "<<r->temod->getTemp()<<endl;}
		else ramp1=true;//cout<<"4: "<<r->temod->getTemp()<<endl;}
		nextPower=difftime(rmpTime, iniTime)*r->slope+r->iniVolt;
		ramp2=(abs(nextPower) < abs(r->endVolt) || abs(nextPower) < 100);
		
		r->temod->setOutputPower(nextPower);
		if (nextPower != prevPower){
			cout << "output= " << r->temod->getOutputPower() << " temperature= " << r->temod->getTemp()<< endl;
			t.resetTimer();
		}
		prevPower=nextPower;
	}
	//After 100% output, keep constant output until target temperature is reached.
	double prevTemp=r->temod->getTemp();//Use prevPower to store previous temperature, power is always 100% after this point.
	while (ramp1 && r->temod->ramp && t.getTime()<r->timeout*1000){
		if (!IsRampUp && r->temod->getTemp()<r->targetTemp) ramp1=false;
		if (IsRampUp && r->temod->getTemp()>r->targetTemp) ramp1=false;	
		if ((!IsRampUp && r->temod->getTemp()<prevTemp)||(IsRampUp && r->temod->getTemp()>prevTemp)){ 
			t.resetTimer();
			prevTemp=r->temod->getTemp();
		}
	}
	r->temod->powerOff();
	r->temod->ramp=false;
	cout << "Temperature ramp finished. Temperature = " << r->temod->getTemp() << endl;
	return 0;
}

unsigned __stdcall TEModule::rampPIDThread(void *param){
	PIDRampParam* r=(PIDRampParam*) param;
	if (!r->temod->isPresent) return 0;
	double startTemp=r->temod->getTemp();

	cout<<"Linearly ramping temperature from "<<startTemp<<" to "<<r->temp<<" in "<<r->time<<" minutes"<<endl;

	double TempResolution=.1;//.1 degree resolution..could make this smaller
	int heatORcool=-1;//cooling
	if (r->temp-startTemp>0) heatORcool=1;//heating

	double finalTemp=r->temp;
	r->temp=finalTemp-5*heatORcool;
	double secondsPerTempResolution=TempResolution*r->time*60*heatORcool/(r->temp-startTemp);
	Timer t;
	Timer t2;
	double timeStep;
	t.startTimer();
	r->temod->setDefaultPID();
	r->temod->tecom->setint("1c",(startTemp)*r->temod->multiplier+.5);//r->temod->setFixedTemp(startTemp+TempResolution,false);
	r->temod->powerOn();
	while(r->temod->ramp && t.getTime()<r->time*60*1000){
		t2.wait(secondsPerTempResolution*1000);
		r->temod->tecom->setint("1c",(startTemp+(r->temp-startTemp)*t.getTime()/(r->time*60*1000))*r->temod->multiplier+.5);//heatORcool*TempResolution)*r->temod->multiplier+.5);
	}
	/*int i;
	for(i=1;i<((r->temp-startTemp)*heatORcool/TempResolution);i++){
		if (!r->temod->ramp) break;
		timeStep=secondsPerTempResolution*i*1000.0;
		
		while(t.getTime()<timeStep && r->temod->ramp){}
		t2.startTimer();
		r->temod->setFixedTemp(startTemp+TempResolution*heatORcool*(1+i),false);
		t2.stopTimer();
	}
	*/
	r->temod->setPID(r->temod->getPID(finalTemp));
	r->temod->setFixedTemp(finalTemp,false);
	//int end=r->temod->getFixedTemp()+.5;
	//assert(end==((int)(r->temp+.5)));
	//cout<<"setFixedTemp took: "<<t2.getTime()/(i-1)<<"ms per call"<<endl;
	CloseHandle(r->temod->hRampThread);
	r->temod->hRampThread=NULL;
	delete r;
	return 0;
}
/* This function should be deleted. Replaced by recordThread.
unsigned __stdcall TEModule::recordTempThread(void *param){
	PeriodicRecordParam* rp=(PeriodicRecordParam*) param;
	if (!rp->tem->isPresent) return 0;
	rp->tem->record=true;
	//Create a file for recording.
	string dir=rp->fName.substr(0,rp->fName.find_last_of('\\',rp->fName.length()-1));
	SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	ofstream recordFile(rp->fName.c_str(),ios::ate);

	while(rp->tem->record){ //keep recording until terminated by some other part of the program
		recordFile << time(NULL) << "	" << rp->tem->getTemp() << "	" << rp->tem->getOutputPower() << endl;
		Sleep(rp->sTime*1000);
	}
	return 0;
}*/

void TEModule::linearOutputRamp(double timeout,bool wait){
	float iniVolt, endVolt, slope, targetTemp;
	cout << "Please type in parameters in the following order: "
		<< "initial voltage (%), end voltage (%), slope, target temperature(C)\n";
		//<< "ramp time, target temperature\n";
	cin >> iniVolt >> endVolt >> slope >> targetTemp;
	OutputRampParam *rmp=new OutputRampParam();
	rmp->iniVolt=iniVolt;
	rmp->endVolt=endVolt;
	rmp->slope=slope;
	rmp->targetTemp=targetTemp;
	rmp->timeout=timeout;
	rmp->temod=this;
	cout << "Current temperature is " << getTemp() << endl;
	cout << "Current output power is " << getOutputPower() << endl;
	//HANDLE hthreadrp;
	ramp=true;
	unsigned threadIDrp, threadIDrd;
	HANDLE hRampThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::rampOutputThread, rmp, 0, &threadIDrp);
	WaitForSingleObject(hRecordAccess,INFINITE);
	TELogFile->write("TE Parameters: \nInitial voltage: " +toString(iniVolt) +" End voltage: " +toString(endVolt) + " Slope: "+toString(slope)+" Target temperature: " +toString(targetTemp),DEBUGCONTROLLER);
	ReleaseMutex(hRecordAccess);
	if (wait){
		WaitForSingleObject(hRampThread, INFINITE);
		CloseHandle(hRampThread);
		hRampThread=NULL;
	}
}

void TEModule::setDefaultPID(){
	if (multiplier==10.0)
		setPID(vlist_of<double>(0)(TE24PROPBAND)(TE24INTGAIN)(TE24DERGAIN)(TE24HEATMULT)(1));
	else
		setPID(vlist_of<double>(0)(TE36PROPBAND)(TE36INTGAIN)(TE36DERGAIN)(TE36HEATMULT)(TE36COOLMULT));
}
void TEModule::stopRecording(){
record=false;
		WaitForSingleObject(hPeriodicRThread, INFINITE);
		CloseHandle(hPeriodicRThread);
		hPeriodicRThread=NULL;
		}

void TEModule::stopRamp(){
	ramp=false;
		WaitForSingleObject(hRampThread, INFINITE);
		CloseHandle(hRampThread);
		hRampThread=NULL;
}
//time in minutes
void TEModule::linearTempRamp(double temp, double time, bool wait,int propBand, double integGain, double derGain){
	CheckExists()
	//setPID(getPID(temp));
	//setDefaultPID();
	//if(propBand!=-1) setPropBand(propBand);
	//if(integGain!=-1) setIntegGain(integGain);
	//if(derGain!=-1) setDerGain(derGain);
	PIDRampParam* r=new PIDRampParam();
	r->temod=this;
	r->temp=temp;
	r->time=time;
	if (hRampThread){
		logFile.write("Linear Ramp already running....killing it",true);
		stopRamp();
	}
	ramp=true;
	hRampThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::rampPIDThread, (void *) r, 0, &TErampThreadID);
	if (wait){
		WaitForSingleObject(hRampThread, INFINITE);
		CloseHandle(hRampThread);
		hRampThread=NULL;
	}
}
/*This function should be deleted and replaced by the above linearTempRamp with PID set to some default.
void TEModule::linearTempRamp(double temp, double time, bool wait){
	CheckExists()
	PIDRampParam* r=new PIDRampParam();
	r->temod=this;
	r->temp=temp;
	r->time=time;
	
	hRampThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::rampPIDThread, (void *) r, 0, &TErecordThreadID);
	if (wait){
		WaitForSingleObject(hRampThread, INFINITE);
		CloseHandle(hRampThread);
		hRampThread=NULL;
	}
}*/

//linear temperature ramp
void TEModule::wait(double deg,double min){
	CheckExists()
	if (deg==-1) deg=waitTolerance;
	if (min==-1) min=waitTimeout;
	if (hRampThread) {
		WaitForSingleObject(hRampThread,INFINITE);
		CloseHandle(hRampThread);
		hRampThread=NULL;
	}
	if (controlType || !isPowerOn){
		logFile.write("TEModule: cannot wait. control type must be PID and power must be ON.",DEBUGTEMODULE);
		return;
	}
	Timer t;
	double temp;
	t.startTimer();
	double fixed=getFixedTemp();
	while(true){
		temp=getTemp();
		if (::abs(temp-fixed)<=deg)
			break;
		int alarm=isAlarmed();
		if (alarm){
			logFile.write("TE Module has shut down due to alarm condition:"+toString(alarm)+" need to reset alarm latch",true);
			break;
		}
		if (WAIT_OBJECT_0==WaitForSingleObject(abortEvent,0))
			_endthreadex(0);
		int timeout;
		timeout=t.getTime();
		if (timeout>(min*60*1000)){
			cout<<"TE Module timeout at "<<dec<<timeout/1000<<" sec.   Temperature is="<<getTemp()<<endl;
			t.stopTimer();
			break;
		}
	}
}

int TEModule::isAlarmed(){
	CheckExists(false);
	long val=0;
	if (multiplier==100.0)
		val=tecom->getcomval("05");
	return val;
}