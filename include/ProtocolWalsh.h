// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, May 31, 2011</lastedit>
// ===================================
#pragma once
#include "ProtocolEric.h"
#include "Solution.h"
#include "Syringe.h"
#include "Pump.h"
#include "Controller.h"
#include "Scan.h"
#include "Definitions.h"
#include "Chamber.h"
#include "DefiniteFocus.h"
#include <vector>
 extern Controller cont;
 extern Record logFile;

class ProtocolWalsh{
public:
	static void runProtocol();
	static void chemCleave();
	static void dilutionEffect();
	static void cleavePrep();
	static void dispense(Solution* sol,double uL,FlowChannel* f,Solution* col,FlowChannel* h);
	static void hybRxn(double minTemp=20,double maxTemp=65);
	static void cleavagePrep();
	static void incorpKinetics();
	static void incorpKinetics2();
	static void incorpKinetics3();
	static void ivTrans();
	static void collect(double uL,FlowChannel* f,Solution* col,FlowChannel* h,Solution* out);
	static void glassFunc();
	static void glassFunc_Push();
	static void glassFunc_multiChan();
	static void rxnLoad(Solution* r,FlowChannel* f,double time);
	static void reagLoad(Solution* r,vector<FlowChannel*> f);
	static void pumpKinetics();
	static void cycleSeq();
	static void loadHyb();
	static void cDNAsynth(vector<FlowChannel*> f);
	static void primeChannels(Solution* s,vector<FlowChannel*> f);
	static void totalSurfPrep();

	//global functions not specific to a microscope
	//static void expose(int numFOVs, double numFOVSpacingY,double time, AcquisitionChannel* channel);
	//static void DefiniteFocus();
	//chamber 0 is left 1 is right and 2 is both
	//channel 0 is left 1 is middle 2 is right
	//section 0 is bottom 1 is center 2 is top
	//static void imageMultipleFOVs(customProtocol& scan);
	//static void imageSingleFOV(int FOVNum, customProtocol& scan);
	//static bool loadProtocol(string filename,customProtocol& scan);
	//static bool selectProtocol(customProtocol& scan);
	//static bool reloadProtocolList();
	
	/*
	static unsigned __stdcall kineticsScanThread(void* param);
	static unsigned __stdcall omnicureThread(void* param);
	static unsigned __stdcall electrophoresisThread(void* param);
	*/
	

#ifdef OBSERVER
	
	static void pumpProtocols();
	static void customScan();
#endif
};




