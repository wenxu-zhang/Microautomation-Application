// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
// TEModule.cpp : Defines the entry point for the console application.
//
#include "Warn.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <process.h>
//#include "Controller.h"
#include "PowerMeter.h"
#include "Timer.h"
#include "RS232.h"
#include "Objective.h"
#include "nr.h"
using namespace std;
#include "Controller.h"
#include "Record.h"
extern Record logFile;
extern Controller cont;
extern HANDLE abortEvent;

PowerMeter::PowerMeter(int COM,double zpos, Objective* obj):zpos(zpos),obj(obj),isPresent(true),isDarkCurrentSet(false){	
	if (obj==NULL){
		logFile.write("PM100 Power Meter NOT PRESENT. No suitable 1x Objective position found",true);
		isPresent=false;
		return;
	}
	pm100=new RS232(COM,115200);
	if (!pm100->isPresent){
		logFile.write("PM100 Power Meter NOT PRESENT. Could not open COM"+toString(COM),true);
		isPresent=false;
		return;
	}
	pm100->write(":HEAD:INFO?\n");
//	pm100->write("*IDN?\n");
//	Timer::wait(10000);
	string info=pm100->read("\r\n");
	if (info==""){
		isPresent=false;
		logFile.write("PM100 Power Meter NOT PRESENT on COM"+toString(COM),true);
		return;
		}
//	logFile.write(info,true);
	logFile.write("PM100 Power Meter ready",true);

	//clear error queue max 30 entries
	for(int i=0;i<40;i++){
		pm100->write(":SYST:ERR?\n");
		pm100->read("\r\n");
	}
	setDarkCurrent();
}

PowerMeter::~PowerMeter(){
	CheckExists()
	delete pm100;
}
void PowerMeter::abort(){
}

void  PowerMeter::setDarkCurrent(){
	//Objective* obj=cont.axio.getCurrentObjective();
	//double z=cont.focus->getZ();

	//setObjective();
	NRVec<DP> x(numReadingsDark);
	for(int i=0;i<numReadingsDark;i++){
		pm100->write(":PHOTOCURRENT?\n");
		string current=pm100->read("\r\n");
		//checkError();
		x[i]=toDouble(current);
	}
	double ave,var;
	NR::avevar(x,ave,var);
	double stddev=sqrt(var);
	checkError();
	logFile.write("Power Meter: average current reading was "+toString(ave)+" +/-"+toString(stddev)+ " A (CV of "+toString(100*stddev/ave)+"%)",true);
	//scientific notation doesnt work so we have to use fixed.
//	Timer::wait(1000);
	pm100->write(":DARKCURRENT "+toStringSci(ave)+"\n");
	Timer::wait(1000);
	checkError();
	//cout<<"150 precision of 2 is "<<resetiosflags(ios_base::floatfield)<<setprecision(2)<<150<<endl;
	//if (obj) obj->set();
	//cont.focus->move(z);
	isDarkCurrentSet=true;
}

void  PowerMeter::setWavelength(int nm){
	string send=":WAVELENGTH "+toStringSci(nm*1e-9,2)+"\n";
	pm100->write(send);
	checkError();
}

double PowerMeter::getWavelength(){
	pm100->write(":WAVELENGTH?\n");
	string wave=pm100->read("\r\n");
	checkError();
	logFile.write("PowerMeter set wavelength is "+toString(int(toDouble(wave)*1e9))+" nm",true);
	return toDouble(wave)*1e9;
}

double PowerMeter::readPower(){
	if (!isDarkCurrentSet){
		logFile.write("Cannot read power. Dark current not set",true);
		return 0;
	}
	NRVec<DP> x(numReadingsPower);
	for(int i=0;i<numReadingsPower;i++){
		pm100->write(":POWER?\n");
		x[i]=toDouble(pm100->read("\r\n"))*1000;
		//checkError();
	}
	double ave,var;
	NR::avevar(x,ave,var);
	double stddev=sqrt(var);
	checkError();
	logFile.write("Power Meter: average power reading was "+toString(ave)+" +/-"+toString(stddev)+ " mW (CV of "+toString(100*stddev/ave)+"%)",true);
	return ave;
}

