// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, June 14, 2011</lastedit>
// ===================================
#include "Warn.h"
#include "Pump.h"
#include <sstream>
#include <iostream>
#include "Timer.h"
#include <process.h>
#include <limits>
#include "Controller.h"

#include "Chamber.h"
#include "ProtocolEric.h"
#include "ProtocolWalsh.h"

extern Controller cont;
extern customProtocol scan;
extern Chamber currentChamber;
extern Record logFile;
extern HANDLE abortEvent;
extern HWND console;
double Pump::MINSPEED=-1;


bool Pump::checkError(HRESULT val){
	unsigned long hr=0;
	if (val != S_OK) {
		logFile.write("Pump: function returned an error",true);
		hr=val;
		if ((hr & 0x0000F000)==0x0000C000){//pump command error
			if (int((hr & 0x0000000F))==6){//ignore this guy
				logFile.write("...error 6 will be ignored",true);
				return true;
			}
			logFile.write("Pump Command Error on device "+toString(int((hr & 0x000000F0)>>4))+".  Error is "+toString(int((hr & 0x0000000F))),true);
		}
		if ((hr & 0x0000F000)==0x0000B000){//com error
			logFile.write("Pump COM Port Error on COM "+toString(int((hr & 0x00000FF0)>>4))+".  Error is "+toString(int((hr & 0x0000000F))),true);
		}else if ((hr & 0x0000F000)==0x0000A000){//general error
			logFile.write("Pump General Error is "+toString(int((hr & 0x0000000F))),true);
		}
		return false;
	}
	return true;
}

Syringe* Pump::getSyringe(int devnum){
	for(vector<Syringe*>::iterator i=this->syringes.begin();i!=syringes.end();i++){
		if ((*i)->devNum==devnum)
			return *i;
	}
	if (devnum==-1 && syringes.size()>0){
		return syringes.front();
	}
	return &dummySyringe;
}

bool Pump::isDummySyringe(Syringe* s){
	return s==&dummySyringe;
}

bool Pump::isDummyValve(Valve* v){
	return v==&dummyValve;
}

Valve* Pump::getValve(int devnum){
	for(vector<Valve*>::iterator i=this->valves.begin();i!=valves.end();i++){
		if ((*i)->devNum==devnum)
			return *i;
	}
	return &dummyValve;
}

Valve* Pump::get1stValve(){
	if (valves.size()>0){
		return valves.front();
	}
	return &dummyValve;
}

Valve* Pump::get2ndValve(){
	if (valves.size()>1){
		return valves.at(1);//2nd element
	}
	return &dummyValve;
}

Valve* Pump::get3rdValve(){
	if (valves.size()>2){
		return valves.at(2);//3rd element
	}
	return &dummyValve;
}


