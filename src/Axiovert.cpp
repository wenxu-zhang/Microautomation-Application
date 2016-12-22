// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Warn.h"

#include <iostream>
#include <string>
#include "Axiovert.h"
#include "Controller.h"
#include <limits>
#include "Utils.h"
#include <atlstr.h>

#ifdef AXIOVERT

#define DEBUGAXIOVERT true
using namespace MTBApi;
using namespace mscorlib;
extern Record logFile;
extern Controller cont;
wchar_t micron[3]={181,'m',0};
wchar_t micronpersec[5]={181,'m','/','s',0};
wchar_t volt[5]={'V','o','l','t',0};
wchar_t percent[2]={'%',0};
wchar_t NA[3]={'N','A',0};
const double Axiovert::softLimitZ=6700;
CComModule _Module;
#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
MIDL_DEFINE_GUID(CLSID, CLSID_MTBCOMContinualEventSink,0xece98723,0x4e0e,0x42eb,0x8a,0x82,0x0b,0x87,0x71,0xa2,0xc3,0xca);

Axiovert::Axiovert(){
	StartErrChk	
	isManLamp=false;
	objTurretCodedPosition=-1;
	//MTB interface ptr to the root of the tree of components of the microscope
	IMTBRootPtr m_pRoot;
	isPresent=true;
	bool b;
	m_conn=NULL;
	m_pRoot=NULL;
	CoInitialize(NULL);
	::_Module.Init(NULL, GetModuleHandle(NULL));
	try{
		m_conn = IMTBConnectionPtr(CLSID_MTBConnection);
		if (!m_conn){
			logFile.write("Axiovert server not present.", true);
			isPresent=false;
			return;
		}
		m_conn->Login("en", &m_ID);
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
			logFile.write("Axiovert server root not present.", true);
			isPresent=false;
			return;
		}
		b=((IMTBBasePtr)m_pRoot)->StartMonitoring((BSTR)m_ID);
	}
	catch(_com_error e){
		logFile.write("Axiovert server root not present.", true);
		isPresent=false;
		return;
	}
	long numDevices=m_pRoot->GetDeviceCount();
	/*Display all of the devices for debuggin purpose
	for (long i=0; i<numDevices;i++){
		std::cout<<((IMTBIdentPtr)(((IMTBRootPtr) m_pRoot)->GetDevice(i)))->GetName()<<endl;
	}*/
	if (numDevices<1){
		logFile.write("Axiovert Microscope not found by MTB Server",true);
		isPresent=false;
		return;
	}
	IMTBDevicePtr scope=m_pRoot->GetDevice(0);
	if (!scope){
		logFile.write("Axiovert Microscope not present.", true);
		isPresent=false;
		return;
	}
	b=((IMTBBasePtr)scope)->StartMonitoring((BSTR)m_ID);
	/*Display all of the components for debugging purpose
	long num=((IMTBDevicePtr) scope)->GetComponentCount();
	for (long i=0; i<num;i++){
		std::cout<<((IMTBIdentPtr)(scope->GetComponent_2(i)))->GetName()<<endl;
	}*/
	//assign all devices
	reflectorTurret=(IMTBChangerPtr) scope->GetComponent("MTBReflectorChanger");
	if (!reflectorTurret){
		logFile.write("ReflectorTurret not found",true);
		isPresent=false;
		return;}
	b=((IMTBBasePtr)reflectorTurret)->StartMonitoring((BSTR)m_ID);
	objectiveTurret=(IMTBChangerPtr) scope->GetComponent("MTBObjectiveChanger");
	if (!objectiveTurret){
		logFile.write("objectiveTurret not found",true);
		isPresent=false;
		return;
			}
	b=((IMTBBasePtr)objectiveTurret)->StartMonitoring((BSTR)m_ID);
	if (((IMTBComponentPtr) objectiveTurret)->Motorization==MTBMotorization_Coded){
		objTurretCodedPosition=objectiveTurret->GetPosition();
	}
	optovarTurret=(IMTBChangerPtr) scope->GetComponent("MTBOptovarChanger");
	if (!optovarTurret){
		logFile.write("optovarTurret not found",true);
		isPresent=false;
		return;	}
	b=((IMTBBasePtr)optovarTurret)->StartMonitoring((BSTR)m_ID);
	halogen=(IMTBLampPtr) scope->GetComponent("MTBTLHalogenLamp");
	if (!halogen){
		logFile.write("halogen not found",true);
		isPresent=false;
		return;	}
	b=((IMTBBasePtr)halogen)->StartMonitoring((BSTR)m_ID);
	sideport=(IMTBChangerPtr) scope->GetComponent("MTBSideportChanger");
	if (!sideport){
		logFile.write("sideport not found",true);
		isPresent=false;
		return;	}
	b=((IMTBBasePtr)sideport)->StartMonitoring((BSTR)m_ID);
	RLshutter=(IMTBShutterPtr) scope->GetComponent("MTBRLShutter");
	if (!RLshutter){
		logFile.write("RLshutter not found",true);
		isPresent=false;
		return;	}
	b=((IMTBBasePtr)RLshutter)->StartMonitoring((BSTR)m_ID);
	IMTBDevicePtr focusDevice=m_pRoot->GetDevice(1);
	if (!focusDevice){	
		logFile.write("focusDevice not found",true);
		isPresent=false;
		return;	
	}
	b=((IMTBBasePtr)focusDevice)->StartMonitoring((BSTR)m_ID);
	/*Display all of the components for debuggin purpose
	num=((IMTBDevicePtr) focusDevice)->GetComponentCount();
	for (long i=0; i<num;i++){
		std::cout<<((IMTBIdentPtr)(focusDevice->GetComponent_2(i)))->GetName()<<endl;
	}*/
	focus=(IMTBAxisPtr) focusDevice->GetComponent("MTBFocus");
	if (!focus){
		logFile.write("focus not found",true);
		isPresent=false;
		return;}
	b=((IMTBBasePtr)focus)->StartMonitoring((BSTR)m_ID);
