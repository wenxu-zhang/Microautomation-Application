// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, May 25, 2011</lastedit>
// ===================================
#include "Warn.h"

#include <iostream>
#include <string>
#include "Observer.h"
#include "Controller.h"
#include <limits>
#include "Utils.h"
#include <atlstr.h>

#ifdef OBSERVER
#define DEBUGOBSERVER true
using namespace MTBApi;
using namespace mscorlib;

extern Record logFile;
extern Controller cont;
extern int protocolIndex;
extern vector<string> protocolFiles;

wchar_t micron[3]={181,'m',0};
wchar_t micronpersec[5]={181,'m','/','s',0};
wchar_t volt[5]={'V','o','l','t',0};
wchar_t percent[2]={'%',0};
wchar_t NA[3]={'N','A',0};
const double Observer::softLimitZ=8800;//definite focus
CComModule _Module;
#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
MIDL_DEFINE_GUID(CLSID, CLSID_MTBCOMContinualEventSink,0xece98723,0x4e0e,0x42eb,0x8a,0x82,0x0b,0x87,0x71,0xa2,0xc3,0xca);

void Observer::waitObserver(MTBApi::IMTBBasePtr& a){
	CheckExists()
	bool b;
	StartErrChk
	b=a->WaitReady(60000);
	EndErrChk
	if (!b) {
		std::cout<<"Error: Wait timed out"<<std::endl;
	}
}

#define CheckExistsDF(param) if (!isDFPresent()) return param;

#define WAIT(device) ((MTBApi::IMTBBasePtr) device)->WaitReady(60000)
string Observer::getConfig(){
	CheckExists("")
	string s(config->ReadActiveConfiguration());
	int begin=s.find_first_of("\"");
	int end=s.find_first_of("\"",begin+1);
	return s.substr(begin+1,end-begin-1);
}

vector<Selection> Observer::getCubes(int& currentCube){
	currentCube=0;
	vector<Selection> cubes;
	CheckExists(cubes)
	IMTBIdentPtr cube;
	for(long i=0;i<reflectorTurret->GetElementCount();i++){
		cube=(IMTBIdentPtr) reflectorTurret->GetElement(i);
		if (!cube)
			continue;
		cubes.push_back(Selection(&cont.reflector,i+1,string(cube->GetName())));
		logFile.write(string("Found Cube: ")+string(cube->GetName()),true);
		if (i==this->getTurret()-1) currentCube=i;
	}
	return cubes;
}

bool Observer::isManualLamp(){
	CheckExists(false)
	return isManLamp;
	
}

