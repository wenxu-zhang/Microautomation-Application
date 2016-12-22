// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, May 25, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#ifdef OBSERVER
#include "Changer.h"
#include "Objective.h"
#include "Selection.h"
#include "LightSource.h"
#include "ZStage.h"
#include <atlbase.h>
#include <atlcom.h>
#include "CMTBCOMContinualEventSink.h"
#import "MTBApi.tlb" named_guids //raw_interfaces_only
using namespace MTBApi;
#import "mscorlib.tlb"
#include "Record.h"

class AcquisitionGroup;
#define OBSERVERDEFAULTHALOGENVOLTAGE 1.2
#define OBSERVERMAXHALOGENVOLTAGE 12
#define OBSERVERMINHALOGENVOLTAGE 0.9
#define OBSERVERCONDENSERNA 0.5

#define StartErrChk try{
#define EndErrChk } catch(_com_error e){Observer::DisplayError(&e);}
extern Record logFile;


/*
#define WAIT(device) {\
if (!a->isPresent) \
	return; \
else {\
	Timer t(true);\
	bool b;\
	b=((MTBApi::IMTBBasePtr) a->device)->WaitReady(10000);\
	if (!b) std::cout<<"Error: Wait timed out"<<std::endl;\
}}
*/
/*std::cout<<"Wait for "<<#device<<" took "<<t.getTime()<<"ms"<<std::endl;\
	*/
//START CODE FOR EVENT SINK
class ATL_NO_VTABLE CMTBCOMBaseEventSink : 
   //public CComObjectRootEx<CComSingleThreadModel>, 
   //public CComCoClass<CMTBCOMContinualEventSink, &CLSID_MTBCOMContinualEventSink>, 
  // public IDispatchImpl<IMTBCOMEventSink, &IID_IMTBCOMEventSink, &LIBID_MTBClientUsingCOMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, 
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
	/*
BEGIN_COM_MAP(CMTBCOMContinualEventSink) 
   COM_INTERFACE_ENTRY(IMTBCOMEventSink) 
   COM_INTERFACE_ENTRY(IDispatch) 
END_COM_MAP() 
DECLARE_PROTECT_FINAL_CONSTRUCT() 
HRESULT FinalConstruct() { 
	return S_OK; 
} 
void FinalRelease() 
   { 
   } 
*/
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
//CMTBCOMContinualEventSink
class ATL_NO_VTABLE CMTBCOMChangerEventSink : 
   //public CComObjectRootEx<CComSingleThreadModel>, 
   //public CComCoClass<CMTBCOMContinualEventSink, &CLSID_MTBCOMContinualEventSink>, 
  // public IDispatchImpl<IMTBCOMEventSink, &IID_IMTBCOMEventSink, &LIBID_MTBClientUsingCOMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, 
  public IDispEventImpl</*nID*/ 1, CMTBCOMChangerEventSink, &MTBApi::DIID_IMTBChangerEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE positionSettled;
public : 
	/*
BEGIN_COM_MAP(CMTBCOMContinualEventSink) 
   COM_INTERFACE_ENTRY(IMTBCOMEventSink) 
   COM_INTERFACE_ENTRY(IDispatch) 
END_COM_MAP() 
DECLARE_PROTECT_FINAL_CONSTRUCT() 
HRESULT FinalConstruct() { 
	return S_OK; 
} 
void FinalRelease() 
   { 
   } 
*/
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
   //public CComObjectRootEx<CComSingleThreadModel>, 
   //public CComCoClass<CMTBCOMContinualEventSink, &CLSID_MTBCOMContinualEventSink>, 
  // public IDispatchImpl<IMTBCOMEventSink, &IID_IMTBCOMEventSink, &LIBID_MTBClientUsingCOMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, 
  public IDispEventImpl</*nID*/ 1, CMTBCOMMoveEventSink, &MTBApi::DIID_IMTBMoveEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE speedSettled;