Pump::Pump(int COM)\
:isPresent(true),\
pumpLog(0),\
oscillating(false),\
hOsc(0),\
b(255)\
{
dummyValve.pmp=this;
dummySyringe.pmp=this;
/*
#ifndef OBSERVER
chamberWidth(4),
air(-1,false,1,7,"Air"),
waste(-1,false,1,8,"Waste"),
mainWash(0,false,1,1,"Wash0"),
s(SYRINGE,2.5,3000,1,2,3){
	washes.push_back(mainWash);
	washes.push_back(Solution(1,false,1,3,"Wash1"));
	washes.push_back(Solution(1,false,1,5,"Wash2"));
	reagents.push_back(Solution(1,false,1,2,"Reagent0"));
	reagents.push_back(Solution(1,false,1,4,"Reagent1"));
	reagents.push_back(Solution(1,false,1,6,"Reagent2"));
#endif
	*/
	//USES_CONVERSION;
	//system("Regsvr32 /s ./bin/PumpCommServer.dll");
	//initializes the COM system
	isCoInit=true;
	HRESULT hCoInit=::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if (hCoInit!=S_OK){
		isCoInit=false;
		//logFile.write("Could not initialize pump server as multithreaded",true);
		//isPresent=false;
		//return;
	}
	//initialize the server
	HRESULT h=pumpServer.CoCreateInstance(__uuidof(PumpComm));
	if (h != S_OK){
		isPresent=false;
		logFile.write("Pump NOT PRESENT: Could not initialize COM",true);
		return;
	}
	pumpLog=CreateFile ((cont.workingDir+"PumpLog.txt").c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);
	//this will open the com port even if it is in use
	char tmp[100];
	sprintf(tmp,"COM%i",COM);
/*	HANDLE hCom = CreateFile(tmp,GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,//OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
	if (hCom==INVALID_HANDLE_VALUE){
		DWORD dw=GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );


		logFile.write(string("Pump: Could not open com port ")+toString(PUMPCOM)+". Error is: "+(LPCTSTR)lpMsgBuf,true);
		isPresent=false;
		return;
	}
	//then we close the com port so that it can be opened by the pump server
	EscapeCommFunction(hCom,CLRRTS);
    CloseHandle(hCom);
	*/
	
	PumpErrChk(pumpServer->PumpSetLogFileID(reinterpret_cast<long>(pumpLog)));
	PumpErrChk(pumpServer->put_EnableLog((VARIANT_BOOL) 0));
	EBaudRate baud=Baud9600;
	if (S_OK != pumpServer->PumpDetectComm(COM)){
		logFile.write(string("Pump NOT PRESENT on COM")+toString(COM)+": access is denied",true);
		isPresent=false;
		return;
	}

	PumpErrChk(pumpServer->put_BaudRate(baud));
	PumpErrChk(pumpServer->put_CommandAckTimeout(10));//500ms timeout
	if (S_OK != pumpServer->PumpInitComm(COM)){
		logFile.write(string("Pump not present on COM")+toString(COM),true);
		isPresent=false;
		return;
	}
	long devStatus;
	int deviceNum;
	int i=0;
	//find all devices at 9600
	vector<Syringe> s9600;
	vector<Valve> v9600;
	vector<Syringe> s38400;
	vector<Valve> v38400;
	for(;i<=15;i++){
		deviceNum=i;	
		PumpErrChk(pumpServer->PumpCheckDevStatus(deviceNum,&devStatus));
		if (devStatus!=1 && devStatus!=0){
			if (devStatus==-1)
				logFile.write("COM error during device detection. Dev num: "+deviceNum,true);
			continue;
		}else{
			//find out if valve or syringe
			if (devStatus==0){//busy{
				logFile.write("Syringe was busy. Plunger movement stopped. Need to reinitialize before use.",true);
				PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("T"),deviceNum,&(b.m_str)));
				wait(deviceNum);
				PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("V5R"),deviceNum,&(b.m_str)));//clear previous command	
			}	
			if (deviceNum%2==0)//syringes are even
				s9600.push_back(Syringe(this,deviceNum,2500,3000,24000,1));
			else//valves are odd
				v9600.push_back(Valve(this,deviceNum));
		}
	}
	//find all devices at 38400
	baud=Baud38400;
	PumpErrChk(pumpServer->PumpExitComm());
	PumpErrChk(pumpServer->put_BaudRate(baud));
	if (S_OK != pumpServer->PumpInitComm(COM)){
		logFile.write(string("Pump not present on COM")+toString(COM),true);
		isPresent=false;
		return;
	}
	i=0;
	for(;i<=15;i++){
		deviceNum=i;	
		PumpErrChk(pumpServer->PumpCheckDevStatus(deviceNum,&devStatus));
		if (devStatus!=1 && devStatus!=0){
			continue;
		}else{
			//find out if valve or syringe
			if (devStatus==0){//busy{
				logFile.write("Syringe was busy. Plunger movement stopped. Need to reinitialize before use.",true);
				PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("T"),deviceNum,&(b.m_str)));
				wait(deviceNum);
				PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("V5R"),deviceNum,&(b.m_str)));//clear previous command	
			}	
			if (deviceNum%2==0)
				s38400.push_back(Syringe(this,deviceNum,2500,3000,24000,1));
			else
				v38400.push_back(Valve(this,deviceNum));
		}
	}
	//choose vector to reset baud rates
	vector<Valve> vBad; //need to change baud rates
	vector<Valve> vGood; //baud rate is correct
	vector<Syringe> sGood;//baud rate is correct
	CComBSTR cmd;
	if (s9600.size()>0){
		if (s38400.size()>0){
			logFile.write(string("Pump not available: Multiple syringes found at different baud rates")+toString(COM),true);
			isPresent=false;
			return;
		}
		baud=Baud38400;
		vBad=v38400;
		vGood=v9600;
		sGood=s9600;
		cmd=CComBSTR("U41R");
	}else{
		baud=Baud9600;
		vBad=v9600;
		vGood=v38400;
		sGood=s38400;
		cmd=CComBSTR("U47R");		
	}
	PumpErrChk(pumpServer->PumpExitComm());
	PumpErrChk(pumpServer->put_BaudRate(baud));
	if (S_OK != pumpServer->PumpInitComm(COM)){
		logFile.write(string("Pump not present on COM")+toString(COM),true);
		isPresent=false;
		return;
	}
	//change bad baud rates
	for(vector<Valve>::iterator viBad=vBad.begin();viBad!=vBad.end();viBad++){
		PumpErrChk(pumpServer->PumpSendCommand(cmd,viBad->devNum,&(b.m_str)));
		pumpServer->PumpSendCommand(CComBSTR("!R"),viBad->devNum,&(b.m_str));
	}
	//sort valves by dev number
	for(vector<Valve>::iterator viBad=vBad.begin(),viGood=vGood.begin();viBad!=vBad.end() || viGood!=vGood.end();){
		if (viBad==vBad.end()){
			this->valves.push_back(new Valve(*viGood));
			viGood++;
		}else if (viGood==vGood.end()){
			this->valves.push_back(new Valve(*viBad));
			viBad++;
		}else if (viBad->devNum>viGood->devNum){
			this->valves.push_back(new Valve(*viGood));
			viGood++;
		}else{
			this->valves.push_back(new Valve(*viBad));
			viBad++;
		}
	}
	//add syringes
	for(vector<Syringe>::iterator si=sGood.begin();si!=sGood.end();si++){
		this->syringes.push_back(new Syringe(*si));
	}
	//change baud rate back
	if (baud==Baud38400){
		baud=Baud9600;
	}else{
		baud=Baud38400;
	}
	PumpErrChk(pumpServer->PumpExitComm());
	PumpErrChk(pumpServer->put_BaudRate(baud));
	if (S_OK != pumpServer->PumpInitComm(COM)){
		logFile.write(string("Pump not present on COM")+toString(COM),true);
		isPresent=false;
		return;
	}
	
	logFile.write("Pump class found "+::toString((int)syringes.size())+" syringe(s) and "+::toString((int)valves.size())+" valve(s)",true);
	cout<<"Pump and Valve ready"<<endl;
}	