Observer::Observer():objPM(NULL){
	StartErrChk	
	basePortPresent=true;
	definiteFocusPresent=true;
	isManLamp=false;
	objTurretCodedPosition=-1;
	//MTB interface ptr to the root of the tree of components of the microscope
	IMTBRootPtr m_pRoot;
	isPresent=true;
	m_conn=NULL;
	m_pRoot=NULL;
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	::_Module.Init(NULL, GetModuleHandle(NULL));
	try{
	m_conn = IMTBConnectionPtr(CLSID_MTBConnection);
	if (!m_conn){
		logFile.write("Observer server not present.", true);
		isPresent=false;
		return;
	}
	m_conn->Login("en", &m_ID);

	logFile.write(string("Using MTB Version: ")+string(CString(m_conn->MTBVersion.GetBSTR())),true);

	config=m_conn->GetConfiguration((BSTR)m_ID);
	if (!config){
		logFile.write("No active configuration found.  Run MTBConfig.exe",true);
		isPresent=false;
		return;
	}
	
	string s(config->ReadActiveConfiguration());
	int pos1=s.find("Lamp.EmptyPort");
	int pos2=s.find("Lamp.Other");
	if (pos2!=string::npos && pos1==string::npos)
		isManLamp=true;
	else if (pos2==string::npos && pos1==string::npos){
		cout<<(int) string::npos<<endl;
		cout<<s<<endl;
		logFile.write("Error: RL Lamp not defined properly in MTB2004 Configuration.",true);
		logFile.write("       Please run MTBConfig.exe and select either \"Empty Lamp Port\" (DG5 or none) or \"Other Lamp\" (XCite) under Reflected Light",true);
	}

	m_pRoot = (IUnknown*)(m_conn->GetRoot((BSTR)m_ID));
	if (!m_pRoot){
		logFile.write("Observer server root not present.", true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;
	}
	}
	catch(_com_error e){
		logFile.write("Observer server root not present.", true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;
	}
	long numDevices=m_pRoot->GetDeviceCount();
	if (numDevices<1){
		logFile.write("Observer Microscope not found by MTB Server",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;
	}
		definiteFocus= (IMTBFocusStabilizerPtr) m_pRoot->GetComponent("MTBFocusStabilizer");
	if (!definiteFocus){
		logFile.write("definite focus not found",true);
		definiteFocusPresent=false;	
	}else{
		bool b=definiteFocus->SetExclusiveMode(MTBOnOff_Off);
		if (!b)
			logFile.write("Definite Focus Error: failed to set exclusive mode",true);
	}
	
	IMTBDevicePtr scope=m_pRoot->GetDevice(0);
	if (!scope){
		logFile.write("Observer Microscope not present.", true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;
	}
	//assign all devices
	reflectorTurret=(IMTBChangerPtr) scope->GetComponent("MTBReflectorChanger");
	if (!reflectorTurret){
		logFile.write("ReflectorTurret not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;}
	objectiveTurret=(IMTBChangerPtr) scope->GetComponent("MTBObjectiveChanger");
	if (!objectiveTurret){
		logFile.write("objectiveTurret not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;
			}
	if (((IMTBComponentPtr) objectiveTurret)->Motorization==MTBMotorization_Coded){
		objTurretCodedPosition=objectiveTurret->GetPosition();
	}
	optovarTurret=(IMTBChangerPtr) scope->GetComponent("MTBOptovarChanger");
	if (!optovarTurret){
		logFile.write("optovarTurret not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	halogen=(IMTBLampPtr) scope->GetComponent("MTBTLHalogenLamp");
	if (!halogen){
		logFile.write("halogen not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	baseport=(IMTBChangerPtr) scope->GetComponent("MTBBaseportChanger");
	if (!baseport){
		logFile.write("baseport not found",true);
		basePortPresent=false;
			}
	sideport=(IMTBChangerPtr) scope->GetComponent("MTBSideportChanger");
	if (!sideport){
		logFile.write("sideport not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	TLshutter=(IMTBShutterPtr) scope->GetComponent("MTBTLShutter");
	if (!TLshutter){
		logFile.write("TLshutter not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	RLshutter=(IMTBShutterPtr) scope->GetComponent("MTBRLShutter");
	if (!RLshutter){
		logFile.write("RLshutter not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	focus=(IMTBAxisPtr) scope->GetComponent("MTBFocus");
	if (!focus){
		logFile.write("focus not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
	condenserAperture=(IMTBContinualPtr) scope->GetComponent("MTBTLApertureStop");
	if (!condenserAperture){
		logFile.write("condenser aperture not found",true);
		isPresent=false;
		basePortPresent=false;
		definiteFocusPresent=false;
		isManLamp=true;
		return;	}
/*	long num=((IMTBRootPtr) scope)->GetDeviceCount();
	for (long i=0; i<num;i++){
		std::cout<<((IMTBIdentPtr)(((IMTBRootPtr) scope)->GetDevice(i)))->GetName()<<endl;
	}
*/

/*THIS CODE NOT WORKING.....YET!!!
	//create server event sinks
	m_COMfocusBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMfocusEvents.CreateInstance(CLSID_MTBContinualEventSink);
	m_COMfocusSpeedEvents.CreateInstance(CLSID_MTBMoveEventSink);
	m_COMhalogenBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMhalogenEvents.CreateInstance(CLSID_MTBContinualEventSink);
	m_COMreflectorBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMreflectorEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMobjectiveBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMobjectiveEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMoptovarEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMoptovarBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMbaseportEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMbaseportBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMsideportEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMsideportBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMTLshutterEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMTLshutterBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
	m_COMRLshutterEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMRLshutterBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);

	//the helper class requires the ID for satisfying event handling
	m_COMfocusBaseEvents->clientID=(BSTR)m_ID;
	m_COMfocusEvents->clientID=(BSTR)m_ID;
	m_COMfocusSpeedEvents->clientID=(BSTR)m_ID;
	m_COMhalogenBaseEvents->clientID=(BSTR)m_ID;
	m_COMhalogenEvents->clientID=(BSTR)m_ID;
	m_COMreflectorBaseEvents->clientID=(BSTR)m_ID;
	m_COMreflectorEvents->clientID=(BSTR)m_ID;
	m_COMobjectiveBaseEvents->clientID=(BSTR)m_ID;
	m_COMobjectiveEvents->clientID=(BSTR)m_ID;
	m_COMoptovarEvents->clientID=(BSTR)m_ID;
	m_COMoptovarBaseEvents->clientID=(BSTR)m_ID;
	m_COMbaseportEvents->clientID=(BSTR)m_ID;
	m_COMbaseportBaseEvents->clientID=(BSTR)m_ID;
	m_COMsideportEvents->clientID=(BSTR)m_ID;
	m_COMsideportBaseEvents->clientID=(BSTR)m_ID;
	m_COMTLshutterEvents->clientID=(BSTR)m_ID;
	m_COMTLshutterBaseEvents->clientID=(BSTR)m_ID;
	m_COMRLshutterEvents->clientID=(BSTR)m_ID;
	m_COMRLshutterBaseEvents->clientID=(BSTR)m_ID;

	//Advise local event sinks with server event sinks
	focusBaseEvents.DispEventAdvise(m_COMfocusBaseEvents);
	focusEvents.DispEventAdvise(m_COMfocusEvents);
	focusSpeedEvents.DispEventAdvise(m_COMfocusSpeedEvents);
	halogenBaseEvents.DispEventAdvise(m_COMhalogenBaseEvents);
	halogenEvents.DispEventAdvise(m_COMhalogenEvents);
	reflectorBaseEvents.DispEventAdvise(m_COMreflectorBaseEvents);
	reflectorEvents.DispEventAdvise(m_COMreflectorEvents);
	objectiveBaseEvents.DispEventAdvise(m_COMobjectiveBaseEvents);
	objectiveEvents.DispEventAdvise(m_COMobjectiveEvents);
	optovarEvents.DispEventAdvise(m_COMoptovarEvents);
	optovarBaseEvents.DispEventAdvise(m_COMoptovarBaseEvents);
	baseportEvents.DispEventAdvise(m_COMbaseportEvents);
	baseportBaseEvents.DispEventAdvise(m_COMbaseportBaseEvents);
	sideportEvents.DispEventAdvise(m_COMsideportEvents);
	sideportBaseEvents.DispEventAdvise(m_COMsideportBaseEvents);
	TLshutterEvents.DispEventAdvise(m_COMTLshutterEvents);
	TLshutterBaseEvents.DispEventAdvise(m_COMTLshutterBaseEvents);
	RLshutterEvents.DispEventAdvise(m_COMRLshutterEvents);
	RLshutterBaseEvents.DispEventAdvise(m_COMRLshutterBaseEvents);
	
	//Advise server event sinks with component interface
	m_COMfocusBaseEvents->Advise(focus.GetInterfacePtr());
	m_COMfocusEvents->Advise(((IMTBContinualPtr) focus).GetInterfacePtr());
	m_COMfocusSpeedEvents->Advise(focus.GetInterfacePtr());
	m_COMhalogenBaseEvents->Advise(halogen.GetInterfacePtr());
	m_COMhalogenEvents->Advise(halogen.GetInterfacePtr());
	m_COMreflectorBaseEvents->Advise(reflectorTurret.GetInterfacePtr());
	m_COMreflectorEvents->Advise(reflectorTurret.GetInterfacePtr());
	m_COMobjectiveBaseEvents->Advise(objectiveTurret.GetInterfacePtr());
	m_COMobjectiveEvents->Advise(objectiveTurret.GetInterfacePtr());
	m_COMoptovarEvents->Advise(optovarTurret.GetInterfacePtr());
	m_COMoptovarBaseEvents->Advise(optovarTurret.GetInterfacePtr());
	m_COMbaseportEvents->Advise(baseport.GetInterfacePtr());
	m_COMbaseportBaseEvents->Advise(baseport.GetInterfacePtr());
	m_COMsideportEvents->Advise(sideport.GetInterfacePtr());
	m_COMsideportBaseEvents->Advise(sideport.GetInterfacePtr());
	m_COMTLshutterEvents->Advise(TLshutter.GetInterfacePtr());
	m_COMTLshutterBaseEvents->Advise(TLshutter.GetInterfacePtr());
	m_COMRLshutterEvents->Advise(RLshutter.GetInterfacePtr());
	m_COMRLshutterBaseEvents->Advise(RLshutter.GetInterfacePtr());
*/

	//enum MTBCalibrationModes lower=1;
	//MTBCalibrationModes::
	//MTBCmdSetModes::
	closeShutter();
	if (!focus->Calibrate(MTBCalibrationModes_OnLowerLimit,MTBCmdSetModes_Default,10000)){
		logFile.write("Error: Unable to calibrate Z Stage",true);//cout<<"performing calibration"<<endl;
	}
	
	/*
	wchar_t micro[3]={181,'m',0};//this should be the greek letter mu
	try{
		
		((IMTBContinualPtr)focus)->GetPosition(BSTR(micro));
	}catch(_com_error e){
		Observer::DisplayError(&e);
	}
	cout<<"Focus position is "<<((IMTBContinualPtr)focus)->GetPosition(BSTR(micro))<<endl;
	*/


	//discover objectives
	IMTBObjectivePtr obj;
	double DOF;
	bool isOil=false;
	int startObj=0;
	StartErrChk
	int totalObj=objectiveTurret->GetElementCount();
	pos2ind=vector<int>(totalObj,-1);
	if (objTurretCodedPosition!=-1){//if coded we cannot move so only check current element
		startObj=getObj()-1;
		totalObj=startObj+1;
	}
	for(int i=startObj;i<totalObj;i++){
		obj=objectiveTurret->GetElement(i);
		if (!obj) continue;
		switch(int(obj->GetMagnification())){
			case 1:
				objPM= new Objective(1,.5,.1,false,&cont.objTurret,i+1,"Power Meter");
				pos2ind.at(i)=-2;//special objPM
				continue;
				break;
			case 10:
				DOF=1;//1 micron?
				break;
			case 20:
				DOF=.6;//600nm
				break;
			case 40:
				DOF=.4;//400nm
				break; 
			case 63:
				DOF=.160;//160nm
				break;
			case 100:
				DOF=.140;//140nm
				break;
			default:
				logFile.write(string("Unknown Objective: ")+toString(obj->GetMagnification())+"x",true);
				continue;//don't add anything to objectives vector, move on to next objective
		}
		if (obj->GetImmersionType()!=MTBObjectiveImmersionTypes_Air) isOil=true;
		objectives.push_back(new Objective(obj->GetMagnification(),DOF,obj->GetWorkingDistance(),isOil,&cont.objTurret,i+1,string(((IMTBIdentPtr)obj)->GetName())));
		pos2ind.at(i)=objectives.size()-1;
		logFile.write(string("Found Objective: ")+string(((IMTBIdentPtr)obj)->GetName()),true);
	}
	EndErrChk



	cout<<"Microscope ready"<<endl;
	/*bool workPosition=true;
	cout<<"Return to original z-position (y or n)?"<<endl;
	char c;
	cin>>c;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (c!='y'){
		workPosition=false;
	}
	///vel="48";//4.5mm/sec
	//acc="12"; //.018um/msec^2
*/
	SetHalogenVoltage(OBSERVERDEFAULTHALOGENVOLTAGE);
	HalogenOff();
	
	((IMTBBasePtr)focus)->WaitReady(10000);
//	cout<<"Wait Ready was called"<<endl;
	//focusBaseEvents.waitLock();
	//focusEvents.wait();
	//enable limits
	focus->SetMeasurementOrigin(0,micron);
	focus->EnableSWLimit(true,true);
	focus->EnableSWLimit(false,true);
	setMaxZ(softLimitZ);
	condenserAperture->SetPosition_2(OBSERVERCONDENSERNA,NA,MTBCmdSetModes_Default);
	WAIT(condenserAperture);
	//((IMTBObjectiveChangerPtr) objectiveTurret)->OilStopActive=false;
/*	if (workPosition) {
		cout<<"After return to work state position is: "<<getPos()<<endl;
	}	else {
		move(20);
	}
*/	IMTBMovePtr movePtr=(IMTBMovePtr)focus;
	IMTBContinualSpeedPtr speedPtr=(IMTBContinualSpeedPtr)focus;
	/*
	cout<<"Observer min move speed is "<<movePtr->GetMinMoveSpeed(movePtr->GetSpeedUnit(0))<<movePtr->GetSpeedUnit(0)<<endl;
	cout<<"Observer max move speed is "<<movePtr->GetMaxMoveSpeed(movePtr->GetSpeedUnit(0))<<movePtr->GetSpeedUnit(0)<<endl;
	cout<<"Observer min acceleration is"<<speedPtr->GetMinContinualAcceleration(speedPtr->GetContinualAccelerationUnit(0))<<speedPtr->GetContinualAccelerationUnit(0)<<endl;
	cout<<"Observer max acceleration is"<<speedPtr->GetMaxContinualAcceleration(speedPtr->GetContinualAccelerationUnit(0))<<speedPtr->GetContinualAccelerationUnit(0)<<endl;
	
	cout<<"Observer min continual speed is"<<speedPtr->GetMinContinualSpeed(speedPtr->GetContinualSpeedUnit(0))<<speedPtr->GetContinualSpeedUnit(0)<<endl;
	cout<<"Observer max continual speed is"<<speedPtr->GetMaxContinualSpeed(speedPtr->GetContinualSpeedUnit(0))<<speedPtr->GetContinualSpeedUnit(0)<<endl;
	cout<<"Start speed unit"<<speedPtr->GetContinualStartSpeedUnit(0)<<endl;
	cout<<"Observer min continual start speed is"<<speedPtr->GetMinContinualStartSpeed(speedPtr->GetContinualStartSpeedUnit(0))<<speedPtr->GetContinualStartSpeedUnit(0)<<endl;
	cout<<"Observer max continual start speed is"<<speedPtr->GetMaxContinualStartSpeed(speedPtr->GetContinualStartSpeedUnit(0))<<speedPtr->GetContinualStartSpeedUnit(0)<<endl;
	*/
	speedPtr->SetContinualAcceleration(10000,speedPtr->GetContinualAccelerationUnit(0));
	speedPtr->SetContinualSpeed(6000,speedPtr->GetContinualSpeedUnit(0));
	}catch(_com_error e){isPresent=false;Observer::DisplayError(&e);logFile.write("Observer Not Present",true);}
}

Objective* Observer::getCurrentObjective(){
	CheckExists(NULL)
	int ind=getObj();
	if (ind<1)//invalid position
		return NULL;
	int val=pos2ind.at(ind-1);	
	if (val==-1)
		return NULL;
	if (val==-2)//special power meter position
		return objPM;
	return objectives.at(val); 
}

int Observer::getObjectiveIndex(Objective* obj){
	int i=0;
	for(vector<Objective*>::iterator o=objectives.begin();o!=objectives.end();o++){
		if (*o == obj)
			return i;
		i++;
	}
	return -1;
}



void Observer::DisplayError(_com_error* e){
   CString message;
   // if it is an application error thrown by .NET
   if (e->Error() >= 0x80041000){
      IErrorInfo* info;
      BSTR msg;
      info = e->ErrorInfo();
      info->GetDescription(&msg);
      info->Release();
      message = CString(msg);
   }
   // other com errors
   else{
      message = e->ErrorMessage();
   }
   logFile.write(string(message),true);
   clickAbort();
} 

void Observer::setMaxZ(double z){
	CheckExists()
	SetLimits(0,z);
}
double Observer::getMaxZ(){
	CheckExists(0)
	return focus->GetSWLimit(true,micron);
}

Observer::~Observer(){
	CheckExists()
	StartErrChk
	stop();
	closeShutter();
	//disable limits
	focus->EnableSWLimit(true,false);
	focus->EnableSWLimit(false,false);
	halogen->SetOnOff_2(MTBOnOff_Off,MTBCmdSetModes_Default);
	((IMTBChangerPtr) RLshutter)->SetPosition_2(MTBOnOff_Off,MTBCmdSetModes_Default);
	((IMTBChangerPtr) TLshutter)->SetPosition_2(MTBOnOff_Off,MTBCmdSetModes_Default);
	
	//unadvise server sinks
	/*m_COMfocusBaseEvents->Unadvise(focus.GetInterfacePtr());
	m_COMfocusEvents->Unadvise(focus.GetInterfacePtr());
	m_COMfocusSpeedEvents->Unadvise(focus.GetInterfacePtr());
	m_COMhalogenBaseEvents->Unadvise(halogen.GetInterfacePtr());
	m_COMhalogenEvents->Unadvise(halogen.GetInterfacePtr());
	m_COMreflectorBaseEvents->Unadvise(reflectorTurret.GetInterfacePtr());
	m_COMreflectorEvents->Unadvise(reflectorTurret.GetInterfacePtr());
	m_COMobjectiveBaseEvents->Unadvise(objectiveTurret.GetInterfacePtr());
	m_COMobjectiveEvents->Unadvise(objectiveTurret.GetInterfacePtr());
	m_COMoptovarEvents->Unadvise(optovarTurret.GetInterfacePtr());
	m_COMoptovarBaseEvents->Unadvise(optovarTurret.GetInterfacePtr());
	m_COMbaseportEvents->Unadvise(baseport.GetInterfacePtr());
	m_COMbaseportBaseEvents->Unadvise(baseport.GetInterfacePtr());
	m_COMsideportEvents->Unadvise(sideport.GetInterfacePtr());
	m_COMsideportBaseEvents->Unadvise(sideport.GetInterfacePtr());
	m_COMTLshutterEvents->Unadvise(TLshutter.GetInterfacePtr());
	m_COMTLshutterBaseEvents->Unadvise(TLshutter.GetInterfacePtr());
	m_COMRLshutterEvents->Unadvise(RLshutter.GetInterfacePtr());
	m_COMRLshutterBaseEvents->Unadvise(RLshutter.GetInterfacePtr());*/
	
	//unadvise local sinks
	focusBaseEvents.DispEventUnadvise(m_COMfocusBaseEvents);
	focusEvents.DispEventUnadvise(m_COMfocusEvents);
	focusSpeedEvents.DispEventUnadvise(m_COMfocusSpeedEvents);
	halogenBaseEvents.DispEventUnadvise(m_COMhalogenBaseEvents);
	halogenEvents.DispEventUnadvise(m_COMhalogenEvents);
	reflectorBaseEvents.DispEventUnadvise(m_COMreflectorBaseEvents);
	reflectorEvents.DispEventUnadvise(m_COMreflectorEvents);
	objectiveBaseEvents.DispEventUnadvise(m_COMobjectiveBaseEvents);
	objectiveEvents.DispEventUnadvise(m_COMobjectiveEvents);
	optovarEvents.DispEventUnadvise(m_COMoptovarEvents);
	optovarBaseEvents.DispEventUnadvise(m_COMoptovarBaseEvents);
	baseportEvents.DispEventUnadvise(m_COMbaseportEvents);
	baseportBaseEvents.DispEventUnadvise(m_COMbaseportBaseEvents);
	sideportEvents.DispEventUnadvise(m_COMsideportEvents);
	sideportBaseEvents.DispEventUnadvise(m_COMsideportBaseEvents);
	TLshutterEvents.DispEventUnadvise(m_COMTLshutterEvents);
	TLshutterBaseEvents.DispEventUnadvise(m_COMTLshutterBaseEvents);
	RLshutterEvents.DispEventUnadvise(m_COMRLshutterEvents);
	RLshutterBaseEvents.DispEventUnadvise(m_COMRLshutterBaseEvents);

	//delete objectives;
	delete objPM;
	for (vector<Objective*>::iterator i=objectives.begin();i!=objectives.end();i++){
		delete *i;
	}

	// if connection is available: close it
	if (m_conn != NULL )
		m_conn->Close();

	::_Module.Term();
	// be a good citizen and clean up COM
	CoUninitialize();
	EndErrChk
}

bool Observer::go(double zspeed){
	CheckExists(false)
		logFile.write(string("Moving Z at constant speed: ")+toString(zspeed)+" um/sec");
	if (abs(zspeed)<0.062){//max is 6000
		logFile.write("Cannot move focus that slowly");
		return false;
		logFile.write("Setting move speed to minimum, Autofocus will not work properly!!!",true);
		if (zspeed<0) zspeed=-0.062;
		else zspeed=0.062;
	}

	StartErrChk
		::Timer t;
	double start=this->getPos();
	moveSpeedSettledPosition=start;
	//int num100ms=zspeed/10/100+2;
	this->move(start-zspeed*.1*3);//300ms delay  THIS IS BAD FOR PHOTOBLEACHING, NEED EVENTS
	//while(start<=this->getPos()){}
	//t.startTimer();
	WAIT(focus);
	double begin=this->getPos();
	double point;
	((IMTBMovePtr)focus)->Move(zspeed,micronpersec);
	//t.startTimer();
	while(true){
		point=this->getPos();
		if (point!=begin) break;
		//if (t.getTime()>200) {
		//	logFile.write("Observer: velocity move did not respond",true);
		//	system("pause");
		//}
	}
	begin=point;
	//t.resetTimer();
	//t.startTimer();
	while(true){
		point=this->getPos();
		if (point!=begin) break;
		//if (t.getTime()>200) logFile.write("Observer: pos 2->velcity move did not respond",true);
	}
	if (start<point) logFile.write("Focus move error: already above start point",true);
	::Timer::wait(1000.0*(start-point)/zspeed);
	//	logFile.write(string("Focus waited ")+toString(1000.0*(start-point)/zspeed)+" ms for start position");
	
	
	//::Timer::wait(zspeed/10.0);//1000.0*zspeed/10000.0    acceleration=10mm/sec*sec
	//t.waitAfterLastStart(100*num100ms-50);
	//start=this->getPos();
	
	//while(start>=moveSpeedSettledPosition){
	//moveSpeedSettledPosition=this->getPos();
	//}
	EndErrChk
	return true;
}


double Observer::getStartPosition(){
	CheckExists(0)
	return moveSpeedSettledPosition;
}
void Observer::stop(){
	CheckExists()
	StartErrChk
	((IMTBMovePtr)focus)->Stop();
	EndErrChk
}

//this function uses the built-in backlash compensation of the microscope
void Observer::move(double zpos){
	CheckExists()
	bool b=false;
	StartErrChk	
	if (this->definiteFocusPresent && this->getExclusive()){
		b=true;
		this->setExclusive(false);
		//waitObserver((MTBApi::IMTBBasePtr)definiteFocus);
		::Timer::wait(200);
	}
	((IMTBContinualPtr)focus)->SetPosition_2(zpos,micron,(MTBCmdSetModes) (MTBCmdSetModes_Default | MTBCmdSetModes_BidirectionalBacklash));
	if (b)
		this->setExclusive(true);
	EndErrChk
}
double Observer::getPos(){
	CheckExists(-1)
	return ((IMTBContinualPtr)focus)->GetPosition(micron);
}

//pos will have the desired position, if the current position is within some tolerance of that position (maybe should be exact)
void Observer::SetLimits(double min,double max){
	CheckExists()
		bool b;
	b=focus->SetSWLimit(true,max,micron);
	b=focus->SetSWLimit(false,min,micron);
	::Timer::wait(100);
}

int Observer::getObj(){
	return objectiveTurret->GetPosition();
}

void Observer::SetObj(int i){
	if (objTurretCodedPosition==i) return;
	if (objTurretCodedPosition!=-1) {
		logFile.write("Error: Move to a different objective is not allowed. Objective Motor is disabled",true);
		return;
	}
	StartErrChk
	objectiveTurret->SetPosition_2((short)i,MTBCmdSetModes_Default);
	EndErrChk
}


void Observer::SetTurret(int i){//1 is pinkel 0 is BF
	CheckExists()
	StartErrChk
	reflectorTurret->SetPosition_2(i,MTBCmdSetModes_Default);
	EndErrChk
}

void Observer::SetHalogenVoltage(double volts){
	CheckExists()
	StartErrChk
	((IMTBContinualPtr)halogen)->SetPosition_2(volts,volt,MTBCmdSetModes_Default);
	EndErrChk
}

double Observer::GetHalogenVoltage(){
	CheckExists(0)
	double v;
	StartErrChk
	v= ((IMTBContinualPtr)halogen)->GetPosition(volt);
	EndErrChk
	return v;
}

void Observer::HalogenOff(bool wait){
	closeTLShutter();
}
void Observer::HalogenOn(double volts, bool wait){
	CheckExists()
	StartErrChk
	SetHalogenVoltage(volts);
	openTLShutter();
	if (wait){
		WAIT(halogen);
		WAIT(TLshutter);
	}
	EndErrChk
}

/*light path positions 
			1: Left (side 2)
			2: Right (side 3) 
			3:Binoculars (side 1 base 2)
			4: Frontport (side 1 base 3)
			5:Baseport (side 1 base 1)
*/
void Observer::SidePort(int pos){
	CheckExists()
	StartErrChk
	switch(pos){
		case 1:
			sideport->SetPosition_2(2,MTBCmdSetModes_Default);
			break;
		case 2:
			sideport->SetPosition_2(3,MTBCmdSetModes_Default);
			break;
		case 3:
			sideport->SetPosition_2(1,MTBCmdSetModes_Default);
			if (basePortPresent) baseport->SetPosition_2(2,MTBCmdSetModes_Default);
			break;
		case 4:
			sideport->SetPosition_2(1,MTBCmdSetModes_Default);
			if (basePortPresent) baseport->SetPosition_2(3,MTBCmdSetModes_Default);

			break;
		case 5:
			sideport->SetPosition_2(1,MTBCmdSetModes_Default);
			if (basePortPresent) baseport->SetPosition_2(1,MTBCmdSetModes_Default);
			break;
		default:
			logFile.write(string("Observer: Light Path Position ")+::toString(pos)+" is not valid",true);
	}
	EndErrChk
}

int Observer::getSidePort(){
	CheckExists(0)
	int ret,pos;
	StartErrChk
	pos=sideport->GetPosition();
	switch(pos){
		case 2:
			ret=1;
			break;
		case 3:
			ret=2;
			break;
		case 1:
			if (basePortPresent){
				switch(baseport->GetPosition()){
					case 2:
						ret=3;
					case 3:
						ret=4;
					case 1:
						ret=5;
				}
			}
			ret=3;
			break;
		case 0:
			logFile.write("Observer: sideport was in undetermined position (was it moving?)",true);
			break;
		default:
			logFile.write(string("Observer: Unknown sideport position ")+::toString(pos),true);
			ret=0;
	} 
	EndErrChk
	return ret;
}

void Observer::SetOptovar(int opt){
	CheckExists()
	StartErrChk
	optovarTurret->SetPosition_2(opt,MTBCmdSetModes_Default);
	EndErrChk
}

int Observer::getTurret(){
	CheckExists(0)
	int turret;
	StartErrChk
	turret= reflectorTurret->GetPosition();
	EndErrChk
	return turret;
}

int Observer::getOptovar(){
	CheckExists(0)
	return optovarTurret->GetPosition();
}

void Observer::closeTLShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr) TLshutter)->SetPosition_2(1,MTBCmdSetModes_Default);
	EndErrChk
}

void Observer::openTLShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr)TLshutter)->SetPosition_2(2,MTBCmdSetModes_Default);
	EndErrChk
}

void Observer::closeShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr) RLshutter)->SetPosition_2(1,MTBCmdSetModes_Default);
	EndErrChk
}
void Observer::openShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr) RLshutter)->SetPosition_2(2,MTBCmdSetModes_Default);
	EndErrChk
}

//definite focus class definitions
ObserverDefiniteFocus::ObserverDefiniteFocus(Observer* a):a(a){
	if (!a->definiteFocusPresent) 
		return;
	this->stopDefiniteFocus();
}

bool ObserverDefiniteFocus::initializeDefiniteFocus(){
	CheckExistsDF(false)
	return internalInitializeDefiniteFocus(0);
}

bool ObserverDefiniteFocus::initializeDefiniteFocus(int index){
	CheckExistsDF(false)
	if (index<1){
		logFile.write("Error Definite Focus: index must be greater than or equal to 1",true);
		return false;
	}else{
		return internalInitializeDefiniteFocus(index);
	}
}

int getSafeArraySize(SAFEARRAY* psa){
	int ret=1;
	for(int i=0;i<SafeArrayGetDim(psa);i++){
		ret=ret*psa->rgsabound[i].cElements;
	}
	return ret*SafeArrayGetElemsize(psa);
}

bool ObserverDefiniteFocus::isStandardMode(){
	CheckExistsDF(false)
	SAFEARRAY* b;
	StartErrChk	
	b=a->definiteFocus->GetStabilizerData(MTBFocusStabilizerDataType_Current);
	EndErrChk
	int s=::getSafeArraySize(b);
	if (s==11)
		return true;
	if (s==15)
		return false;
	else logFile.write(string("No Stabilization data: ")+toString(s),true);
	return false;
}

bool ObserverDefiniteFocus::internalInitializeDefiniteFocus(int index){//use current position for initializing the definite focus. num is the index in the StabilizerData array for this initialization.  most people will only want one so the index will default to 0
	
		SAFEARRAY* b=NULL;
	StartErrChk	
		if (!a->definiteFocus->InitOnCurrentFocusPosition_6(&b,DEFINITEFOCUSTIMEOUT)){
			logFile.write("Error. Definite Focus initialization timed out",true);
			return false;
		}
	EndErrChk
		if (stabilizationData.size()<index+1){
			while(stabilizationData.size()<index+1){
				stabilizationData.push_back(NULL);
			}
		}
		if (isInitialized(index)){
			logFile.write(string("Definite Focus: overwriting previous stabilization data at index")+toString(index),DEBUGDEFINITEFOCUS);
		}
		stabilizationData[index]=b;
		logFile.write(string("Definite Focus: saving stabilization data at index")+toString(index),DEBUGDEFINITEFOCUS);
		logFile.write(string("Stabilization Data contained ")+toString(::getSafeArraySize(b))+" bytes (15 for reference mode BAD, 11 for standard mode GOOD)",DEBUGDEFINITEFOCUS);
		currentIndex=index;
		return true;
}


bool ObserverDefiniteFocus::isInitialized(int index){
	CheckExistsDF(true)
	if (stabilizationData.size()<index+1 || stabilizationData[index]==NULL)
		return false;
	else return true;
}

bool ObserverDefiniteFocus::loadInitializationData(int index){
	CheckExistsDF(true)
	if (index<1){
		logFile.write("Error Definite Focus: loading inititalization data, index must be greater than or equal to 1",true);
		return false;
	}else{
		return internalLoadInitializationData(index);
	}
}

bool ObserverDefiniteFocus::internalLoadInitializationData(int index){//
	bool b;
	if (isInitialized(index)){
		StartErrChk
		b=a->definiteFocus->InitWithStabilizingData_2(stabilizationData[index],MTBCmdSetModes_Default,DEFINITEFOCUSTIMEOUT);
		EndErrChk
		if (!b){
			logFile.write("Error: Definite focus initialization time out",true);
			return false;
		}
		currentIndex=index;
		return true;
	}
	logFile.write("Error: Definite focus is not initialized",true);
	return false;
}

void ObserverDefiniteFocus::removeInitializationData(){
CheckExistsDF()
	internalRemoveInitializationData(0);
}

void ObserverDefiniteFocus::removeInitializationData(int index){
CheckExistsDF()
	if (index<1)
		logFile.write("Error: index must be greater than or equal to 1 to remove indexed initialization data",true);
	internalRemoveInitializationData(index);	
}
void ObserverDefiniteFocus::internalRemoveInitializationData(int index){
	if (isInitialized(index)){
		stabilizationData[index]=NULL;
	} 
}

//remove all but the zeroth
void ObserverDefiniteFocus::clearIndexedInitializationData(){
	if (stabilizationData.empty()) 
		return;
	for(vector<SAFEARRAY*>::iterator i=stabilizationData.begin()+1;i!=stabilizationData.end();i++){
		*i=NULL;
	}
}

bool ObserverDefiniteFocus::getDefiniteFocus(bool wait){
	if (!a->definiteFocusPresent) {
			logFile.write("Error: Attempt to use definite focus when it is not present",true);	
			return false;
		}
	bool b;
	if (!isInitialized(0)){
		logFile.write("Error: Definite Focus has not been initialized",true);
		return false;
	}
	if (currentIndex!=0){
		b=internalLoadInitializationData(0);
		if (!b)
			return false;
	}

	return internalGetDefiniteFocus(wait);
}

bool ObserverDefiniteFocus::getDefiniteFocus(int index,bool wait){
	CheckExistsDF(false)
	if (index==-1 && currentIndex==0){
		logFile.write("Error: Definite Focus: an indexed stabilization point is not initialized, call this function with a valid index or call loadInitializationData first",true);
		return false;
	}
	if (index!=-1 && !isInitialized(index)){
		logFile.write("Error: Definite Focus has not been initialized",true);
		return false;
	}
	if  (index!=-1 && index!=currentIndex){
		bool b=loadInitializationData(index);
		if (!b){
			return false;
		}
	}
	return internalGetDefiniteFocus(wait);
}

bool ObserverDefiniteFocus::internalGetDefiniteFocus(bool wait){//get focus once and wait if necessary
	bool b=false;
	short s;
	if (wait){
#ifdef DEBUGDEFINITEFOCUS
		::Timer t;
		if (DEBUGDEFINITEFOCUS){
			t.startTimer();
		}
#endif
		StartErrChk
		s=	a->definiteFocus->StabilizeNow_2(MTBCmdSetModes_Synchronous,DEFINITEFOCUSTIMEOUT);
		b=s;
		EndErrChk
#ifdef DEBUGDEFINITEFOCUS
		if (DEBUGDEFINITEFOCUS){
			t.stopTimer();
			logFile.write(string("Definite focus took ")+::toString(t.getTime()),true);
		}
#endif
		if (!b){
			logFile.write("Error. Definite Focus timed out",true);
			return false;
		}
	}else{
		StartErrChk
		s=a->definiteFocus->StabilizeNow_2(MTBCmdSetModes_Default,DEFINITEFOCUSTIMEOUT);
		b=s;
		EndErrChk
		if (!b){
			logFile.write("Error. Definite Focus timed out",true);
			return false;
		}
	}
	return true;
}

bool ObserverDefiniteFocus::startDefiniteFocus(unsigned long periodSec){
	CheckExistsDF(false)
	bool b;
	if (currentIndex!=0){
		b=internalLoadInitializationData(0);
		if (!b)
			return false;
	}
	return internalStartDefiniteFocus(periodSec);
}

bool ObserverDefiniteFocus::startDefiniteFocus(int index,unsigned long periodSec){
	CheckExistsDF(false)
	if (index==-1 && currentIndex==0){
		logFile.write("Error: Definite Focus: an indexed stabilization point is not initialized, call this function with a valid index or call loadInitializationData first",true);
		return false;
	}
	if  (index!=-1 && index!=currentIndex){
		bool b=loadInitializationData(index);
		if (!b){
			return false;
		}
	}
	return internalStartDefiniteFocus(periodSec);
}

	bool ObserverDefiniteFocus::internalStartDefiniteFocus(unsigned long periodSec){//start periodic definite focusing with the given period, a value of zero will be as fast as possible
		bool b;
		StartErrChk
		b=a->definiteFocus->StabilizePeriodically_2(MTBOnOff_On,periodSec);
		EndErrChk
		if (!b){
			logFile.write("Error: could not start periodic definite focus, timeout",true);
			return false;
		}
		return true;
	}



	bool ObserverDefiniteFocus::isDFPresent(){
		if (!a->definiteFocusPresent)
			logFile.write("Error: Attempt to use definite focus when it is not present",true);	
		return a->definiteFocusPresent;
	}
void ObserverDefiniteFocus::stopDefiniteFocus(){//stop periodic definite focusing
	CheckExistsDF()
		StartErrChk
		a->definiteFocus->StabilizePeriodically(MTBOnOff_Off);
		EndErrChk
	}

void ObserverDefiniteFocus::wait(){
	CheckExistsDF()
		StartErrChk
		a->waitObserver((MTBApi::IMTBBasePtr)a->definiteFocus);
		EndErrChk
	}

bool Observer::getExclusive(){
	if (!this->definiteFocusPresent)
		return false;
	MTBOnOff val;
	val=definiteFocus->GetExclusiveModeOnOff();
	if (val==MTBOnOff_On)
		return true;
	else
		return false;
}


void Observer::setExclusive(bool isOn){
	if (!this->definiteFocusPresent)
		return;
	MTBOnOff val;
	if (isOn)
		val=MTBOnOff_On;
	else
		val=MTBOnOff_Off;
	StartErrChk
	waitObserver((MTBApi::IMTBBasePtr)focus);
	bool b=definiteFocus->SetExclusiveMode(val);
	if (!b)
		logFile.write("Definite Focus Error: failed to set exclusive mode",true);
	EndErrChk
	
}

void ObserverDefiniteFocus::definiteFocusControl(){
	CheckExistsDF()
		char c;
	string periodSec;
	int index;
	string t3;
	while(true){
		cout<<"Please select a task"<<endl;
		cout<<"0: Start continous focusing on current position"<<endl;
		cout<<"1: Start periodic focusing on current position"<<endl;
		cout<<"2: Stop Focusing"<<endl;
		cout<<"3: Focus once"<<endl;
		cout<<"4: Save current position data (exclusive mode will be on)"<<endl;
		cout<<"5: Stop exclusive mode"<<endl;
		cout<<"6: Autofocus and initialize definite focus (exclusive mode will be on)"<<endl;
		cout<<"7: Focus once and snap picture"<<endl;
		cout<<"8: Get Focus Mode (standard or reference)"<<endl;
		cout<<"9: Get Status of definite focus"<<endl;
		cout<<"e: Exit Definite Focus Control"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '0'://start continuous focusing
				cont.df.initializeDefiniteFocus();
				cont.df.startDefiniteFocus();
				break;
			case '1'://start periodic focusing
				cont.df.initializeDefiniteFocus();
				cout<<"Please enter desired period in seconds"<<endl;
				cin>>periodSec;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.df.startDefiniteFocus(toInt(periodSec));
				break;
			case '2'://stop focusing
				cont.df.stopDefiniteFocus();
				break;
			case '3'://focus once
				{::Timer t(true);
				cont.df.getDefiniteFocus(cont.df.currentIndex);
				cout<<"Focus took "<<t.getTime()<<"ms"<<endl;
				break;}
			case '4'://save current focus position data
				cont.df.setExclusive(true);
				if (protocolIndex==-1){
					cout<<"Please enter desired index for focus position data"<<endl;
					cont.df.initializeDefiniteFocus(getInt());
				}else{
					logFile.write(string("Definite Focus: initializing for protocol file ")+::toString(protocolIndex)+protocolFiles.at(protocolIndex-1),true);
					cont.df.initializeDefiniteFocus(protocolIndex);
				}
					
				break;
			case '5'://load position data
				cont.df.setExclusive(false);
				break;
			case '6':{
				int i;
				cont.df.setExclusive(false);
				if (protocolIndex==-1){
					cout<<"Please enter desired index for focus position data"<<endl;
					i=getInt();
					cont.currentFocus.modify();
					cont.currentFocus.getFocus();
					cont.df.initializeDefiniteFocus(i);
				}else{
					cont.currentFocus.modify();
					cont.currentFocus.getFocus();
					logFile.write(string("Definite Focus: initializing for protocol file ")+::toString(protocolIndex)+protocolFiles.at(protocolIndex-1),true);
					cont.df.initializeDefiniteFocus(protocolIndex);
				}
				cont.df.setExclusive(true);
				showConsole();
				break;}
			case '7':
				cont.currentChannel().modify();
				cont.df.getDefiniteFocus(cont.df.currentIndex);
				cont.currentChannel().takePicture("definite focus test");
				break;
			case '8':
				if (isStandardMode())
					logFile.write("DF is in standard mode",true);
				else
					logFile.write("DF is in reference mode, this is not ideal",true);
				break;
			case '9':
				if (cont.df.isOn())
					cout<<"definite focus is on"<<endl;
				else
					cout<<"definite focus is off"<<endl;
				break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
	
	}


bool ObserverDefiniteFocus::isOn(){
		MTBFocusStabilizerPeriodicallyOnOffChangedReason reason;
		MTBOnOff DFSstatus;
		DFSstatus=a->definiteFocus->GetStabilizePeriodicallyOnOff(&reason);
		return DFSstatus;
}
int ObserverDefiniteFocus::getOnOffChangedReason(){
		MTBFocusStabilizerPeriodicallyOnOffChangedReason reason;
		a->definiteFocus->GetStabilizePeriodicallyOnOff(&reason);
		return reason;		
}
#endif