public : 
	/*
BEGIN_COM_MAP(CMTBCOMContinualEventSink) 
   COM_INTERFACE_ENTRY(IMTBCOMEventSink) 
   COM_INTERFACE_ENTRY(IDispatch) 
END_COM_MAP() 
DECLARE_PROTECT_FINAL_CONSTRUCT() 
HRESULT FinalConstruct() { 
	return S_OK; 
} 
void FinalRelease() 
   { 
   } 
*/
public : 
	CMTBCOMMoveEventSink(){
		speedSettled=CreateEvent(NULL,false,false,NULL);
	}
	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBMoveSpeedSettled() 
	{ 
		SetEvent(speedSettled);
	} 
	void wait(){
		WaitForSingleObject(speedSettled,INFINITE);
	}
BEGIN_SINK_MAP(CMTBCOMMoveEventSink) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBMoveEvents, /*dispid =*/ 0x1, OnMTBMoveSpeedSettled) 	
END_SINK_MAP()   
};
//END CODE FOR EVENT SINK

class Observer{

public:
		// MTB interface pointer to the connection class
	MTBApi::IMTBConnectionPtr m_conn;
	ATL::CComBSTR m_ID;

	MTBApi::IMTBChangerPtr reflectorTurret;//MTBReflectorChanger
	MTBApi::IMTBChangerPtr objectiveTurret;//MTBObjectiveChanger
	MTBApi::IMTBChangerPtr optovarTurret;//MTBOptovarChanger
	MTBApi::IMTBLampPtr halogen;//MTBTLHalogenLamp
	MTBApi::IMTBChangerPtr baseport;//MTBBaseportChanger
	MTBApi::IMTBChangerPtr sideport;//MTBSideportChanger
	MTBApi::IMTBShutterPtr TLshutter;//MTBTLShutter
	MTBApi::IMTBShutterPtr RLshutter;//MTBRLShutter
	MTBApi::IMTBAxisPtr focus;//MTBFocus
	MTBApi::IMTBContinualPtr condenserAperture;//MTBTLApertureStop
	MTBApi::IMTBFocusStabilizerPtr definiteFocus;//MTBFocusStabilizer 

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
	CMTBCOMChangerEventSink baseportEvents;
	MTBApi::IMTBEventSinkPtr m_COMbaseportEvents;
	CMTBCOMBaseEventSink baseportBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMbaseportBaseEvents;
	CMTBCOMChangerEventSink sideportEvents;
	MTBApi::IMTBEventSinkPtr m_COMsideportEvents;
	CMTBCOMBaseEventSink sideportBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMsideportBaseEvents;
	CMTBCOMChangerEventSink TLshutterEvents;
	MTBApi::IMTBEventSinkPtr m_COMTLshutterEvents;
	CMTBCOMBaseEventSink TLshutterBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMTLshutterBaseEvents;
	CMTBCOMChangerEventSink RLshutterEvents;
	MTBApi::IMTBEventSinkPtr m_COMRLshutterEvents;
	CMTBCOMBaseEventSink RLshutterBaseEvents;
	MTBApi::IMTBEventSinkPtr m_COMRLshutterBaseEvents;

	std::string getConfig();//get name of the active configuration used for setting up objectives and channels
	
	std::vector<Objective*> objectives;//return available objectives for active configuration
	Objective* objPM;//used for power meter mounted objective position
	Objective* getCurrentObjective();

	int getObjectiveIndex(Objective* obj);//-1 for does not exist
	vector<int> pos2ind;//using objective position to get index of corresponding Objective in objectives vector
	std::vector<Selection> getCubes(int& currentCube);//return available reflector cubes for active configuration

	//true if  "Other Lamp" is selected in MTB2004 Configuration under Reflected Light
	//false if "Empty Lamp Port" is selected in MTB2004 Configuration under Reflected Light
	//true means we are using the XCite, false mean we are using DG5 (if present) or no RL lamp (e.g. lasers only)
	bool isManualLamp();//uses isManLamp member
	bool isManLamp;//set in contructor
	int objTurretCodedPosition;//-1 if the objective turret is not coded (e.g. motorized and can rotate)
							   //position it is in if it is coded (Definite Focus with Ludl Stage)
						  
