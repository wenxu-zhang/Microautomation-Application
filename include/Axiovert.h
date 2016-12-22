// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
//#include "Controller.h"
#include "Definitions.h"
#ifdef AXIOVERT
#include <cstdlib>
#include <string>
#include "ScopeComm.h"
#include "Timer.h"
#include "Changer.h"
#include "Objective.h"
#include "Selection.h"
#include "LightSource.h"
#include "ZStage.h"
#include "Record.h"
#include <atlbase.h>
#include <atlcom.h>
#include "CMTBCOMContinualEventSink.h"
#import "MTBApi.tlb" named_guids //raw_interfaces_only
using namespace MTBApi;
#import "mscorlib.tlb"
#define AXIOVERTDEFAULTHALOGENVOLTAGE 1.2
#define AXIOVERTMINHALOGENVOLTAGE 0.9
#define AXIOVERTMAXHALOGENVOLTAGE 12.0

#define StartErrChk try{
#define EndErrChk } catch(_com_error e){Axiovert::DisplayError(&e);}
extern Record logFile;
class AcquisitionGroup;

//START CODE FOR EVENT SINK
class ATL_NO_VTABLE CMTBCOMBaseEventSink : 
  public IDispEventImpl</*nID*/ 1, CMTBCOMBaseEventSink, &MTBApi::DIID_IMTBBaseEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE busyChanged;
	HANDLE lockChanged;
	std::string name;
public : 
	CMTBCOMBaseEventSink(std::string name="UNKNOWN"):name(name){
		busyChanged=CreateEvent(NULL,false,false,NULL);
	}
	void waitBusy(){
		WaitForSingleObject(busyChanged,INFINITE);
	}
	void waitLock(){
		WaitForSingleObject(lockChanged,INFINITE);
	}
public : 
	// Event handler for event: MTBErrorEvent 
	void __stdcall OnMTBError(long code, BSTR msg)
	{ 
		std::cout<<"MTBError in "<<name<<":"<<code<<msg<<std::endl;
	}
	void __stdcall OnMTBLockingChanged(bool started){
		if (started) return;
		SetEvent(lockChanged);
	}
	void __stdcall OnMTBBusyChanged(bool busy){
		if (busy) return;
		SetEvent(busyChanged);
	}
BEGIN_SINK_MAP(CMTBCOMBaseEventSink) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBBaseEvents, /*dispid =*/ 0x1, OnMTBError)  
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBBaseEvents, /*dispid =*/ 0x2, OnMTBLockingChanged)  
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBBaseEvents, /*dispid =*/ 0x4, OnMTBBusyChanged)  
END_SINK_MAP()   
}; 
class ATL_NO_VTABLE CMTBCOMChangerEventSink : 
  public IDispEventImpl</*nID*/ 1, CMTBCOMChangerEventSink, &MTBApi::DIID_IMTBChangerEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE positionSettled;
public : 
	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBPositionChanged(short newPosition) 
	{ 
		std::cout<<"Position changed"<<std::endl;
	} 
	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBPositionSettled(short newPosition) 
	{ 
		SetEvent(positionSettled);
	} 
	void wait(){
		WaitForSingleObject(positionSettled,INFINITE);
	}
BEGIN_SINK_MAP(CMTBCOMChangerEventSink) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBChangerEvents, /*dispid =*/ 0x1, OnMTBPositionChanged) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBChangerEvents, /*dispid =*/ 0x3, OnMTBPositionSettled) 
END_SINK_MAP()   
}; 

class ATL_NO_VTABLE CMTBCOMMoveEventSink : 
  public IDispEventImpl</*nID*/ 1, CMTBCOMMoveEventSink, &MTBApi::DIID_IMTBMoveEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE speedSettled;
public : 
	CMTBCOMMoveEventSink(){
		speedSettled=CreateEvent(NULL,false,false,NULL);
	}
	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBMoveSpeedSettled() 
	{ 
		SetEvent(speedSettled);
		logFile.write("Speed settled",true,"Axiovert.h");
	} 
	void wait(){
		WaitForSingleObject(speedSettled,INFINITE);
	}
BEGIN_SINK_MAP(CMTBCOMMoveEventSink) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBMoveEvents, /*dispid =*/ 0x1, OnMTBMoveSpeedSettled) 	
END_SINK_MAP()   
}; 
//END CODE FOR EVENT SINK

