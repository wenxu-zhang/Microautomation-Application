// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Friday, February 10, 2012</lastedit>
// ===================================
#pragma once
/********************************************************										
 ****   main controller class for all experiments	  ***	
 ********************************************************/
#include "Definitions.h"
#include <stdio.h>
#define NOMINMAX
#include <Windows.h>
#include <string>
#include "Record.h"
#include "OutPort.h"
#include "DisplayList.h"
#include "ThreadList.h"
#include "Camera.h"
//#include "XYStage.h"
//#include "ZStage.h"
#include "AcquisitionChannel.h"
#include "Channel.h"
#include "Magnification.h"
#include "focus.h"
#ifdef AXIOVERT
#include "Axiovert.h"
#endif
#ifdef OBSERVER
#include "Observer.h"
#endif
#include "Lambda10_3.h"
#include "Galvo.h"
#include "NIDAQ.h"
#include "DG.h"
#include "Laser.h"
#include "Pump.h"
#include "TEModule.h"
#include <vector>
#include <limits>
#include "PowerMeter.h"


class ZStage;
class XYStage;
class AcquisitionParameters;
class Objective;
class Optovar;
class Selection;
class Andor885;
class Changer;
class Ludl;
class FilterWheel;
struct customProtocol{
	double moveDelay;
	int chamber;
	Chamber cham;
	FluidicsSetup fs;
	vector<double> loadingVolume;//how much volume to get to the middle of the chamber
	double tubingDiameterInches;//.0625"?
	double totalReagentFlow;//in mm of the channel, how much flow over the FOV before stopping the pump
	double imagingTime;//total time in seconds for kintics imaging
	double startCushion;//imaging time prior to reagent coming into the FOV
	double DFDelay;//maximum time given to DF before we abort the kinetics experiment.....this will determine how much wash solution we pull before the air gap reaches the FOV
	double DFInitTimeout;//maximum time  give to DF initialization
	double airGapmmChannel;//length of air gap in channel (could be zero but mixing will occur)
	double mmpsChannelSpeed;//during experiment how fast do we pull solution?
	double loadTimeSec;//during reagent loading how long to get to the beginning of the channel (e.g. 30 sec)..this will affect the amount of mixing, faster speed means more mixing and dilution
	double washNumChannelVolumes; //dilution ratio.  100 means wash total volume of cham.uLchannelVolume*100. 
	double washSpeedmmpsChannel; //speed during wash
	double loadNumChannelVolumes;//determines how much excess reagent volume we have so we can center it on the channel (e.g. 2x would be 2*channelVolume)

	int channelNum;//which channel for the chamber
	double fractionLength;//position along channel to start experiment (width will be 0.5....note liveView will begin here and experiment will be performed the desired FOV spacing away from that)
	int channel;
	int section;
	int direction;
	double FOVfocus;
	double FOVscan;
	Focus coarse;
	Focus initial;
	Focus final;
	std::vector<std::vector<AcquisitionChannel>> FOVChannels;
	bool focusBleachCompensation;
 };

// standard exceptions
#include <iostream>
#include <exception>
using namespace std;

class abortException: public exception
{
public:

	abortException(string msg):msg(msg){}
  virtual const char* what() const throw()
  {
	  return msg.c_str();
  }
private:
  	string msg;
};

class Controller{
public:
	Controller();
	~Controller();

	void stop();
	void abort();

		//state variables
	std::string workingDir;
	int currentChan;
	int currentOut;
	std::vector<std::vector<AcquisitionChannel*>> currentChannels; 
	Focus currentFocus;
	AcquisitionChannel& currentChannel();
	void setCurrentChannel(AcquisitionChannel& acq);
	void setCurrentChannel(Channel* chan);
	void setCurrentChannel(OutPort* out);
	void addDefaultAC(AcquisitionChannel& acq,int &chan,int &out);
	AcquisitionChannel* getAcquisitionChannel(Channel* chan, OutPort* out);
	//list of supported configurations for each setup.  Given as index into vectors 
#ifdef AXIOVERT
	Axiovert axio;
	AxiovertRL rl;
	AxiovertHalogen hal;
	AxiovertReflectorTurret reflector;
	AxiovertOutPort outPort;
	AxiovertObjTurret objTurret;
	AxiovertOptTurret optTurret;
	NIDAQ daqusb;
	Lambda10_3 ld;
	FilterWheel fw;
	DG dg5;