/*
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
	m_COMsideportEvents.CreateInstance(CLSID_MTBChangerEventSink);
	m_COMsideportBaseEvents.CreateInstance(CLSID_MTBBaseEventSink);
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
	m_COMsideportEvents->clientID=(BSTR)m_ID;
	m_COMsideportBaseEvents->clientID=(BSTR)m_ID;
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
	sideportEvents.DispEventAdvise(m_COMsideportEvents);
	sideportBaseEvents.DispEventAdvise(m_COMsideportBaseEvents);
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
	m_COMsideportEvents->Advise(sideport.GetInterfacePtr());
	m_COMsideportBaseEvents->Advise(sideport.GetInterfacePtr());
	m_COMRLshutterEvents->Advise(RLshutter.GetInterfacePtr());
	m_COMRLshutterBaseEvents->Advise(RLshutter.GetInterfacePtr());
*/
	closeShutter();
	//double originalPos=focus->GetMeasurementPosition(micron);
	//cout<<"Original position when the microscope was turned on: "<<originalPos<<endl;
	if (!focus->Calibrate(MTBCalibrationModes_OnLowerLimit,MTBCmdSetModes_Synchronous,10000)){
		logFile.write("Error: Unable to calibrate Z Stage",true);//cout<<"performing calibration"<<endl;
	}
	//cout<<"Position after calibration: "<<focus->GetMeasurementPosition(micron)<<endl;
	cout<<"Microscope ready"<<endl;

	HalogenOff(true);
	
	//focus->SetLoadWork(MTBLoadWorkPosition_Load,MTBCmdSetModes_Synchronous,15000);
	((IMTBBasePtr)focus)->WaitReady(10000);
	//::Timer::wait(60000);
	//cout<<"Position before SetMeasurementOrigin: "<<focus->GetMeasurementPosition(micron)<<endl;
	//focus->SetMeasurementOrigin(0,micron);
	focus->SetMeasurementPosition(0,micron);//This is replacing the above to use the measurement positions while the calibration is not working.
	//cout<<"Position after SetMeasurementOrigin: "<<focus->GetMeasurementPosition(micron)<<endl;
	//focus->EnableSWLimit(true,true);
	focus->EnableSWLimit(false,true);
	setMaxZ(softLimitZ);
	IMTBMovePtr movePtr=(IMTBMovePtr)focus;
	IMTBContinualSpeedPtr speedPtr=(IMTBContinualSpeedPtr)focus;

	speedPtr->SetContinualAcceleration(10000,speedPtr->GetContinualAccelerationUnit(0));
	speedPtr->SetContinualSpeed(6000,speedPtr->GetContinualSpeedUnit(0));
	//((IMTBContinualPtr)focus)->SetPosition_2(originalPos,micron,MTBCmdSetModes_Default);
	EndErrChk

	/*old CAN commands
	isPresent=true;
	debug=false;
	changed=true;
	scopecom=new ScopeComm();
	if (!scopecom->opencom("COM3")){
		cout<<"Microscope NOT PRESENT"<<endl;
		isPresent=false;
		return;
	}
	cout<<"Microscope ready"<<endl;
	scopecom->write("FPZM5,0\r");//disable software limits
	//scopecom->write("FPZM5,0\r");

	//double startpos;
	//startpos=getPos();
	bool workPosition=true;
	//send calibration command
	//double ret=writeAndGetReturnedPosition("FPZpi\r");
	//double RealPos=getPos()+ret;
	//cout<<"Real Starting Position is "<<RealPos<<"    Offset is "<<ret<<endl;
	scopecom->write("FPZGI2,1\r");
	//scopecom->write("FPZGI2,1\r");
	cout<<"Return to original z-position (y or n)?"<<endl;
	char c;
	cin>>c;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (c!='y'){
		workPosition=false;
	}
	
//	wait();
//	cout<<"After Calibration position is: "<<getPos()<<endl;
	//scopecom->write("FPZP0\r");
	//scopecom->write("FPZQ0\r");
	//Timer::wait(1000);
//	cout<<"After 1 second position is: "<<getPos()<<endl;
	//Timer::wait(1000);
//	cout<<"After 2 seconds position is: "<<getPos()<<endl;
	
	//scopecom->write("HPDL0\r");
	//cout<<"After reference zero position is: "<<getPos()<<endl;
	//Sleep(5000);
	//move(0);
	//wait();
	//
	//scopecom->write("FPZpi\r");
	//char x[16];
	//int ret=scopecom->read(x,8);
	//x[8]=0;
	//cout<<"Focus reference position:"<<x<<endl;
	//cout<<"Focus Start Position:"<<getPos()<<endl;
	
	//move(100);
	//wait();
	//cout<<"Moved to 100 Position is now "<<getPos()<<endl;
	//set default velocity
	vel="48";//4.5mm/sec

	scopecom->write("FPZV"+vel+"\r");
	//set default acceleration
	acc="12"; //.018um/msec^2
	scopecom->write("FPZA"+acc+"\r");
	scopecom->write("FPZM5,1\r");
	sentHalogenBinary=0;
	desiredHalogenBinary=volts2Int(.4);
	halogenOn=false;
	halogenOn=true;
	HalogenOff();
	
	
	//openShutter();
	//Timer::wait(2000);
	waitCalibration();
	//cout<<"Measurement Origin is"<<writeAndGetReturnedPosition("FPZo\r")<<endl;
	//Sleep(3000);
	//cout<<"Measurement Origin is"<<writeAndGetReturnedPosition("FPZo\r")<<endl;
	setMaxZ(softLimitZ);
	//move(0);
	//scopecom->write("FPZQ0\r");//this is a permanent zero unlike the FPZO command
	if (workPosition) {
		//if (RealPos>0) move(RealPos);
		//else move(20);
		scopecom->write("FPZWU\r"); 
		wait();
		cout<<"After return to work state position is: "<<getPos()<<endl;
	}
	else {
		move(20);
	}
	//if (this->getPos()<=0) move(20);
	//cout<<"Measurement Origin is"<<writeAndGetReturnedPosition("FPZo\r")<<endl;
	//wait(false);
	//cout<<"GetPosition is:"<<getPos()<<endl;
	//scopecom->write("FPZO000000\r");
	//cout<<"Measurement Origin is"<<writeAndGetReturnedPosition("FPZo\r")<<endl;
	//cout<<"GetPosition is:"<<getPos()<<endl;
	//scopecom->write("FPZO00FFFF\r");
	//cout<<"GetPosition is:"<<getPos()<<endl;
	//this can only be done after calibration is complete otherwise it will be wrong
	//SidePort(Axiovert::SIDEPORTCAM);	
	//SetObj(OBJ_20);
	*/
}

