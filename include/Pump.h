// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, November 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include "Solution.h"
#include "Syringe.h"
#include "Valve.h"
#include <atlbase.h>
#include <vector>
#import "PumpCommServer.dll" no_namespace, raw_interfaces_only, named_guids

using namespace std;


#define PumpErrChk(functionCall) checkError(functionCall)


struct OscArg{
	double mmps;
	int msPause;
	double vol;
};

class Pump{
public:
	static double MINSPEED;//used to choose minimum speed (will be -1 internally)
	bool isCoInit;
	bool checkError(HRESULT val);
	//class variables
	CComPtr<IPumpComm> pumpServer;
	HANDLE pumpLog;
	//class methods
	Pump(int COM);
	~Pump();

	//GUI
	void pumpControl();
	Syringe* selectSyringe();
	Valve* selectValve();

	void terminate(int devNum);
	bool isBusy(int deviceNum,int num=0);

	void stop();//graceful, should only be used during washes. not suggested for oscillations
	void abort();//not graceful, stops oscillations immediately

	string sendCommand(std::string command, int devNum);
	void sendCommandNoExecute(std::string command, int devNum);
	
	void executeCommand(int devNum);
	bool pendingCommand(int devNum);

	bool isPresent;

	CComBSTR b;

	//test
	void testPumpCommDelay();
	double testPullTiming(double vol, double uLps,double& calc);
	//variables
	bool oscillating;
	HANDLE hOsc;
	OscArg o;
	
	vector<Syringe*> syringes;
	vector<Valve*> valves;
	Syringe dummySyringe;
	Valve dummyValve;
	//methods
	Syringe* getSyringe(int devnum=-1);
	bool isDummySyringe(Syringe* s);
	bool isDummyValve(Valve* v);
	Valve* getValve(int devnum);
	Valve* get1stValve();
	Valve* get2ndValve();
	Valve* get3rdValve();
	void wait(int devNum);

	
	static unsigned __stdcall oscThread(void *param);
	void shutDown();
	

	friend class Solution;
	friend class Syringe;
	friend class ProtocolEric;
};