Pump::~Pump(){
	for(vector<Syringe*>::iterator i=syringes.begin();i!=syringes.end();i++){
		delete (*i);
	}
	for(vector<Valve*>::iterator i=valves.begin();i!=valves.end();i++){
		delete (*i);
	}
	//uninitializes the COM system and COM port
	if (pumpServer) pumpServer->PumpExitComm();
	pumpServer = NULL;
	if (isCoInit)
		::CoUninitialize();
	CloseHandle(pumpLog);
}

void Pump::pumpControl(){
	char c;
	string t;//temp string for input output
	string t2;
	string t3;
	string filename;
	int devnum;
	Valve* v;
	Syringe* s;
	while(true){
		try{
		cout<<"Please select a task"<<endl;
		cout<<"0: Initialize All Devices"<<endl;
		cout<<"1: FluidicsSetup Control"<<endl;
		cout<<"2: Roller Pump Protocols"<<endl;
		cout<<"3: Walsh Pump Protocols"<<endl;
		cout<<"4: Send PumpLink command (no R)"<<endl;
		cout<<"5: Send PumpLink command No Execute (no R)"<<endl;
		cout<<"6: Execute Command"<<endl;
		cout<<"7: Disable Microstepping"<<endl;
		cout<<"8: Enable Microstepping"<<endl;
		cout<<"9: Initialize single device"<<endl;
		cout<<"q: Query last answer"<<endl;
		cout<<"s: Stop first syringe"<<endl;
		cout<<"w: waste syringe"<<endl;
		cout<<"e: Exit Pump Control"<<endl;
		c=getChar();
		switch(c){
			case '0'://initialize all
				{
					for(vector<Syringe*>::iterator i=this->syringes.begin();i!=this->syringes.end();i++){
						(*i)->init();
					}
					for(vector<Valve*>::iterator i=this->valves.begin();i!=this->valves.end();i++){
						(*i)->init();
					}
				break;}
			case '9'://initialize 
				{
				cout<<"Initialize a syringe or a valve? (s or v)"<<endl;
				char c=getChar();
				if (c=='s')
					selectSyringe()->init();
				else if (c=='v')
					selectValve()->init();
				break;}
			case '1'://fludics control  
				scan.fs.fluidicsControl();
				break;
			case '2'://Roller protocols  
				ProtocolEric::pumpProtocols();
				break;
			case '3'://Walsh's protocols  
				ProtocolWalsh::pumpProtocols();
				break;
					case '4'://send command
				{int dev;

				cout<<"Please type in the command\n";
				t=getString();
				cout<<"Which device number to write to?\n";
				dev=getInt();
				sendCommand(t,dev);
				break;}
			case '5'://send pumplink command no execute
				{int dev;
				cout<<"Please type in the command\n";
				t=getString();
				cout<<"Which device number to write to?\n";
				dev=getInt();
				sendCommandNoExecute(t,dev);
				break;}
		
			case '6'://Execute pumplink command
				{int dev;
				cout<<"Which device number to write to?\n";
				dev=getInt();
				executeCommand(dev);
				break;}
			case '7'://disable microstepping
				s=selectSyringe();
				s->disableMicrostepping();
				break;
			case '8'://enable microstepping
				s=selectSyringe();
				s->enableMicrostepping();
				break;
			case 'Q':{ //get last answer
				cout<<"Enter device number for querying last answer"<<endl;
				devnum=getInt();
				cont.pmp.pumpServer->PumpGetLastAnswer(devnum,&(b.m_str));
				int ret=toInt(wstring(b));
				cout<<ret<<endl;;
				break;}
			case 's':
				{
					if (syringes.size()>0) syringes.front()->stop();
				}break;
			case 'w':
				{
					selectSyringe()->waste();
				}break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
		}catch(std::ios_base::failure e)
			    {
					logFile.write("Pump Control Menu Exception: "+string(e.what()),true);
					continue;
				}
	}
}


Syringe* Pump::selectSyringe(){
	if (syringes.size()<1){
		cout<<"No syringes to select from"<<endl;
		return &dummySyringe;
	}
	cout<<"Please select a Syringe"<<endl;
	int ind=0;
	for(vector<Syringe*>::const_iterator i=syringes.begin();i!=syringes.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	ind=getInt();
	if (ind>syringes.size()-1 || ind<0){
		logFile.write("invalid syringe selected.",true);
		return &dummySyringe;
	}
	return syringes.at(ind);
}

Valve* Pump::selectValve(){
	if (valves.size()<1){
		cout<<"No valves to select from"<<endl;
		return &dummyValve;
	}
	cout<<"Please select a Valve"<<endl;
	int ind=0;
	for(vector<Valve*>::const_iterator i=valves.begin();i!=valves.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	ind=getInt();
	if (ind>valves.size()-1 || ind<0){
		logFile.write("invalid valve selected.",true);
		return &dummyValve;
	}
	return valves.at(ind);
}

void Pump::testPumpCommDelay(){
	logFile.clear();
	//check position multiple times
	Timer t;
	int numTimes=1000;
	logFile.write("GetPosition Times:");
	for(int i=0;i<numTimes;i++){
		t.startTimer();
		syringes.front()->getPosition();
		t.stopTimer();
		logFile.write(toString(t.getTime())+"\t");//getTime()-stdDev.back());
		t.resetTimer();
	}
	logFile.write("Plunger move, timing differences 1000,100 (actual, calc):");
	syringes.front()->waste();
	double calc,actual;
	int num=50;
	for(int i=0;i<num;i++){
		actual=testPullTiming(2500,100,calc);
		logFile.write(toString(actual)+"\t"+toString(calc)+"\t");
	}
	logFile.write("Plunger move, timing differences 100,10 (actual, calc):");
	for(int i=0;i<num;i++){
		actual=testPullTiming(250,10,calc);
		logFile.write(toString(actual)+"\t"+toString(calc)+"\t");
	}
	logFile.write("Plunger move, timing differences 25,2.5 (actual, calc):");
	for(int i=0;i<num;i++){
		actual=testPullTiming(62.5,2.5,calc);
		logFile.write(toString(actual)+"\t"+toString(calc)+"\t");
	}
	//cont.pmp.wait();
}

double Pump::testPullTiming(double vol, double uLps, double& calcTime){
	//cont.pmp.wait();

	//cont.pmp.sendCommand(cont.pmp.s.moveValve(reagents.at(0)),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
	//cont.pmp.sendCommandNoExecute(cont.pmp.s.pull(reagents.at(0),vol,uLps,&calcTime),cont.pmp.s.deviceNum);
	//Timer::wait(1000);
	Timer t2;
	//cont.pmp.executeCommand(cont.pmp.s.deviceNum);
	t2.startTimer();
	//cont.pmp.wait(cont.pmp.s.deviceNum);
	t2.stopTimer();
	//cont.pmp.sendCommand(cont.pmp.s.waste(),cont.pmp.s.deviceNum);
	return t2.getTime();
}

void Pump::terminate(int devNum){
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("T"),devNum,&(b.m_str)));
	while(isBusy(devNum));
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("V5R"),devNum,&(b.m_str)));//clear previous command	
}