	static const int BF=0;
	static const int UV=-1;
	static const int DAPI=1;
	static const int FITC=2;
	static const int CY3=3;
	static const int CY35=3;
	static const int CY5=3;
	static const int CY55=4;

	static const int OUT_CAM=0;			//The sequence of which outPorts were push_back().
	static const int OUT_OCULARS=1;
	static const int OUT_CAM_OCULARS=2;
	static const int OUT_BLOCKED=2;//oculars is better than camera

	static const int MAG_10x=0;			//The sequence of which Magnifications were push_back().
	static const int MAG_16x=1;
	static const int MAG_20x=2;
	static const int MAG_32x=3;
	static const int MAG_40x=4;
	static const int MAG_64x=5;
	static const int MAG_40xOil=6;
	static const int MAG_64xOil=7;
	static const int MAG_63xOil=8;
	static const int MAG_100xOil=9;	
#endif
#ifdef OBSERVER
	NIDAQ daqpci;
	Observer axio;
	ObserverHalogen hal;
	ObserverRL rl;
	ObserverTL tl;
	ObserverReflectorTurret reflector;
	ObserverOutPort outPort;
	ObserverObjTurret objTurret;
	ObserverOptTurret optTurret;
	ObserverDefiniteFocus df;
	ObserverTIRFSlider tirf;
	DG dg5;
	Laser violet;
	Laser blue;
	//Galvo g473;//needed for laser power modulation
	Galvo g532;//needed for laser power modulation
	Laser green;	
	Laser red;
	Laser red660;
	
	
	
//	Lambda10_3 ld;
//	FilterWheel fwA;
	

	//initialize these to -1 and let them be defined by the active configuration
	int BF;
	int UV;
	int DAPI;
	int DAPILASER;
	int FITC;
	int CY3;
	int CY35;
	int CY5;
	int CY55;
	int QDOT705;
	int CY7;
	int FRET405488;
	int FRET425594;
	int FRETFITCCy35;
	int FRETCy3Cy5Sem;int FRETCy3Cy5;
	int FRETCy3Cy5Thor;
	int TIRF405;
	int TIRF488;
	int TIRF532;
	int TIRF532_EX;
	int TIRF642;
	int TIRF642_EX;
	int TIRF660;
	int EPI405;
	int EPI473;
	int EPI532;
	int EPI642;
	int EPI660;
	int BFPINKEL;
	int DAPIPINKEL;
	int FITCPINKEL;
	int CY3PINKEL;
	int CY5PINKEL;
	
	bool noChannels;

	int OUT_CAM;
	int OUT_CAM_LEFT;
	int OUT_CAM_RIGHT;
	int OUT_CAM_RIGHTTRANS;
	int OUT_CAM_RIGHTREFL;
	int OUT_CAM_DUAL;
	int OUT_OCULARS;
	int OUT_BLOCKED;//baseport
	int OUT_FRONT;
	int OUT_CAM_OCULAR;//not supported

	int MAG_10x;			//The sequence of which Magnifications were push_back().
	int MAG_20x;
	int MAG_40x;
	int MAG_40xOil;
	int MAG_63xOil;
	int MAG_100xOil;
	
#endif

	//virtual objects
	ZStage *focus;
	XYStage *stg;
	


	std::vector<Camera*> cameras;//vector of all available cameras
	std::vector<OutPort> outPorts;//vector of all available output Ports
	std::vector<Channel> channels;//vector of all available channels
	std::vector<Magnification> mags;//vector of all magnifications
	std::vector<LightSource*> lightSources;//vector of all lightSources so we can iterate through them
	//other objects
	Pump pmp;
	TEModule te;
	
	//lists of active processes
	DisplayList displays;
	ThreadList threads;

	//utility functions
//	void takePicture(AcquisitionChannel* ac,std::string fileName, Focus* f,bool removeReagentTemporarily=false, bool wait4Pump=true,bool scaleBar=true);
	void UVExpose(double seconds);
	void setWorkingDir(std::string d);
	static unsigned __stdcall mainThread(void* param);

		//gui functions
	void modifyWorkingDir();

	//Modular Controls: Using individual parts.
	void DAQControl();
	//void pumpControl();moved to pump class
	void TEModuleControl();
	void liveView();
	void snapMode();
	PowerMeter pm;
	
};