void PowerMeter::setObjective(){
	cont.focus->move(zpos);
	cont.focus->wait();
	obj->set();
	obj->wait();
}

double PowerMeter::getPower(AcquisitionChannel* ac){
	Objective* obj=cont.axio.getCurrentObjective();
	double z=cont.focus->getZ();

	setObjective();
	setWavelength(ac->chan->lite().ls->cp->wavelength);
	AcquisitionChannel myAC(*ac);
	if (cont.OUT_BLOCKED!=-1)
		cont.outPorts.at(cont.OUT_BLOCKED).set();
	ac->chan->on(ac->intensity,true,0);//should be EPI but whatever, close enough
	Timer::wait(1000);
	double ret=readPower();
	ac->chan->off();
	if (obj) obj->set();
	cont.focus->move(z);
	return ret*ac->chan->lite().ls->getObjEff(obj);
}

string PowerMeter::checkError(){
	pm100->write(":SYST:ERR?\n");
	string ret=pm100->read("\r\n");
	stringstream ss;
	ss<<ret;
	string val;
	getline(ss,val,',');
	if (val=="" || toInt(val)!=0)
		logFile.write("Power Meter Error: "+ret,true);
	return ret;
}

void PowerMeter::powerMeterControl(){
	char c;	
	while(true){
		cout<<"Welcome to Power Meter Control\n";
		cout<<"Please select a task"<<endl;
		cout<<"a: check all max powers"<<endl;
		cout<<"b: check light source max power"<<endl;
		cout<<"0: Calibrate all"<<endl;
		cout<<"1: Calibrate light source"<<endl;
		cout<<"2: Set Objective Position"<<endl;
		cout<<"3: Set Dark Current"<<endl;
		cout<<"4: Set Wavelength"<<endl;
		cout<<"5: Get Wavelength"<<endl;
		cout<<"6: Read Power"<<endl;
		cout<<"7: Get Error"<<endl;
		cout<<"8: Prepare for alignment"<<endl;
		cout<<"e: Exit"<<endl;
		c=getChar();
		switch(c){
			case 'a':{
				Objective* obj=cont.axio.getCurrentObjective();
				double z=cont.focus->getZ();
				setObjective();
				for(vector<LightSource*>::iterator i=cont.lightSources.begin();i!=cont.lightSources.end();i++){
					(*i)->checkMaxPower();
				}
				if (obj) obj->set();
				cont.focus->move(z);
				break;
					 }
			 case 'b':
				 LightSource::select()->checkMaxPower();
				 break;
			case '0':{
				Objective* obj=cont.axio.getCurrentObjective();
				double z=cont.focus->getZ();
				setObjective();
				for(vector<LightSource*>::iterator i=cont.lightSources.begin();i!=cont.lightSources.end();i++){
					(*i)->calibrate();
				}
				if (obj) obj->set();
				cont.focus->move(z);
				break;
					 }
			 case '1':
				 LightSource::select()->calibrate();
				 break;
			case '2':
				setObjective();
				break;
			case '3':
				setDarkCurrent();
				break;
			case '4':{
				cout<<"Enter desired wavelength (in nm)"<<endl;
				int wavelength=getInt();
				setWavelength(wavelength);
				break;
					 }
			case '5':
				getWavelength();
				break;
			case '6':
				readPower();
				break;
			case '7':
				logFile.write("Check Error returned: "+checkError(),true);
				break;
			case '8':{
				LightSource* ls=LightSource::select();
				if (!ls->prepareCalChan())
					break;
				ls->onAlign();
				cout<<"Press enter when done"<<endl;
				getString();
				ls->endCalChan();
					  }
				 break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
}