	double moveSpeedSettledPosition;
	static void DisplayError(_com_error* e);
	void waitObserver(MTBApi::IMTBBasePtr& a);
	void setMaxZ(double z);
	double getMaxZ();
	static const double softLimitZ;
	int getObj();
	bool isPresent;
	Observer();//initialize dll function loadfocdefparam, turn focus power on, initialize focus by setting zero, set load work to 1 (working mode)
	~Observer();//clean up?? turn focus off
	void move(double zpos); //move to zpos in micrometers, haven't implemented fine focus yet, initially it will be rough movements
	double getPos();//return current zpos, this can be used to check that the focus has arrived at its current destination
	bool go(double zspeed);
	void stop();
	void SetObj(int i);//set turret position
	void SetOptovar(int opt);
	int getOptovar();
	void SetTurret(int i);//reflector turret that is
	int getTurret();//reflector turret that is
	void SetLimits(double min,double max);
	void SetHalogenVoltage(double volts);
	double GetHalogenVoltage();
	void HalogenOff(bool wait=true);
	void HalogenOn(double volts=-1,bool wait=true);
	void SidePort(int pos);//
	int getSidePort();
	void openTLShutter();
	void closeTLShutter();
	void openShutter();//RL reflected light
	void closeShutter();//RL reflected light
	double getStartPosition();//get start position of constant velocity move
	bool basePortPresent;
	bool definiteFocusPresent;
	bool getExclusive();
	void setExclusive(bool isOn);

private:

};

class ObserverObjTurret:public Changer{
public:
	Observer* a;
	ObserverObjTurret(Observer* a):a(a){}
	void set(double pos){a->SetObj(pos);}
	double get(){return a->getObj();}
	void wait(){
		a->waitObserver((MTBApi::IMTBBasePtr)a->objectiveTurret);
	}
};

class ObserverOptTurret:public Changer{
public:
	Observer* a;
	ObserverOptTurret(Observer* a):a(a){}
	void set(double pos){a->SetOptovar(pos);}
	double get(){return a->getOptovar();}
	void wait(){
		a->waitObserver((MTBApi::IMTBBasePtr) a->optovarTurret);
	}
};

class ObserverReflectorTurret:public Changer{
public:
	Observer* a;
	ObserverReflectorTurret(Observer* a):a(a){}
	void set(double pos){a->SetTurret(pos);}
	double get(){return a->getTurret();}
	void wait(){
		StartErrChk
		a->waitObserver((MTBApi::IMTBBasePtr)a->reflectorTurret);
		EndErrChk
	}
};

/*light path positions 
			1: Left (side 2)
			2: Right (side 3) 
			3:Binoculars (side 1 base 2)
			4: Frontport (side 1 base 3)
			5:Baseport (side 1 base 1)
*/
class ObserverOutPort:public Changer{
public:
	Observer* a;
	ObserverOutPort(Observer* a):a(a){}
	void set(double pos){a->SidePort(pos);}
	double get(){return a->getSidePort();}
	void wait(){
		if (a->basePortPresent) a->waitObserver((MTBApi::IMTBBasePtr)a->baseport);
		a->waitObserver((MTBApi::IMTBBasePtr)a->sideport);
	}
};

class ObserverRL:public LightSource{
public:
	Observer* a;
	ObserverRL(Observer* a):a(a),LightSource(0,"reflected",0,1){}
	void on(int pos,double intensity){a->openShutter();}
	void off(){
		a->closeShutter();
	}
	void wait(){
		StartErrChk
			a->waitObserver((MTBApi::IMTBBasePtr)a->RLshutter);
		EndErrChk
	}
	void enterRingBuffer(AcquisitionGroup& ag){logFile.write("Error: RL Shutter cannot be triggered",true);}//not supported
	void exitRingBuffer(){return;}//not supported
	std::string intensityToString(double intensity){return "100%";}
	double getIntensity(double intensity=-1,std::string intensityUnit="",Objective* obj=NULL){return 1;}
	//bool isValidIntensity(double intensity){return true;}
};