class Axiovert{

public:

	MTBApi::IMTBConnectionPtr m_conn;
	ATL::CComBSTR m_ID;

	MTBApi::IMTBChangerPtr reflectorTurret;//MTBReflectorChanger
	MTBApi::IMTBChangerPtr objectiveTurret;//MTBObjectiveChanger
	MTBApi::IMTBChangerPtr optovarTurret;//MTBOptovarChanger
	MTBApi::IMTBLampPtr halogen;//MTBTLHalogenLamp
	MTBApi::IMTBChangerPtr sideport;//MTBSideportChanger
	MTBApi::IMTBShutterPtr RLshutter;//MTBRLShutter
	MTBApi::IMTBAxisPtr focus;//MTBFocus
	
	MTBApi::IMTBConfigurationPtr config;//provides access to active configuration information

	//MTB event sinks one for client side, one for MTB server side
	CMTBCOMBaseEventSink focusBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMfocusBaseEvents;
	CMTBCOMContinualEventSink focusEvents;
	MTBApi::IMTBEventSinkPtr m_COMfocusEvents;
	CMTBCOMMoveEventSink focusSpeedEvents;
	MTBApi::IMTBEventSinkPtr m_COMfocusSpeedEvents;
	CMTBCOMBaseEventSink halogenBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMhalogenBaseEvents;
	CMTBCOMContinualEventSink halogenEvents;
	MTBApi::IMTBEventSinkPtr m_COMhalogenEvents;
	CMTBCOMBaseEventSink reflectorBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMreflectorBaseEvents;
	CMTBCOMChangerEventSink reflectorEvents;
	MTBApi::IMTBEventSinkPtr m_COMreflectorEvents;
	CMTBCOMBaseEventSink objectiveBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMobjectiveBaseEvents;
	CMTBCOMChangerEventSink objectiveEvents;
	MTBApi::IMTBEventSinkPtr m_COMobjectiveEvents;
	CMTBCOMChangerEventSink optovarEvents;
	MTBApi::IMTBEventSinkPtr m_COMoptovarEvents;
	CMTBCOMBaseEventSink optovarBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMoptovarBaseEvents;
	CMTBCOMChangerEventSink sideportEvents;
	MTBApi::IMTBEventSinkPtr m_COMsideportEvents;
	CMTBCOMBaseEventSink sideportBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMsideportBaseEvents;
	CMTBCOMChangerEventSink RLshutterEvents;
	MTBApi::IMTBEventSinkPtr m_COMRLshutterEvents;
	CMTBCOMBaseEventSink RLshutterBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMRLshutterBaseEvents;

	std::string getConfig();//get name of the active configuration used for setting up objectives and channels
	std::vector<Objective> getObjectives();//return available objectives for active configuration
	std::vector<Selection> getCubes();//return available reflector cubes for active configuration

	//true if  "Other Lamp" is selected in MTB2004 Configuration under Reflected Light
	//false if "Empty Lamp Port" is selected in MTB2004 Configuration under Reflected Light
	//true means we are using the XCite, false mean we are using DG5 (if present) or no RL lamp (e.g. lasers only)
	bool isManualLamp();//uses isManLamp member
	bool isManLamp;//set in contructor
	int objTurretCodedPosition;//-1 if the objective turret is not coded (e.g. motorized and can rotate)
							   //position it is in if it is coded (Definite Focus with Ludl Stage)
	
	//copied from Observer for MTB interface
	double moveSpeedSettledPosition;
	static void DisplayError(_com_error* e);
	void waitAxiovert(MTBApi::IMTBBasePtr& a);
	//the above was copied from Observer for MTB interface

	//Old Axiovert member variables and functions.
	double getWorkingDistance();
	void setMaxZ(double z);
	double getMaxZ();
	//double writeAndGetReturnedPosition(string cmd);
	//void waitCalibration();
	//double maxZ;
	static const double softLimitZ;
	/*
	static const int TURRET_PINKEL= 1;
	static const int TURRET_FITC= 2;
	static const int TURRET_CY5= 4;
	static const int TURRET_CY7= 4;//not present...no need for it
	static const int TURRET_CY3= 3;
	static const int TURRET_BF= 1;//not present and not needed...using pinkel instead
	static const int TURRET_UV=5;

	static const int OBJ_10= 1;
	static const int OBJ_20= 2;
	static const int OBJ_40= 3;
	static const int OBJ_63= 5;

	static const int SIDEPORT5050 =   3;
	static const int SIDEPORTCAM  =   2;
	static const int SIDEPORTOCULAR = 1;
	*/
	void enableLampController();
	int getObj();
	int getTotalMag();

