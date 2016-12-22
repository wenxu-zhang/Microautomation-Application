// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, March 31, 2011</lastedit>
// ===================================
#pragma once
#include <shlobj.h>
#include <string.h>
class comm;
using namespace std;
#define DEBUGTEMODULE true
class TEModule{
public:
	vector<vector<double>> gainScheduleTC36;
	vector<vector<double>> gainScheduleTC24; //vector of quadruples (temp,p,i,d); 
	bool isPresent;
	bool record;//true=contine to record temperature false=stop
	bool triggerRecord;//true=record now, usually set to false.
	bool ramp;//true=continue to ramp temp, false=stop added by ER
	//bool FinishRamp; replaced by ramp
	double waitTolerance;//in degrees C
	double waitTimeout;//in minutes
	string comment;//for recording picture numbers or others defined by other parts of the program
	HANDLE hRampThread;
	//HANDLE hTriggerRThread;
	HANDLE hPeriodicRThread;
	HANDLE hRecordAccess;//Event to access the TELogFile
	void stopRamp();
	void stopRecording();
	double getMultiplier();
	void displayControlOn();
	void displayControlOff();
	//functions
	TEModule(int COMM);//open comm communications and establish default values (sensor type, degrees celsius etc.)
	~TEModule();//close comm communications
	void abort();
	void powerOn();
	void powerOff();
	void setPropBand(int width);//width in degrees Celsius of temperature operation for 0% to 100% power output
	void setIntegGain(double rpm);//.01 ro 10 repeats per minute
	void setDerGain(double cpm);//.01 to 10 cycles per minute
	void setPID(vector<double> params);
	void setDefaultPID();
	vector<double> getPID(double temp);
	void setHeatMult(double mult);//output power scaled by this value for heating
	void setColdMult(double mult);//output power scaled by this value for cooling
	double getColdMult();//used to tell if we are using a TC2425 or a TC3625 controller.  TC2425 does not support Cold side multiplier
	void setOutputPower(float val);//between -100 and 100
	void setFixedTemp(double temp,bool wait=true);//temp in degrees Celsius
	void clearAlarmLatch();
	double getTemp();//return temperature in degrees Celsius
	double getTemp2();//read 2nd thermistor (on the heat sink), return temperature in degrees Celsius
	double getFixedTemp();
	double getOutputPower();//return output in %
	void periodicRecording(double sampleTime,bool wait=false);//record temperatures every "sampleTime" amount of seconds
//	void triggerRecording();//record temperatures every "sampleTime" amount of seconds
	//to be removed: void recordTemps2(double sampleTime, string fileName);//sample time in ms record time in ms
	//to be removed: void linearTempRamp(double temp, double time, bool wait);
	static unsigned __stdcall rampPIDThread(void *param);
	static unsigned __stdcall rampOutputThread(void *param);//ramp temperature with linear voltage output. iniVolt is initial voltage. slope is the slope empirically determined.
	static unsigned __stdcall periodicRecordThread(void *param);
	static unsigned __stdcall triggerRecordThread(void *param);
	//to be removed: static unsigned __stdcall recordTempThread(void *param); 
	void linearTempRamp(double temp, double time, bool wait,int propBand=-1, double integGain=-1, double derGain=-1);
	void linearOutputRamp(double timeout=INFINITE,bool wait=false);
	void wait(double deg=-1,double min=-1); //wait until temperature is within +-deg degrees or timeout at min minutes. Use waitTolerace/waitTimeout as default(deg/min=-1).
	bool changeLogFile(string dir);
	Record *TELogFile;
	int isAlarmed();
private:
	comm* tecom;
	//HANDLE hCommMutex; What is this for? It's never initialized.
	int COMM;//com port number
	double multiplier;
	bool controlType;//false==PID CONTROL(fixed temp)  true=COMPUTER CONTROL(fixed power)
	bool isPowerOn;//0=false 1=true
	unsigned TErecordThreadID, TErampThreadID;
	
};

struct PeriodicRecordParam{
	double sTime;//sample time in seconds
	TEModule *tem;
};
struct OutputRampParam{
	int iniVolt;//initial output%
	double slope;//slope in output%/s
	int endVolt;//end output%
	double targetTemp;
	double timeout;//in seconds
	TEModule *temod;
};
struct PIDRampParam{
	double temp;
	double time;//in minutes
	TEModule *temod;
};