//only needed if not using the halogen illumination from the top (e.g. UV illumination from the top controlled by TL shutter).
class ObserverTL:public LightSource{
public:
	Observer* a;
	ObserverTL(Observer* a):a(a){}
	void on(int pos,double intensity){a->openTLShutter();}
	void off(){
		a->closeTLShutter();
	}
	void wait(){
		StartErrChk
			a->waitObserver((MTBApi::IMTBBasePtr)a->TLshutter);
		EndErrChk
	}
	void enterRingBuffer(AcquisitionGroup& ag){logFile.write("Error: TL Shutter cannot be triggered",true);}//not supported
	void exitRingBuffer(){return;}//not supported
	std::string intensityToString(double intensity,Objective* obj=NULL){return "100%";}
	double getIntensity(double intensity=-1,std::string intensityUnit="",Objective* obj=NULL){return 1;}
	//bool isValidIntensity(double intensity){return true;}
};

class ObserverHalogen:public LightSource{
public:
	Observer* a;
	ObserverHalogen(Observer* a):a(a),LightSource(0,"Halogen",0,OBSERVERDEFAULTHALOGENVOLTAGE){}
	void on(int pos,double intensity){
		if (intensity==cp->zeroIntensity)
			off();
		else
			a->HalogenOn(intensity,false);
	}//a->closeShutter();}
	void off(){a->HalogenOff(false);}
	void wait(){
		StartErrChk			
		a->waitObserver((MTBApi::IMTBBasePtr)a->TLshutter);
		a->waitObserver((MTBApi::IMTBBasePtr)a->halogen);
		EndErrChk
	}
	void enterRingBuffer(AcquisitionGroup& ag){logFile.write("Error: Halogen cannot be triggered",true);}//not supported
	void exitRingBuffer(){return;}//not supported
	std::string intensityToString(double intensity,Objective* obj=NULL){
		return toString(intensity)+"V";
	}

	double getPower(double intensity, string& units,Objective* obj=NULL){
		units="V";
		return intensity;
	}
	//default intensity unit is volts
	double getIntensity(double intensity=-1, std::string intensityUnit="",Objective* obj=NULL){
		if (intensity==-1)
			return ncp->defaultIntensity;
		if (intensityUnit=="")
			return getIntensity(intensity,"V");
		if (intensityUnit=="V"){
			if (intensity<OBSERVERMINHALOGENVOLTAGE){
				logFile.write("Error: (Observer Halogen) Invalid intensity "+toString(intensity) +" must be >= "+toString(OBSERVERMINHALOGENVOLTAGE)+"V",true);
				return OBSERVERMINHALOGENVOLTAGE;
			}else if (intensity>OBSERVERMAXHALOGENVOLTAGE){ 
				logFile.write("Error: (Observer Halogen) Invalid intensity "+toString(intensity) +" must be <= "+toString(OBSERVERMAXHALOGENVOLTAGE)+"V",true);
				return ncp->defaultIntensity;
			}
			return intensity;
		}
		if (intensityUnit=="f"){
			if (intensity*OBSERVERMAXHALOGENVOLTAGE<OBSERVERMINHALOGENVOLTAGE){
				logFile.write("Error: (Observer Halogen) Invalid fractional intensity "+toString(intensity) +" must be >= "+toString(OBSERVERMINHALOGENVOLTAGE/OBSERVERMAXHALOGENVOLTAGE),true);
				return OBSERVERMINHALOGENVOLTAGE;
			}else if (intensity>1){
				logFile.write("Error: (Observer Halogen) Invalid fractional intensity "+toString(intensity) +" must be <= 1",true);
				return OBSERVERMAXHALOGENVOLTAGE;
			}
		}
		if (intensityUnit=="%"){
			if (intensity*OBSERVERMAXHALOGENVOLTAGE/100.0<OBSERVERMINHALOGENVOLTAGE){
				logFile.write("Error: (Observer Halogen) Invalid intensity "+toString(intensity) +" must be >= "+toString(100.0*OBSERVERMINHALOGENVOLTAGE/OBSERVERMAXHALOGENVOLTAGE)+"%",true);
				return OBSERVERMINHALOGENVOLTAGE;
			}else if (intensity>100){
				logFile.write("Error: (Observer Halogen) Invalid intensity "+toString(intensity) +" must be <= 100%",true);
				return OBSERVERMAXHALOGENVOLTAGE;
			}
		}
		logFile.write("Error:  (Observer Halogen) Unsupported intensity unit "+intensityUnit,true);
		return OBSERVERMINHALOGENVOLTAGE;
	}