	//START PUBLIC HALOGEN LAMP CONTROL
	void HalogenOff(bool wait=true);
	void HalogenOn(double volts=-1, bool wait=true);
	double voltage;
	Timer voltageTimer;
	void waitHalogen();
	//END HALOGEN LAMP CONTROL

	double getDOF();
	ScopeComm* scopecom;
	bool isPresent;
	Axiovert();//initialize dll function loadfocdefparam, turn focus power on, initialize focus by setting zero, set load work to 1 (working mode)
	~Axiovert();//clean up?? turn focus off
	void move(double zpos); //move to zpos in micrometers, haven't implemented fine focus yet, initially it will be rough movements
	//void inc(double um);//increase zpos by um micrometere, um could be negative 
	double getPos();//return current zpos, this can be used to check that the focus has arrived at its current destination
	bool go(double zspeed);
	bool debug;
	void stop();
	//void wait();//return when the objective has finished moving to its correct z position
	void SetObj(int i);//set turret position
	void SetOptovar(int opt);
	bool getOptovar();
	void SetTurret(int i);//reflector turret that is
	int getTurret();//reflector turret that is
	void SetLimits(double min,double max);
	
	void SidePort(int pos);//
	int getSidePort();
	void openShutter();
	void closeShutter();
	double getStartPosition();//get start position of constant velocity move
	std::string vel;
	std::string acc;
	
	void unlockDisplay();
	
private:
	//start private halogen voltage control
	//void adjustVoltage();
	//int GetHalogenVoltageBinary();
	//void SetHalogenVoltage(double volts);
	double GetHalogenVoltage();
	//void SetHalogenVoltageBinary(int binary);
	//int sentHalogenBinary;//from 0 to 255
	//int desiredHalogenBinary;
	//bool changed;
	//int volts2Int(double volts);
	//double int2Volts(int binary);
	//bool halogenOn;
	::Timer halogenTimer;
	//end private halogen voltage control

	double zpos;//current position in micrometers
};

class AxiovertObjTurret:public Changer{
public:
	Axiovert* a;
	AxiovertObjTurret(Axiovert* a):a(a){}
	void set(double pos){a->SetObj(pos);}
	double get(){return a->getObj();}
	void wait(){
		a->waitAxiovert((MTBApi::IMTBBasePtr)a->objectiveTurret);
	}
};

class AxiovertOptTurret:public Changer{
public:
	Axiovert* a;
	AxiovertOptTurret(Axiovert* a):a(a){}
	void set(double pos){a->SetOptovar(pos);}
	double get(){return a->getOptovar();}
	void wait(){
		a->waitAxiovert((MTBApi::IMTBBasePtr) a->optovarTurret);
	}
};

class AxiovertReflectorTurret:public Changer{
public:
	Axiovert* a;
	AxiovertReflectorTurret(Axiovert* a):a(a){}
	void set(double pos){a->SetTurret(pos);}
	double get(){return a->getTurret();}
	void wait(){
		a->waitAxiovert((MTBApi::IMTBBasePtr)a->reflectorTurret);
	}
};

class AxiovertOutPort:public Changer{
public:
	Axiovert* a;
	AxiovertOutPort(Axiovert* a):a(a){}
	void set(double pos){a->SidePort(pos);}
	double get(){return a->getSidePort();}
	void wait(){
		a->waitAxiovert((MTBApi::IMTBBasePtr)a->sideport);
	}
};