string Pump::sendCommand(string command, int devNum){
	CheckExists("")
	if (pendingCommand(devNum))
		logFile.write("Pump error: previous command sent to pump will not be executed",true);
	
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR(string(command+"R").c_str()),devNum,&(b.m_str)));
	wstring val=wstring(b);
	string s(val.begin(), val.end());
	return s;
	/*
	bool resp=PumpErrChk(pumpServer->PumpSendCommand(CComBSTR(string(command+"R").c_str()),devNum,&(b.m_str)));
	std::string s;
	wstring ws(b);
	s.assign(ws.begin(), ws.end()); 
	if (!resp) 
		logFile.write("Pump error response was: "+s,true);
	return s;
	*/
}

void Pump::sendCommandNoExecute(string command, int devNum){
	CheckExists()
	if (pendingCommand(devNum))
		logFile.write("Pump error: previous command sent to pump will not be executed",true);
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR(command.c_str()),devNum,&(b.m_str)));
}

void Pump::executeCommand(int devNum){
	CheckExists()
	if (!pendingCommand(devNum))
		logFile.write("Pump Error: no command in buffer to execute",true);
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("R"),devNum,&(b.m_str)));
}

bool Pump::pendingCommand(int devNum){
	if (!cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("F"),devNum, &(b.m_str))))
		return true;
	wstring val=wstring(b);
	int ret=0;

	if (val.empty()){//retry once
		if (!cont.pmp.PumpErrChk(cont.pmp.pumpServer->PumpSendCommand(CComBSTR("F"),devNum, &(b.m_str))))
			return true;
		val=wstring(b);
		int ret=0;
	}
	ret=toInt(val);
		if (ret) 
			return true;
		else 
			return false;
}