Axiovert::~Axiovert(){
	CheckExists()
	StartErrChk
	stop();
	closeShutter();
	//disable limits
	focus->EnableSWLimit(true,false);
	focus->EnableSWLimit(false,false);
	halogen->SetOnOff_2(MTBOnOff_Off,MTBCmdSetModes_Default);
	((IMTBChangerPtr) RLshutter)->SetPosition_2(MTBOnOff_Off,MTBCmdSetModes_Default);
	
	//unadvise server sinks
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
	sideportEvents.DispEventUnadvise(m_COMsideportEvents);
	sideportBaseEvents.DispEventUnadvise(m_COMsideportBaseEvents);
	RLshutterEvents.DispEventUnadvise(m_COMRLshutterEvents);
	RLshutterBaseEvents.DispEventUnadvise(m_COMRLshutterBaseEvents);

	// if connection is available: close it
	if (m_conn != NULL )
		m_conn->Close();

	::_Module.Term();
	// be a good citizen and clean up COM
	CoUninitialize();
	EndErrChk
	/*old CAN commands
	stop();
	scopecom->write("FPZM5,0\r");
	SetEvent(scopecom->hCommAccess);
	scopecom->closecom();
	delete scopecom;
	*/
}
void Axiovert::DisplayError(_com_error* e){
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
} 
void Axiovert::waitAxiovert(MTBApi::IMTBBasePtr& a){
	CheckExists()
	bool b;
	//int b;
	StartErrChk
	b=a->WaitReady(10000);
	EndErrChk
	if (!b) {
		string ID=((MTBApi::IMTBComponentPtr)a)->ID;
		logFile.write("Error: Wait timed out for "+ID,true);
	}
}
#define WAIT(device) ((MTBApi::IMTBBasePtr) device)->WaitReady(60000)
string Axiovert::getConfig(){
	CheckExists("")
	string s(config->ReadActiveConfiguration());
	int begin=s.find_first_of("\"");
	int end=s.find_first_of("\"",begin+1);
	return s.substr(begin+1,end-begin-1);
}
vector<Objective> Axiovert::getObjectives(){
	vector<Objective> objectives;
	CheckExists(objectives)
	IMTBObjectivePtr obj;
	double DOF;
	bool isOil=false;
	int startObj=0;
	int totalObj=objectiveTurret->GetElementCount();
	if (objTurretCodedPosition!=-1){
		startObj=objectiveTurret->GetPosition()-1;
		totalObj=startObj+1;
	}
		
	for(int i=startObj;i<totalObj;i++){
		obj=objectiveTurret->GetElement(i);
		if (!obj) continue;
		switch(int(obj->GetMagnification())){
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
		objectives.push_back(Objective(obj->GetMagnification(),DOF,obj->GetWorkingDistance(),isOil,&cont.objTurret,i+1,string(((IMTBIdentPtr)obj)->GetName())));
		logFile.write(string("Found Objective: ")+string(((IMTBIdentPtr)obj)->GetName()),true);
	}
	return objectives;
}

vector<Selection> Axiovert::getCubes(){
	vector<Selection> cubes;
	CheckExists(cubes)
	IMTBIdentPtr cube;
	for(long i=0;i<reflectorTurret->GetElementCount();i++){
		cube=(IMTBIdentPtr) reflectorTurret->GetElement(i);
		if (!cube)
			continue;
		cubes.push_back(Selection(&cont.reflector,i+1,string(cube->GetName())));
		logFile.write(string("Found Cube: ")+string(cube->GetName()),true);
	}
	return cubes;
}

bool Axiovert::isManualLamp(){
	CheckExists(false)
	return isManLamp;
	
}

void Axiovert::setMaxZ(double z){
	CheckExists()
	SetLimits(0,z);
}
double Axiovert::getMaxZ(){
	CheckExists(0)
	return focus->GetSWLimit(true,micron);
}

/*legacy function for CAN commands
double Axiovert::writeAndGetReturnedPosition(string cmd){
	char x[16];
	int i=scopecom->writeAndRead(cmd,x,8);
	x[i]=0;
	sscanf(x+2,"%X",&i);
	string s=string(x+2);
	i=(i<<(32-s.length()*4))>>(32-s.length()*4);
	return (double) (i*.025);
}
void Axiovert::wait(){
	CheckExists()
	char x[16],y[16];
	int i=scopecom->writeAndRead("FPZt\r",x,3);
	x[i]=0;
	int j=scopecom->writeAndRead("HPSb1\r",y,5);
	y[j]=0;
	while(toInt(string(x+2))!=0 || toInt(string(y+2))!=0){
		//if (toInt(string(x+2))==0 && toInt(string(y+2))==1) break; //This is to bypass the reflector wait temporarily
		i=scopecom->writeAndRead("FPZt\r",x,3);
		x[i]=0;
		j=scopecom->writeAndRead("HPSb1\r",y,5);
		y[j]=0;
	} 
}
void Axiovert::waitCalibration(){
	char x[16];
	int i=scopecom->writeAndRead("FPZw\r",x,4);
	int j,k;
	x[i]=0;
	string s=string(x+2);
	while(s=="11"||s=="12"||s=="13"||s=="14"){
		i=scopecom->writeAndRead("FPZw\r",x,4);
		x[i]=0;
		s=string(x+2);
	} 
}*/
bool Axiovert::go(double zspeed){
/* old CAN commands
	CheckExists(false)
	char x[16];
	int i=0.5+16.777216*(zspeed/.025);
	sprintf(x,"%X",i);
	//set speed
	scopecom->write("FPZG"+string(x)+"\r");
	//start movement and get constant velocity start position
	scopecom->writeAndRead("FPZs+\r",x,1);
	return true;
	*/
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
	((IMTBBasePtr)focus)->StopMonitoring((BSTR)m_ID);
	::Timer t;
	double start=this->getPos();
	moveSpeedSettledPosition=start;
	this->move(start-zspeed*0.3);//.3s delay  THIS IS BAD FOR PHOTOBLEACHING, NEED EVENTS
	WAIT(focus);
	double begin=this->getPos();
	double point;
	double time0,time1, time2;
	//int counter1=1;
	//int counter2=1;
	//t.startTimer();
	((IMTBMovePtr)focus)->Move(zspeed,micronpersec);
	//time0=t.getTime();
	while(true){
		point=this->getPos();
		if (point!=begin) break;
	//	counter1++;
	}
	//time1=t.getTime();
	begin=point;
	while(true){
		point=this->getPos();
		if (point!=begin) break;
	//	counter2++;
	}
	//t.stopTimer();
	//time2=t.getTime();
	//logFile.write("Probed the position "+toString(counter1)+" times and "+toString(counter2)+" times.",true,"Axiovert::go");
	//logFile.write("Time spent on go/Move is "+toString(time0)+" and "+toString(time1)+" and "+toString(time2),true,"Axiovert::go");
	if (start<point) {
		logFile.write("Focus move error: already above start point",true);
		logFile.write("Target speed was "+toString(zspeed)+". Actuall speed was "+toString(point)+"-"+toString(start-zspeed*3)+"/"+toString(t.getTime()*0.001)+"="+toString((point-(start-zspeed*1.5))/(t.getTime()*0.001)),true,"Axiovert::go");
	}
	::Timer::wait(1000.0*(start-point)/zspeed);
	((IMTBBasePtr)focus)->StartMonitoring((BSTR)m_ID);
	EndErrChk
	return true;
}


double Axiovert::getStartPosition(){
	/*old CAN commands
	char x[8];
	int r=scopecom->read(x,7);
	x[r]=0;//null terminate string
	int i=0;
	sscanf(x+1,"%X",&i);
	string s=string(x+1);
	i=(i<<(32-s.length()*4))>>(32-s.length()*4);
	double ret= (i*.025);
	return ret;
	*/
	CheckExists(0)
	return moveSpeedSettledPosition;
}
void Axiovert::stop(){
	CheckExists()
	StartErrChk
	((IMTBMovePtr)focus)->Stop();
	EndErrChk
	/*old CAN commands
	scopecom->write("FPZS\r");//stop immediately not smoothly;
	scopecom->write("FPZV"+vel+"\r");//return to default velocity
	*/
}
/*
void Axiovert::inc(double um){
	CheckExists()
	char x[10];
	int i;
	if (um<0){
		i=(-0.5+um/.025);
	}else i=(0.5+um/.025);
	sprintf(x,"%06X",(int) (i&0x00FFFFFF));
	scopecom->write("FPZB"+string(x)+"\r");
}
*/
//this function may need to check how far the move is and use fine focus accordingly
//we don't want the check to take too long though;
void Axiovert::move(double zpos){
	CheckExists()
	StartErrChk
	//This is used to switch to measurement position because the absolute position calibration has problems
	//double origin=focus->GetMeasurementOrigin(micron);
	((IMTBContinualPtr)focus)->SetPosition_2(zpos,micron,MTBCmdSetModes_Default);
	EndErrChk	
	/*old CAN commands
	this->zpos=zpos;
	char x[10];
	int i;
	//To avoid backlash problem, always move to zpos-1, then move up 1 um to zpos.
	if (zpos<0){
		i=(-0.5+(zpos-1)/.025);
	}else i=(0.5+(zpos-1)/.025);
	sprintf(x,"%06X",(int) (i&0x00FFFFFF));
	scopecom->write("FPZT"+string(x)+"\r");
	if (zpos<0){
		i=(-0.5+zpos/.025);
	}else i=(0.5+zpos/.025);
	sprintf(x,"%06X",(int) (i&0x00FFFFFF));
	scopecom->write("FPZT"+string(x)+"\r");
	*/
}
double Axiovert::getPos(){
	CheckExists(-1)
	return ((IMTBContinualPtr)focus)->GetPosition(micron);
	//return focus->GetMeasurementPosition(micron);//This is included because we have a problem calibrating the absolute position.
	/*old CAN commands
	double ret;
	char x[16];
	int r=scopecom->writeAndRead("FPZp\r",x,8);
	x[r]=0;
	int i;
	sscanf(x+2,"%X",&i);
	string s=string(x+2);
	i=(i<<(32-s.length()*4))>>(32-s.length()*4);
	ret= (i*.025);
	if (debug){
		cout<<"internal difference is "<<ret-zpos<<endl;
	}
	return ret;
	*/
}

//pos will have the desired position, if the current position is within some tolerance of that position (maybe should be exact)
void Axiovert::SetLimits(double min,double max){
	CheckExists()
	focus->SetSWLimit(true,max,micron);
	focus->SetSWLimit(false,min,micron);
	/*
	char x[10];
	int i;
	if (min<0){
		i=(-0.5+min/.025);
	}else i=(0.5+min/.025);
	sprintf(x,"%06X",(int) (i&0x00FFFFFF));
	scopecom->write("FPZL"+string(x)+"\r");
	if (max<0){
		i=(-0.5+max/.025);
	}else i=(0.5+max/.025);
	sprintf(x,"%06X",(int) (i&0x00FFFFFF));
	scopecom->write("FPZU"+string(x)+"\r");
	*/
}

void Axiovert::SetTurret(int i){//1 is pinkel 0 is BF
	CheckExists()
	StartErrChk
	reflectorTurret->SetPosition_2(i,MTBCmdSetModes_Default);
	EndErrChk
	/*old CAN commands
	openShutter();
	char buf[16];
	scopecom->write("HPCR1,"+string(itoa(i,buf,10))+"\r");
	*/
}
int Axiovert::getObj(){
	return objectiveTurret->GetPosition();
	/*old CAN commands
	CheckExists(0)
	char x[16];
	int i=scopecom->writeAndRead("HPCr2,1\r",x,3);
	for(int count=0;i!=3&&count<3;count++)
		i=scopecom->writeAndRead("HPCr2,1\r",x,3);
	if(i!=3)
		clickAbort();
	x[i]=0;
	return toInt(string(x+2));
	*/
}

void Axiovert::SetObj(int i){
	/*old CAN commands
	CheckExists()
	scopecom->write("HPCR2,"+toString(i)+"\r");
	*/
	if (objTurretCodedPosition==i) return;
	if (objTurretCodedPosition!=-1) {
		logFile.write("Error: Move to a different objective is not allowed. Objective Motor is disabled",true);
		return;
	}
	StartErrChk
	objectiveTurret->SetPosition_2((short)i,MTBCmdSetModes_Default);
	EndErrChk
}
//legacy functions 
/*
int Axiovert::getTotalMag(){
	CheckExists(1)
	int mag;
	int obj=getObj();
	switch(obj){
		case OBJ_10:
			mag=10;
			break;
		 
		case OBJ_20:
			mag=20;
			break;
			  
		case OBJ_40:
			mag=40;
			break;
			  
		case OBJ_63:
			mag=63;
			break;
		default: return 1;//assert(false);
	}
	if (getOptovar()) return mag*1.6;
	else return mag;
}

int Axiovert::volts2Int(double volts){
	CheckExists(0)
	int i=1+(volts-.4)*255/(12.2-.4);//no rounding cause Zeiss decided not too....and why did they doe this?
	if (i>255) {
		logFile.write("Scope: Desired Voltage: "+toString(volts)+" too High. Setting to 12.2V",DEBUGAXIOVERT);	
		i=255;
	}
	if (i<0) {
		i=0;
		logFile.write("Scope: Desired Voltage: "+toString(volts)+" too Low. Setting to 0.4V",DEBUGAXIOVERT);
	}
	return i;
}

double Axiovert::int2Volts(int i){
	CheckExists(0)
	static int count=0;
	if(i<0 || i>255)
		clickAbort();
	count=0;
	return (i*((12.2-.4)/255)+.4);
}

void Axiovert::enableLampController(){
	CheckExists()
	scopecom->write("HPCT13,1\r");
}
void Axiovert::adjustVoltage(){
	CheckExists()
	Timer t(true);
	if (changed){
		sentHalogenBinary=desiredHalogenBinary-4;
		SetHalogenVoltageBinary(sentHalogenBinary);
		//Timer::wait(4000);
		int realHalogenBinary=GetHalogenVoltageBinary();

		while(toString(int2Volts(desiredHalogenBinary),false,2)!=toString(int2Volts(realHalogenBinary),false,2)){
			if (sentHalogenBinary<=0 || sentHalogenBinary>=255) {assert(sentHalogenBinary==0 ||sentHalogenBinary==255); break;}
			if (realHalogenBinary<desiredHalogenBinary) sentHalogenBinary++;
			else sentHalogenBinary--;
			SetHalogenVoltageBinary(sentHalogenBinary);
			Timer::wait(500);//30ms to update voltage? needs to be longer?
			//int tempBin=GetHalogenVoltageBinary();
			//while(GetHalogenVoltageBinary()!=tempBin) {Sleep(2000);tempBin=GetHalogenVoltageBinary();Sleep(2000);}//wait for stabilization
			realHalogenBinary=GetHalogenVoltageBinary();
		}
		logFile.write("Scope: Adjusted Lamp Voltage in "+toString(t.getTime())+"ms. Desired Voltage is "+toString(int2Volts(desiredHalogenBinary))+"("+toString(desiredHalogenBinary)+") Sent Voltage is "+toString(int2Volts(sentHalogenBinary))+"("+toString(sentHalogenBinary)+") Actual Voltage is "+toString(GetHalogenVoltage())+"("+toString(GetHalogenVoltageBinary())+")",DEBUGAXIOVERT);
	}
}
*/
void Axiovert::waitHalogen(){
	CheckExists()
	halogenTimer.waitAfterLastStart(4000);
}
/*

void Axiovert::SetHalogenVoltageBinary(int halogenBinary){
	CheckExists()
	scopecom->write("HPCV1,"+toString(halogenBinary)+"\r");
}

int Axiovert::GetHalogenVoltageBinary(){
	CheckExists(0)
	char x[16];
	int r=scopecom->writeAndRead("HPCv5\r",x,5);
	x[r]=0;
	int i=toInt(string(x+2));
	static int count=0;
	if (i<0 || i>255)
	{
		count++;
		if(count>5)
			clickAbort();
		return GetHalogenVoltageBinary();
	}
	count=0;
	return toInt(string(x+2));
}

*///all of the above are old
double Axiovert::GetHalogenVoltage(){
	CheckExists(0)
	double v;
	StartErrChk
	v= ((IMTBContinualPtr)halogen)->GetPosition(volt);
	EndErrChk
	return v;
	//return int2Volts(GetHalogenVoltageBinary());
}

//just use HalogenOn
/*void Axiovert::SetHalogenVoltage(double volts){
	CheckExists()
	StartErrChk
	((IMTBContinualPtr)halogen)->SetPosition_2(volts,volt,MTBCmdSetModes_Default);
	EndErrChk
	/*old CAN commands	
	if (volts<.9) {
		logFile.write("Scope: Voltage Not Set: Halogen Voltage cannot be controlled down to that level ("+toString(volts)+") Try it manually and you will see it is impossible.",DEBUGAXIOVERT);
		changed=false;	
		return;
	}
	desiredHalogenBinary=volts2Int(volts);
	
	
	if (volts==-1) return;
	string old=toString(int2Volts(halogenBinary),false,1);
	string cur=toString(volts,false,1);
	if (old==cur) {
		logFile.write("Scope: Desired voltage is same as current voltage. Ignoring.",DEBUGAXIOVERT);
		return;
	}
	if (volts<.9) {
		logFile.write("Scope: Voltage Not Set: Halogen Voltage cannot be controlled down to that level ("+toString(volts)+") Try it manually and you will see it is impossible.",DEBUGAXIOVERT);
		return;
	}

	
	if (halogenOn) HalogenOn(-1,wait);
	
}
*/

//will this fix display issues? Zeiss says NO! We can figure something out
void Axiovert::unlockDisplay(){
	CheckExists()
	scopecom->write("HPDL0\r");
}

void Axiovert::HalogenOff(bool wait){
	CheckExists()
	StartErrChk
	if (wait){
		halogen->SetOnOff_2(MTBOnOff_Off,MTBCmdSetModes_Synchronous);
		halogenTimer.resetTimer();
		halogenTimer.startTimer();
		waitHalogen();
	}else{
		halogen->SetOnOff_2(MTBOnOff_Off,MTBCmdSetModes_Default);
		halogenTimer.resetTimer();
		halogenTimer.startTimer();
	}
	EndErrChk
	/*old CAN commands
	CheckExists()
	scopecom->write("HPCT8,1\r");
	//if (!halogenOn) return;
	
	halogenOn=false;
	halogenTimer.restart();
	*/
}
void Axiovert::HalogenOn(double volts, bool wait){
	CheckExists()
	voltage=volts;
	StartErrChk
	//if (abs(volts-this->GetHalogenVoltage())<.05) return;
	halogen->SetOnOff_2(MTBOnOff_On,MTBCmdSetModes_Default);
	halogenTimer.resetTimer();
	halogenTimer.startTimer();
	if (wait){
		((IMTBContinualPtr)halogen)->SetPosition_2(volts,volt,MTBCmdSetModes_Synchronous);
		waitHalogen();
	}else{
		((IMTBContinualPtr)halogen)->SetPosition_2(volts,volt,MTBCmdSetModes_Default);
	}
	EndErrChk
	/*old CAN commands
	scopecom->write("HPCT8,0\r");//halogen On
	if (this->volts2Int(volts)==desiredHalogenBinary){
		if (!halogenOn){
		SetHalogenVoltageBinary(sentHalogenBinary);
		changed=false;
		halogenTimer.restart();
		}
	}else {
		changed=true;
		SetHalogenVoltage(volts);
		adjustVoltage() ;
	}
	halogenOn=true;
	/*
	if (halogenOn && !changed) {
		if (wait) this->wait();
		return;//lamp already on and voltage unchanged
	}
	//scopecom->write("HPCT8,0\r");//halogen On
	//SetHalogenVoltageBinary(this->halogenBinary);
	halogenOn=true;
	if (!changed) { //lamp not on but voltage not changed
			halogenTimer.restart();
			if (wait) this->wait();
			return;
	}
	if (volts!=-1) return SetHalogenVoltage(volts,wait);
	
	waitHalogen();
	if (wait) this->wait();
	*/
}

void Axiovert::SidePort(int pos){
	CheckExists()
	StartErrChk
	switch(pos){
		case 1://eye piece
			sideport->SetPosition_2(1,MTBCmdSetModes_Default);
			break;
		case 2://camera (left)
			sideport->SetPosition_2(2,MTBCmdSetModes_Default);
			break;
		case 3://50/50
			sideport->SetPosition_2(3,MTBCmdSetModes_Default);
			break;
		default:
			cout<<"Axiovert: Light Path Position "<<pos<<" is not valid"<<endl;
	}
	EndErrChk
	/*old CAN commands
	char buf[16];
	scopecom->write("HPCR39,"+string(itoa(pos,buf,10))+"\r");
	*/	
}

int Axiovert::getSidePort(){
	CheckExists(0)
	return sideport->GetPosition();	
	/*old CAN commands
	char x[16];
	int i=scopecom->writeAndRead("HPCr39,1\r",x,3);
	assert(i==3);
	x[i]=0;
	int pos=toInt(string(x+2));
	return pos;
	*/
}
void Axiovert::SetOptovar(int opt){
	CheckExists()
	StartErrChk
	optovarTurret->SetPosition_2(opt,MTBCmdSetModes_Default);
	EndErrChk
	/*old CAN commands
	if (opt){
	scopecom->write("HPCR36,"+toString(2)+"\r");
	}else{
	scopecom->write("HPCR36,"+toString(1)+"\r");
	}
	*/
}

int Axiovert::getTurret(){
	CheckExists(0)
	int turret;
	StartErrChk
	turret= reflectorTurret->GetPosition();
	EndErrChk
	return turret;
	/*old CAN commands
	char x[16];
	//wait();
	int i=scopecom->writeAndRead("HPCr1,1\r",x,3);
	assert(i==3);
	x[i]=0;
	int pos=toInt(string(x+2));
	return pos;	
	*/
}

bool Axiovert::getOptovar(){
	CheckExists(0)
	if (optovarTurret->GetPosition()!=0) return true;
	else return false;
	/*old CAN commands
	char x[16];
	//wait();
	int i=scopecom->writeAndRead("HPCr36,1\r",x,3);
	assert(i==3);
	x[i]=0;
	int pos=toInt(string(x+2));
	if (pos==2) return true;
	else return false;
	*/
}
void Axiovert::closeShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr) RLshutter)->SetPosition_2(1,MTBCmdSetModes_Default);
	EndErrChk
	/*old CAN commands
	scopecom->write("HPCK1,1\r");
	*/
}

void Axiovert::openShutter(){
	CheckExists()
	StartErrChk
	((IMTBChangerPtr) RLshutter)->SetPosition_2(2,MTBCmdSetModes_Default);
	EndErrChk
	/*old CAN commands
	scopecom->write("HPCK1,2\r");
	*/
}
#endif