class AxiovertRL:public LightSource{
public:
	Axiovert* a;
	AxiovertRL(Axiovert* a):a(a){}
	void on(int pos,double intensity){
		a->openShutter();
	}
	void off(){
		a->closeShutter();
	}
	void wait(){
		StartErrChk
			a->waitAxiovert((MTBApi::IMTBBasePtr)a->RLshutter);
		EndErrChk
	}
	void enterRingBuffer(AcquisitionGroup ag){logFile.write("Error: RL Shutter cannot be triggered",true);}//not supported
	void exitRingBuffer(){return;}//not supported
	std::string intensityToString(double intensity){return "100%";}
	double getIntensity(double intensity=-1,std::string intensityUnit=""){return 1;}
	//bool isValidIntensity(double intensity){return true;}
};
class AxiovertHalogen:public LightSource{
public:
	Axiovert* a;
	AxiovertHalogen(Axiovert* a, Trigger* t=0, int triggerLine=0,double defaultIntensity=1.5):LightSource(t,triggerLine,defaultIntensity),a(a){}
	void on(int pos,double intensity){
		a->HalogenOn(intensity,true);}
	void off(){a->HalogenOff(true);}
	void wait(){
		StartErrChk			
		a->waitHalogen();
		EndErrChk
	}
	void enterRingBuffer(AcquisitionGroup ag){return;}//not supported
	void exitRingBuffer(){return;}//not supported
	std::string intensityToString(double intensity){
		return toString(intensity)+"V";
	}
	/*bool isValidIntensity(double intensity){
		if (intensity>=AXIOVERTMINHALOGENVOLTAGE && intensity<=AXIOVERTMAXHALOGENVOLTAGE) 
			return true; 
		else return false;
	}*/
	double getIntensity(double intensity=-1, std::string intensityUnit=""){
		if (intensity==-1)
			return AXIOVERTDEFAULTHALOGENVOLTAGE;
		if (intensityUnit=="")
			return getIntensity(intensity,"V");
		if (intensityUnit=="V"){
			if (intensity<AXIOVERTMINHALOGENVOLTAGE){
				logFile.write("Error: (Axiovert Halogen) Invalid intensity "+toString(intensity) +" must be >= "+toString(AXIOVERTMINHALOGENVOLTAGE)+"V",true);
				return AXIOVERTMINHALOGENVOLTAGE;
			}else if (intensity>AXIOVERTMAXHALOGENVOLTAGE){ 
				logFile.write("Error: (Axiovert Halogen) Invalid intensity "+toString(intensity) +" must be <= "+toString(AXIOVERTMAXHALOGENVOLTAGE)+"V",true);
				return AXIOVERTDEFAULTHALOGENVOLTAGE;
			}
			return intensity;
		}
		if (intensityUnit=="f"){
			if (intensity*AXIOVERTMAXHALOGENVOLTAGE<AXIOVERTMINHALOGENVOLTAGE){
				logFile.write("Error: (Axiovert Halogen) Invalid fractional intensity "+toString(intensity) +" must be >= "+toString(AXIOVERTMINHALOGENVOLTAGE/AXIOVERTMAXHALOGENVOLTAGE),true);
				return AXIOVERTMINHALOGENVOLTAGE;
			}else if (intensity>1){
				logFile.write("Error: (Axiovert Halogen) Invalid fractional intensity "+toString(intensity) +" must be <= 1",true);
				return AXIOVERTMAXHALOGENVOLTAGE;
			}
		}
		if (intensityUnit=="%"){
			if (intensity*AXIOVERTMAXHALOGENVOLTAGE/100.0<AXIOVERTMINHALOGENVOLTAGE){
				logFile.write("Error: (Axiovert Halogen) Invalid intensity "+toString(intensity) +" must be >= "+toString(100.0*AXIOVERTMINHALOGENVOLTAGE/AXIOVERTMAXHALOGENVOLTAGE)+"%",true);
				return AXIOVERTMINHALOGENVOLTAGE;
			}else if (intensity>100){
				logFile.write("Error: (Axiovert Halogen) Invalid fractional intensity "+toString(intensity) +" must be <= 100%",true);
				return AXIOVERTMAXHALOGENVOLTAGE;
			}
		}
		logFile.write("Error:  (Axiovert Halogen) Unsupported intensity unit "+intensityUnit,true);
		return AXIOVERTMINHALOGENVOLTAGE;
	}
};

class AxiovertZ:public ZStage{
public:
	Axiovert* a;
	AxiovertZ(Axiovert* a):a(a){}
	void move(double z){a->move(z);}
	bool velocityMove(double umpsec){return a->go(umpsec);}
	double getVelocityMoveStart(){return a->getStartPosition();}
	double getZ(){return a->getPos();}
	double getMaxZ(){return a->getMaxZ();}
	void setMaxZ(double z){a->setMaxZ(z);}
	void stop(){a->stop();}
	void wait(){
		a->waitAxiovert((MTBApi::IMTBBasePtr)a->focus);
	}
};
#endif