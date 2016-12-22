// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include "USB.h"
#include "Changer.h"
#include "LightSource.h"
#include "Record.h"
extern Record logFile;
class Lambda10_3{
public:
	Lambda10_3();//open USB connection and initialize shutters and FilterWheel wheel positions
			 //also establish default speeds and other settings
	~Lambda10_3();//close USB connection and reset shutters and FilterWheel wheel positions
	//void SwitchExFilter(int pos,bool w8=false); //change excitation filter to position "pos"
	//void SwitchEmFilter(int pos,bool w8=false);//change emmission filter to position "pos"
	void openShutterA(bool w8=false);
	void closeShutterA(bool w8=false);
	void openShutterB(bool w8=false);
	void closeShutterB(bool w8=false);
	void switchFilterA(int pos, bool wait=false);
	void switchFilterB(int pos, bool wait=false);
	int getFilterA(){return posA;}
	int getFilterB(){return posB;}
	void waitFilterA(){wait();}
	void waitFilterB(){wait();}

	void CloseShutters(bool w8=false);
	//void CloseShuttersSwitchFilters(int posEx, int posEm, bool w8=false);
	void OpenShutters(bool w8=false);
	//void SwitchFilters(int posEx,int posEm, bool w8=false);
	void wait();//.....should return when the filter is not moving
	//void waitShutters();
	//void waitFilters();
	static const int OPEN=0;
	static const int FITC=1;
	static const int CY3=2;
	static const int CY5=3;
private:
	bool isPresent;
	bool waited;
	DWORD bytes_sent;
	USB Fusb;//USB communication object
	//filter wheel positions are between 0 and 9
	int posA;//position of filter wheel A (excitation)
	int posB;//position of filter wheel B (emmission)
	bool Aopen;//TRUE if shutter A is open
	bool Bopen;//TRUE if shutter B is open
	static const int speed=2;//should be between 0 and 7

};

class FilterWheel:public Changer, public LightSource{
public:
	Lambda10_3* lambda;
	const bool isA;//is filter wheel A?  if false then it is filter wheel B
	FilterWheel(Lambda10_3* lambda,bool isA):lambda(lambda),isA(isA){}
	~FilterWheel(){}
	void set(double position){if (isA) lambda->switchFilterA(position); else lambda->switchFilterB(position);}
	double get(){if (isA) return lambda->getFilterA(); else return lambda->getFilterB();}
	void wait(){lambda->wait();}

	//light source methods
	void on(int pos,double intes){
		if (isA) {
			lambda->openShutterA();
			lambda->switchFilterA(pos);
		}else {
			lambda->closeShutterB();
			lambda->switchFilterB(pos);
		}
	}
	void off(){
		if (isA) 
			lambda->closeShutterA();
		else 
			lambda->closeShutterB();
	}
	void enterRingBuffer(AcquisitionGroup ag){logFile.write("Error: Filter Wheels cannot be triggered");}
	void exitRingBuffer(){}
	std::string intensityToString(double intensity){return "100%";}
	double getIntensity(double intensity=-1,std::string intensityUnit=""){return 1;}
	//bool isValidIntensity(double intensity){return true;}
};