void Pump::wait(int devNum){
	CheckExists()
		DWORD result;
	while(isBusy(devNum)){
		result=WaitForSingleObject(abortEvent,1);
		if (result==WAIT_OBJECT_0){
		//	PumpErrChk(pumpServer->PumpSendNoWait(CComBSTR("T"),s.deviceNum));
		 throw abortException("program aborted in pump wait function");
		}
	}
}

bool Pump::isBusy(int devNum, int num){
	if (num>10)
		throw exception(string("Pump Error: 10 failed attempts to CheckDevStatus of device "+toString(devNum)+".").c_str());
	//this is non blocking but cannot be stopped correctly, use device status instead	
	//PumpErrChk(pumpServer->PumpWaitForDevice(devNum));
	long devStatus=0;
//	if (pendingCommand(devNum))
//		logFile.write("Pump error: previous command has not been executed...no need to wait",true);	
	PumpErrChk(pumpServer->PumpCheckDevStatus(devNum,&devStatus));
	switch (devStatus){
		case -1: //COM error
			logFile.write("Pump: COM error during CheckDevStatus for device "+toString(devStatus)+". Attempting function call again.",true);
			return isBusy(devNum,num+1);
		case  0: //Busy
			return true;
		case  1: //Diluter ready
			return false;
		case  2: //Time out error
			logFile.write("Pump: COM timeout error during CheckDevStatus for device "+toString(devNum)+". Attempting function call again.",true);
			return isBusy(devNum,num+1);
		default:
			throw exception(string("Pump Error: unknown return value "+toString(devNum)+ " from CheckDevStatus for device "+toString(devNum)+".").c_str());
	}

	//obviously the code below is never reached
	PumpErrChk(pumpServer->PumpSendCommand(CComBSTR("Q"),devNum,&(b.m_str)));
	wstring val=wstring(b);
	int ret=0;
	ret=toInt(val)&0x00000020;
	return !bool(ret);
}