	/*bool isValidIntensity(double intensity){
		if (intensity>=OBSERVERMINVOLTAGE && intensity<=OBSERVERMAXVOLTAGE) 
			return true; 
		else return false;
	}
	*/
};

class ObserverZ:public ZStage{
public:
	Observer* a;
	ObserverZ(Observer* a):a(a){}
	void move(double z){a->move(z);}//backlash compensation is built into the MTB2004 function call
	bool velocityMove(double umpsec){return a->go(umpsec);}
	double getVelocityMoveStart(){return a->getStartPosition();}
	double getZ(){return a->getPos();}
	void setMaxZ(double z){a->setMaxZ(z);}
	double getMaxZ(){return a->getMaxZ();}
	void stop(){a->stop();}
	void wait(){
		if (a->definiteFocusPresent && a->getExclusive())
			a->waitObserver((MTBApi::IMTBBasePtr)a->definiteFocus);
		else
			a->waitObserver((MTBApi::IMTBBasePtr)a->focus);
		
	}
	bool doesExist(){return a->isPresent;}
};

class ObserverDefiniteFocus{
	friend class DefiniteFocus;
private:
	Observer* a;
	std::vector<SAFEARRAY*> stabilizationData;
	bool internalInitializeDefiniteFocus(int index);
	bool internalGetDefiniteFocus(bool wait);
	bool internalStartDefiniteFocus(unsigned long periodSec);
	bool internalLoadInitializationData(int index);
	void internalRemoveInitializationData(int index);
protected:	
	int currentIndex;//index for current loaded stabilization data

public:
	ObserverDefiniteFocus(Observer* a);
	~ObserverDefiniteFocus(){
		if (!a->definiteFocusPresent)
			return;
		setExclusive(false);
		Timer::wait(200);
	}
	//for single stabilization point
	bool initializeDefiniteFocus();//use current position for initializing the definite focus.
	bool getDefiniteFocus(bool wait=true);//get focus once and wait if necessary, default index is for single point, value of -1 will use the current loaded index (not 0)
	bool startDefiniteFocus(unsigned long periodSec=0);//start periodic definite focusing with the given period, a value of zero will be as fast as possible
	void removeInitializationData();
	//////////////////////////////////
	
	//for multiple stabilization points
	bool initializeDefiniteFocus(int index);//use current position for initializing the definite focus. num is the index (>=1) in the StabilizerData array for this initialization.
	bool loadInitializationData(int index);
	bool getDefiniteFocus(int index,bool wait=true);
	bool startDefiniteFocus(int index,unsigned long periodSec=0);//start periodic definite focusing with the given period, a value of zero will be as fast as possible
	void removeInitializationData(int index);
	void clearIndexedInitializationData();
	////////////////////////////////
	
	void setExclusive(bool isOn){return a->setExclusive(isOn);}
	bool getExclusive(){return a->getExclusive();}
	void stopDefiniteFocus();//stop periodic definite focusing
	bool isDFPresent();
	bool isInitialized(int index=0);
	bool isStandardMode();
	void wait();
	
	void definiteFocusControl();
	bool isOn();
	int getOnOffChangedReason();
};

class ObserverTIRFSlider:public Changer{
public:
	Trigger* t;
	int analogLine;
	bool isPresent;
	double lastVoltage;
	Timer lastMove;
	ObserverTIRFSlider(Trigger* t, int analogLine):t(t),analogLine(analogLine){
		isPresent=true;
		lastMove.startTimer();
		if (!t->isValidAnalogOutLine(analogLine)){
			isPresent=false;
			return;
		}
		lastVoltage=-20;
		set(0);
	}
	void set(double pos){
		CheckExists();
		if (pos< -8){
			logFile.write("TIRF Angle Error: TIRF angle must be greater than -2. Are you trying to blind yourself?",true);
			pos=-2;
		}
		if (lastVoltage==pos)
			return;
		t->setVoltage(analogLine,pos);
		lastMove.restart();
		lastVoltage=pos;
	}
	double get(){
		return lastVoltage;
	}
	void wait(){
		lastMove.waitTotal(300);
	}

	Trigger* getTrigger(){return t;}
	int getTriggerLine(){return analogLine;}
};
#endif