// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
//#include "IOPort.h"
#pragma once
#include "LightSource.h"
#include "Trigger.h"
#include <string>

using namespace std;

#define DGFULL 1  //full max intensity position offset
#define DGHALF .5  //one half max intensity position offset
#define DGTHIRD .33 //one third max intensity position offset
class DG:public LightSource,public Trigger{
public:

	//implementation of abstract LightSource class
	void on(int pos, double intensity);
	void off();
	void wait();
	void enterRingBuffer(AcquisitionGroup& ag);
	void exitRingBuffer();
	bool supportsRingBuffer(){return true;}
	std::string intensityToString(double intensity,Objective* obj=NULL);
	double getIntensity(double intensity=-1,std::string intensityUnit="",Objective* obj=NULL);
	//bool isValidIntensity(double intensity){if (intensity==DGFULL || intensity==DGHALF || intensity==DGTHIRD) return true; else return false;}	


	//implementation of abstract Trigger class
	void prepareDigitalLine(int line){}
	void prepareAnalogLine(int line){}
	void setLineHigh(int line);
	void setLineLow(int line);
	void setVoltage(int line, double voltage){}
	double getVoltage(int line){return 0;}
	void setWaveform(AcquisitionGroup channels);
	void prepareWaveform();
	void generateWaveform();
	void waitGenerationStart(){return;}
	bool isValidLine(int triggerLine){if (triggerLine==1 || triggerLine==0) return true; else return false;}
	bool isValidAnalogOutLine(int triggerLine){return false;}
	bool isValidAnalogInLine(int triggerLine){return false;}
	vector<AcquisitionChannel> channels;


	//actual implementation

	void DGControl();
	


	int chan;
	bool isPresent;
	DG(Trigger* t=0, int triggerLine=0,double defaultIntensity=1);//turn ON turbo blanking
	~DG();
	void switchFilter(BYTE pos);
	void closeShutter();
	void delay(double microsec);//delay execution for microsec amount of time
	UINT OUTPUT;
	double del;
	static void errorExit();
	
private:
	void triggerBoth(double d, float exps);
	bool bufferMode;
	LARGE_INTEGER start;
	UINT INPUT;
	BYTE currentPosition;
};