/*
void Pump::recycleReagent(){	
	if (!filled){
		logFile.write("Pump: Error, attempt to recycle reagent when there was none in chamber");
		return;
	}
	if (!current.isRecyclable) {
		//just unprime unused reagent
	}else{
		//throw away unused wash and air gap and then unprime reagent
		s.wait();
		waste.valveSelect(); //waste should always be on first valve
		sendCommand(s.push(VALVE2CHAMBERTUBING-(OVERSHOOT)*CHAMBERLENGTH*CHAMBERHEIGHT*chamberWidth,current.mmps),s.deviceNum);
		s.wait();
		current.valveSelect();
		sendCommand(s.push((1+AIRGAP+2*OVERSHOOT)*CHAMBERLENGTH*CHAMBERHEIGHT*chamberWidth,current.mmps),s.deviceNum);
	}
	unprimeReagent();
	Timer::wait(2000);
	filled=false;
}
*/


void Pump::stop(){
	CheckExists()
		if (!syringes.empty()){
			for (vector<Syringe*>::iterator i=syringes.begin();i!=syringes.end();i++){
				if (isBusy((*i)->devNum)){
					(*i)->stop();
				}
			}
		}
		isPresent=false;
}

void Pump::abort(){
	CheckExists()
	stop();
	if (hOsc) TerminateThread(hOsc, 0);
	//sendCommand("T",s.deviceNum);
	//Sleep(10);
	//sendCommand("T",s.deviceNum);
}




/*

void Pump::pauseReaction(){
	CheckExists()
	stopOscillation();
	if (!filled){
		logFile.write("Pump:  Error, attempt to pause reaction when no reagent was in chamber");
		return;
	}	
	//push wash into chamber.  select waste valve first so unused wash doesnt get pushed into wash container
	
	logFile.write("Pump: Pausing Reaction");
	s.wait();
	waste.valveSelect();
	sendCommand(s.push((3+AIRGAP+2*OVERSHOOT)*CHAMBERLENGTH*CHAMBERHEIGHT*chamberWidth,current.mmps),s.deviceNum);
	s.wait();
}

void Pump::resumeReaction(){
	CheckExists()
	if (!filled){
		logFile.write("Pump:  Error, attempt to resume reaction when no reagent was in chamber");
		return;
	}
	//pull reagent back into chamber.  select air valve first so no contaminated solution will reach chamber
	
	logFile.write("Pump: Resuming Reaction");
	s.wait();
	air.valveSelect();
	sendCommand(s.pull((3+AIRGAP+2*OVERSHOOT)*CHAMBERLENGTH*CHAMBERHEIGHT*chamberWidth,current.mmps),s.deviceNum);
	s.wait();
}

*/
