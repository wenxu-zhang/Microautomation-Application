// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, July 10, 2012</lastedit>
// ===================================
// Controller.cpp : Defines the entry point for the console application.
//virtual classes 
#include "Controller.h" 
#include "Camera.h"
#include "XYStage.h"
#include "ZStage.h"

//device classes
#include "AndorCam.h"
#include "Ludl.h"
#include "Axiovert.h"
#include "Lambda10_3.h"
#include "Pump.h"
#include "TEModule.h"
#include "DG.h"
#include "MultiCam.h"
//helper classes
#include "ProtocolEric.h"
#include "ProtocolWalsh.h"
#include "ProtocolYing_Ja.h"
#include "Channel.h"
#include "Magnification.h"
#include "Optovar.h"
#include "Objective.h"
#include "Light.h"
#include "Scan.h"
#include "ScanSpots.h"
#include "Eye.h"
#include "Chamber.h"

#include "Timer.h"
#include "Utils.h"
#include <math.h>
#include <vector>
#include <sstream>
#include <process.h>
#include <windows.h>
#include <Aclapi.h>
#include <time.h>


#include <algorithm>
/////////////////////////////////////////////////////////////
///INITIALIZATION CODE///////////////////////////////////////
/////////////////////////////////////////////////////////////
HWND console=initializeConsoleWindow();
HANDLE abortEvent=CreateEvent(NULL,true,false,NULL);
//HANDLE hInput=GetStdHandle(STD_INPUT_HANDLE);
int LUDLCOM=5;
//HANDLE inputWaitEvents[2]={hInput,abortEvent};
string user(::getUser());
string WORKINGDIR(string(DEFAULTWORKINGDIR)+user);
Record logFile(string(WORKINGDIR)+"\\default\\"+"logFile.txt");//default log file
Record focusLogFile(string(WORKINGDIR)+"\\default\\"+"focusLog.txt",0,logFile.swatch);
HANDLE AndorSelectCamMutex=CreateMutex(NULL,false,NULL);//synchronize andor driver calls with mutex
//int andorCamInit=Andor885::init();
Controller cont;
Chamber currentChamber;
bool internalAbort=false;
int protocolIndex=-1;
std::vector<std::string> protocolFiles;
customProtocol scan;
//HANDLE AndorSelectCamMutex=CreateEvent(NULL,true,true,NULL);//synchronize andor driver calls with event
/////////////////////////////////////////////////////////////
///END INITIALIZATION CODE///////////////////////////////////
/////////////////////////////////////////////////////////////

/*
void Controller::takePicture(AcquisitionChannel* ac,string fileName, Focus* f,bool removeReagentTemporarily,bool wait4Pump,bool scaleBar){
	if (removeReagentTemporarily) pmp.pauseReaction();
	if (wait4Pump) pmp.wait();
	if (f){
		if  (f->a.m!=ac->m) logFile.write("Controller: Focusing with a different objective is not recommended",DEBUGCONTROLLER);
		f->getFocus();
	}
	ac->takePicture(fileName);
	if (removeReagentTemporarily) pmp.resumeReaction();	
}*/


Controller::Controller():workingDir(WORKINGDIR+"\\default\\"),
#ifdef AXIOVERT
//INITIALIZATION LIST
daqusb("Dev1"),
axio(),
rl(&axio),
hal(&axio),
reflector(&axio),
outPort(&axio),
objTurret(&axio),
optTurret(&axio),
ld(),
fw(&ld,true),
dg5(&daqusb,0,1),
focus(new AxiovertZ(&axio)),
stg(new Ludl()),//this use of copy constructor should be fine
pmp(),
te(5),
currentChannel((Channel*)NULL,NULL,NULL,AcquisitionParameters(.01,4,1,ImageRegion(0),false),1.4),
currentFocus(currentChannel,20,.5)
{//CONSTRUCTOR BODY
	cameras.push_back(new Andor885(&daqusb,1));
	cameras.push_back(new Eye());
	outPorts.push_back(OutPort(Selection(&outPort,2,"Cam"),cameras[0]));
	outPorts.push_back(OutPort(Selection(&outPort,1,"Oculars"),cameras[1]));
	outPorts.push_back(OutPort(Selection(&outPort,3,"Cam+Oculars"),cameras[0]));

	//TEMPORARY OBJECTS TO CONSTRUCT ALL MAGNIFICATIONS AND CHANNELS
	//reflector positions
	Selection pinkel(&reflector,5,"pinkel");
	Selection fitc(&reflector,1,"fitc");
	Selection cy3(&reflector,2,"cy3");
	Selection cy35(&reflector,3,"cy35");
	Selection cy7(&reflector,4,"cy7");

	//emfilter positions
	Selection openEm(&fw,0,"Open");
	Selection FITCEm(&fw,1,"FITC");
	Selection Cy3Em(&fw,2,"Cy3");
	Selection Cy5Em(&fw,3,"Cy5");
	Selection blockedEm(&fw,9,"Blocked");

	//light positions
	
	Light halogen(&hal,0,"Halogen");//no position for axiovert because it is not motorized
	//Light* blocked=new Light(dg5,0,"Blocked");
	Light open(&dg5,1,"Open");
	Light DAPI(&dg5,2,"DAPI");
	Light FITC(&dg5,3,"FITC");
	Light Cy3(&dg5,4,"Cy3");
	Light Cy5(&dg5,5,"Cy5");

	//objectives
	Objective o10(10,2,2000,false,&objTurret,1,"10xNA0.45");
	Objective o20(20,.5,550,false,&objTurret,2,"20xNA0.8");
	Objective o40(40,1,2900,false,&objTurret,3,"40xNA0.6");
	Objective o40oil(40,.5,180,true,&objTurret,6,"40xOilNA1.3");//not sure about these specs
	Objective o63(63,.25,180,true,&objTurret,5,"63xOilNA1.4");

	//optovars
	Optovar opt1(1,&optTurret,1,"1x");
	Optovar opt2(1.6,&optTurret,2,"1.6x");
	//END TEMPORARY OBJECTS

	//magnifications
	mags.push_back(Magnification(o10,opt1));
	mags.push_back(Magnification(o10,opt2));
	mags.push_back(Magnification(o20,opt1));
	mags.push_back(Magnification(o20,opt2));
	mags.push_back(Magnification(o40,opt1));
	mags.push_back(Magnification(o40,opt2));
	mags.push_back(Magnification(o40oil,opt1));
	mags.push_back(Magnification(o40oil,opt2));
	mags.push_back(Magnification(o63,opt1));
	mags.push_back(Magnification(o63,opt2));

	//create channels vector
	/*0*/channels.push_back(Channel("Transmitted Light", "Bright field excitation from halogen lamp",halogen,pinkel));
	/*1*/channels.push_back(Channel("DAPI Pinkel","DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)",DAPI,pinkel));
	/*2*/channels.push_back(Channel("FITC Pinkel","FITC, Alexa 488             Ex(485nm) Em(520nm)",FITC,pinkel));
	/*3*/channels.push_back(Channel("Cy3 Pinkel","Cy3, TRITC, Alexa 568 Ex(560nm) Em(606nm)",Cy3,pinkel));
	/*4*/channels.push_back(Channel("Cy5 Pinkel","Cy5, Alexa 680, Alexa 660   Ex(650nm) Em(699nm)",Cy5,pinkel));
	/*5*/channels.push_back(Channel("FITC Zeiss","Zeiss' FITC filter set (#10)",open,fitc));
	/*6*///channels.push_back(Channel("DAPI Semrock","DAPI Semrock",open,dapi));
	/*3*/channels.push_back(Channel("Cy3 Zeiss","Cy3 Zeiss (#26)",open,cy3));
	/*7*/channels.push_back(Channel("Cy3.5 Semrock","TexasRed",open,cy35));
	/*7*/channels.push_back(Channel("Cy7 Zeiss","Cy7 filter set (Chroma)",open,cy7));
	/*8*///channels.push_back(Channel("Transfluosphere 488/605","FITC ex. Cy3 em.",open,fitc));
	/*9*///channels.push_back(Channel("Transfluosphere 488/645","FITC ex. Cy5 em.",open,fitc));
	/*10*///channels.push_back(Channel("Ultra Violet","For Photocleaving",open,UV));

	currentChannel=AcquisitionChannel(BF,OUT_CAM,MAG_20x,AcquisitionParameters(.05,15,1),1.35);
	currentFocus=Focus(currentChannel,currentChannel().m->obj.dof*20,currentChannel().m->obj.dof);
#endif
#ifdef OBSERVER
//INITIALIZATION LIST
//initialize these to -1 and let them be defined by the active configuration
BF(-1),UV(-1),DAPI(-1),DAPILASER(-1),FITC(-1),CY3(-1),CY35(-1),CY5(-1),CY55(-1),QDOT705(-1),CY7(-1),
FRET405488(-1),FRET425594(-1),FRETFITCCy35(-1),FRETCy3Cy5Sem(-1),FRETCy3Cy5Thor(-1),FRETCy3Cy5(-1),TIRF405(-1),TIRF488(-1),TIRF532(-1),TIRF642(-1),TIRF660(-1),
EPI405(-1),EPI473(-1),EPI532(-1),EPI642(-1),EPI660(-1),BFPINKEL(-1),
DAPIPINKEL(-1),FITCPINKEL(-1),CY3PINKEL(-1),CY5PINKEL(-1),
MAG_10x(-1),MAG_20x(-1),MAG_40x(-1),MAG_40xOil(-1),MAG_63xOil(-1),MAG_100xOil(-1),
daqpci("PCI6733"),
axio(),
rl(&axio),
tl(&axio),
hal(&axio),
reflector(&axio),
outPort(&axio),
objTurret(&axio),
optTurret(&axio),
df(&axio),
tirf(&daqpci,6),
dg5(&daqpci,0,1),//&dg5,1,1,&rl),
//ld(),
//fwA(&ld,true),
//daq("USB6251"),
violet(&daqpci,"L405nm"),//analog only
blue(&daqpci,"L488nm"),//analog only
g532(&daqpci,"L532nmGalvo"),//analog only
green(&daqpci,"L532nmDPSS",&g532),//analog only
red(&daqpci,"L642nm"),//analog only
red660(&daqpci,"L660nm"),//analog only
focus(new ObserverZ(&axio)),
stg(new Ludl()),//this use of copy constructor should be fine
pmp(1),
te(2),
currentChan(0),
currentFocus(AcquisitionChannel((Channel*)NULL,NULL,NULL,AcquisitionParameters(.01,4,1,ImageRegion(0)),1.4),20,.5),
pm(3,100,axio.objPM)//1,.5,.1,false,&objTurret,1,"Power Meter"))
{//CONSTRUCTOR BODY


	AndorCam* a0=new AndorCam(&daqpci,1,1,2);//this camera (right port reflected) image needs to be mirrored in the x and y direction
	AndorCam* a1=new AndorCam(&daqpci,2,2,1);//this camera (right port transmitted) image needs to be mirrored in the  y direction	
	AndorCam* a2=new AndorCam(&daqpci,3,0);//left port
	a0->waitSetup();
	a1->waitSetup();
	a2->waitSetup();
	this->cameras.push_back(a0);//right port reflected
	this->cameras.push_back(a1);//right port trans
	MultiCam* dualCam=new MultiCam(cameras);
	this->cameras.push_back(a2);//left port
	this->cameras.push_back(dualCam);
	this->cameras.push_back(new Eye());
	
	//order will determine preference for default outport which will be overridden if an alternate name is specified when creating the channel;
	outPorts.push_back(OutPort(Selection(&outPort,2,"CamRightRefl"),cameras[0]));
	outPorts.push_back(OutPort(Selection(&outPort,1,"CamLeft"),cameras[2]));
	outPorts.push_back(OutPort(Selection(&outPort,2,"CamRightTrans"),cameras[1]));
	outPorts.push_back(OutPort(Selection(&outPort,2,"CamDual"),cameras[3]));
	outPorts.push_back(OutPort(Selection(&outPort,3,"Oculars"),cameras[4]));
	outPorts.push_back(OutPort(Selection(&outPort,5,"Blocked"),cameras[4]));//baseport
	
	OUT_CAM=0;//single cam default should be reflected (mirror position on dual cam slider) since the other dual cam slider position should always have a FRET imaging cube. 
	OUT_CAM_LEFT=1;
	OUT_CAM_RIGHT=0;
	OUT_CAM_RIGHTTRANS=2;
	OUT_CAM_RIGHTREFL=0;
	OUT_CAM_DUAL=3;
	OUT_OCULARS=4;
	OUT_BLOCKED=5;
	OUT_FRONT=-1;
	OUT_CAM_OCULAR=-1;


	/*this section has been depricated. current Outport will be the default for the current channel
	currentOut=outPort.get();
	switch(currentOut){
		case 1:
			currentOut=OUT_CAM_LEFT;
			break;
		case 2:
			currentOut=OUT_CAM_RIGHT;
			break;
		case 3:
			currentOut=OUT_OCULARS;
			break;
		case 4:
			currentOut=OUT_FRONT;
			break;
		case 5:
			currentOut=OUT_BLOCKED;
			break;
		default:
			logFile.write(string("Error: outPort position not recognized. Value was ")+::toString(currentOut),true);
			currentOut=OUT_CAM;
	}
	if (currentOut==-1)
		currentOut=OUT_CAM;
		*/
	//TEMPORARY OBJECTS TO CONSTRUCT ALL MAGNIFICATIONS AND CHANNELS
	cout<<endl;
	logFile.write(string("Active Configuration: \"")+axio.getConfig()+"\"",true);
	int currentCube;
	vector<Selection> cubes=axio.getCubes(currentCube);

	//emfilter positions
	//Selection openEm(&fw,0,"Open");
	//Selection FITCEm(&fw,1,"FITC");
	//Selection Cy3Em(&fw,2,"Cy3");
	//Selection Cy5Em(&fw,3,"Cy5");
	//Selection blockedEm(&fw,9,"Blocked");

	//this could be stored in a config file and read in;
	double TIRFangle405obj100x=5;
	double TIRFangle488obj100x=5;
	double TIRFangle532obj100x=5;
	double TIRFangle642obj100x=5;
	double TIRFangle660obj100x=3.8;
	double TIRFangle405obj63x=5;
	double TIRFangle488obj63x=5;
	double TIRFangle532obj63x=5;
	double TIRFangle642obj63x=5;
	double TIRFangle660obj63x=3.8;
	double EPIangle405obj100x=0;
	double EPIangle488obj100x=0;
	double EPIangle532obj100x=0;
	double EPIangle642obj100x=0;
	double EPIangle660obj100x=-1;
	double EPIangle405obj63x=0;
	double EPIangle488obj63x=0;
	double EPIangle532obj63x=0;
	double EPIangle642obj63x=0;
	double EPIangle660obj63x=-1;
	
	Light l405(&violet,0,"405nm Laser");
//	Light argon488(&fwA,8,"Argon Ion 488");
	Light l488(&blue,0,"488nm Laser");
	Light l532(&g532,0,"532nm Laser");
	Light l642(&red,0,"642nm Laser");
	Light l660(&red660,0,"660nm Laser");

	//light positions
	Light halogen(&hal,0,"Halogen");//no position for axiovert because it is not motorized
	
	Light xcite(&rl,0,"Xcite");//all light from xcite

	Light dgOPEN(&dg5,1,"OPEN");
	Light dgDAPI(&dg5,2,"DAPI");
	Light dgFITC(&dg5,3,"FITC");
	Light dgCY3(&dg5,4,"Cy3");
	Light dgCY5(&dg5,5,"Cy5");

	lightSources.push_back(&violet);
	lightSources.push_back(&blue);
	lightSources.push_back(&green);
	lightSources.push_back(&g532);
	lightSources.push_back(&red);
	lightSources.push_back(&red660);
	lightSources.push_back(&hal);
	lightSources.push_back(&rl);

	//optovars
	Optovar opt1(1,&optTurret,1,"1x");
	Optovar opt2(1.6,&optTurret,2,"1.6x");	
	Optovar opt3(2.5,&optTurret,3,"2.5x");
	
	//END TEMPORARY OBJECTS
	int currentOpt=optTurret.get();
	//magnifications
	int currentMag=0;
	int magNum=0;
	Objective* obj=axio.getCurrentObjective();
	for (vector<Objective*>::iterator i=axio.objectives.begin();i != axio.objectives.end() ; i++){
		mags.push_back(Magnification(**i,opt1));
		switch ((int)(*i)->mag){
			case 10:
				cont.MAG_10x=magNum++;
				break;
			case 20:
				cont.MAG_20x=magNum++;
				break;
			case 40:
				if ((*i)->isOil)
					cont.MAG_40xOil=magNum++;
				else
					cont.MAG_40x=magNum++;
				break;
			case 63:
				cont.MAG_63xOil=magNum++;
				break;
			case 100:
				cont.MAG_100xOil=magNum++;
				break;
		}
		mags.push_back(Magnification(**i,opt2));
		magNum++;
		mags.push_back(Magnification(**i,opt3));
		magNum++;
		Objective o=**i;
		if (obj!=NULL && o.mag==axio.getCurrentObjective()->mag)
			currentMag=magNum-(3-currentOpt)-1;	
	}

	//create channels
	int chanNum=0;
	Light* allLight=NULL;
	if (axio.isManualLamp())
		allLight=&xcite;
	else if (dg5.isPresent)
		allLight=&dgOPEN;
	int j=0;
	for(vector<Selection>::iterator i=cubes.begin();i != cubes.end() ; i++,j++){//j increments by one but what if there is an empty cube?
		if (j==currentCube)
			currentChan=chanNum;
		if (i->name=="Semrock Pinkel"){
			channels.push_back(Channel("TransPinkel", "Bright field transmitted light from halogen lamp with Pinkel emission filter",halogen,*i));
			cont.BFPINKEL=chanNum++;
			if (dg5.isPresent){
				channels.push_back(Channel("DAPI-pinkel","DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)",dgDAPI,*i));
				cont.DAPIPINKEL=chanNum++;
				channels.push_back(Channel("FITC-pinkel","FITC, Alexa 488 Ex(485nm)  Em(520nm)",dgFITC,*i));
				cont.FITCPINKEL=chanNum++;
				channels.push_back(Channel("Cy3-pinkel","Cy3, Alexa ",dgCY3,*i));
				cont.CY3PINKEL=chanNum++;
				channels.push_back(Channel("Cy5-pinkel","Cy5, Alexa 680, Alexa 660  Ex(650nm) Em(699nm)",dgCY5,*i));
				cont.CY5PINKEL=chanNum++;
			}
			continue;
		}
		if (i->name=="BF"){
			channels.push_back(Channel("TransBF","Bright field transmitted light from halogen lamp with empty cube position",halogen,*i));
			cont.BF=chanNum++;
			continue;
		}
		if (allLight){
			if (i->name=="UV Cube"){
				channels.push_back(Channel("UV", "Photocleavage",*allLight,*i));
				cont.UV=chanNum++;
				continue;
			}
			//Single Band EPIFLUORESCENCE requires allLight
			if (i->name=="DAPI"){
				channels.push_back(Channel("DAPI","DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)",*allLight,*i));
				cont.DAPI=chanNum++;continue;				
			}if (i->name=="FITC"){
				channels.push_back(Channel("FITC","FITC, Alexa 488 Ex(485nm)  Em(520nm)",*allLight,*i));
				cont.FITC=chanNum++;continue;				
			}
			if (i->name=="Cy 3"){
				channels.push_back(Channel("Cy3","Cy3, Alexa 555, Alexa 546",*allLight,*i));
				cont.CY3=chanNum++;continue;				
			}
			if (i->name=="Cy 3.5"){
				channels.push_back(Channel("Cy3.5","Cy3.5, Texas Red, Alexa 568, Alexa 594 Ex(562nm) Em(624nm)",*allLight,*i));
				cont.CY35=chanNum++;continue;				
			}
			if (i->name=="Cy 5"){
				channels.push_back(Channel("Cy5","Cy5, Alexa 660  Ex(650nm) Em(699nm)",*allLight,*i));
				cont.CY5=chanNum++;continue;				
			}
			if (i->name=="Cy 5.5"){
				channels.push_back(Channel("Cy5.5","Cy5.5, Alexa 680",*allLight,*i));
				cont.CY55=chanNum++;continue;				
			}
			if (i->name=="Qdot705"){
				channels.push_back(Channel("Qdot705","Qdot705",*allLight,*i));
				cont.QDOT705=chanNum++;continue;				
			}
			if (i->name=="Cy 7"){
				channels.push_back(Channel("Cy7","Cy7, Alexa 750",*allLight,*i));
				cont.CY7=chanNum++;continue;				
			}
		}
		if (i->name=="FRET405"){
			Channel c("FRET405488","Alexa 405 -> Alexa 488",l405,*i,&tirf);
			c.addTIRFangle(TIRFangle405obj100x,Magnification::getObjective("100xOil"));
			c.addTIRFangle(TIRFangle405obj63x,Magnification::getObjective("63xOil"));
			vector<string> v(1,string("FRET_405_Donor"));v.push_back("FRET_488_Acceptor");
			c.addAlternateName(cont.outPorts.at(cont.OUT_CAM_DUAL),v);
			c.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTREFL),vector<string>(1,string("FRET_405_Donor")));
			c.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTTRANS),vector<string>(1,string("FRET_488_Acceptor")));
			
			c.denyOutport(cont.outPorts.at(cont.OUT_CAM_LEFT));
			channels.push_back(c);
			cont.FRET405488=chanNum++;
			
			Channel c2("FRET425594","ATTO 425 -> ATTO 594",l405,*i,&tirf);
			c2.addTIRFangle(TIRFangle405obj100x,Magnification::getObjective("100xOil"));
			c2.addTIRFangle(TIRFangle405obj63x,Magnification::getObjective("63xOil"));
			vector<string> v2(1,string("FRET_425_Donor"));v.push_back("FRET_594_Acceptor");
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_DUAL),v2);
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTREFL),vector<string>(1,string("FRET_425_Donor")));
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTTRANS),vector<string>(1,string("FRET_594_Acceptor")));
			
			c2.denyOutport(cont.outPorts.at(cont.OUT_CAM_LEFT));
			channels.push_back(c2);
			cont.FRET425594=chanNum++;continue;
		}
		if (i->name=="FRET488"){
			Channel c1("FRETFITCCy3.5","FITC->Cy3.5 or ATTO 488->ATTO 594",l488,*i,&tirf);
			c1.addTIRFangle(TIRFangle488obj100x,Magnification::getObjective("100xOil"));
			c1.addTIRFangle(TIRFangle488obj63x,Magnification::getObjective("63xOil"));
			vector<string> v(1,string("FRET_FITC_Donor"));v.push_back("FRET_Cy3.5_Acceptor");
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_DUAL),v);
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTREFL),vector<string>(1,string("FRET_FITC_Donor")));
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTTRANS),vector<string>(1,string("FRET_Cy3.5_Acceptor")));
			
			c1.denyOutport(cont.outPorts.at(cont.OUT_CAM_LEFT));
			channels.push_back(c1);
			cont.FRETFITCCy35=chanNum++;
			continue;
		}
		if (i->name=="FRET532"){
			Channel c1("FRETCy3Cy5","Cy3->Cy5 with Semrock Imaging Dichroic",l532,*i,&tirf);
			c1.addTIRFangle(TIRFangle532obj100x,Magnification::getObjective("100xOil"));
			c1.addTIRFangle(TIRFangle532obj63x,Magnification::getObjective("63xOil"));
			vector<string> v1(1,string("FRET_532_Sem_Donor"));v1.push_back("FRET_532_Sem_Acceptor");
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_DUAL),v1);
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTREFL),vector<string>(1,string("FRET_532_Sem_Donor")));
			c1.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTTRANS),vector<string>(1,string("FRET_532_Sem_Acceptor")));
			
			c1.denyOutport(cont.outPorts.at(cont.OUT_CAM_LEFT));
			channels.push_back(c1);
			cont.FRETCy3Cy5=chanNum++;
			/*Channel c2("FRETCy3Cy5Thor","Cy3->Cy5 with Thorlabs Imaging Dichroic",l532,*i,tirf);
			c2.addTIRFangle(TIRFangle532obj100x,Magnification::getObjective("100x"));
			c2.addTIRFangle(TIRFangle532obj63x,Magnification::getObjective("63x"));
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTREFL),vector<string>(1,string("FRET_532_Thor_Donor")));
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_RIGHTTRANS),vector<string>(1,string("FRET_532_Thor_Acceptor")));
			vector<string> v2(1,string("FRET_532_Thor_Donor"));v2.push_back("FRET_532_Thor_Acceptor");
			c2.addAlternateName(cont.outPorts.at(cont.OUT_CAM_DUAL),v2);	
			c2.denyOutport(cont.outPorts.at(cont.OUT_CAM_LEFT));
			channels.push_back(c2);
			cont.FRETCy3Cy5Thor=chanNum++;
			cont.FRETCy3Cy5=cont.FRETCy3Cy5Sem;//Default will be semrock imaging cube
			*/
			continue;
		}
		if (i->name=="TIRF405"){
			channels.push_back(Channel("EPI405","DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)",l405,*i,&tirf));
			channels.back().addTIRFangle(EPIangle405obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(EPIangle405obj63x,Magnification::getObjective("63xOil"));
			cont.EPI405=chanNum++;
			channels.push_back(Channel("TIRF405","DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)",l405,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle405obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle405obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF405=chanNum++;continue;
		}
		if (i->name=="TIRF488"){
			channels.push_back(Channel("EPI488","FITC, Alexa 488 Ex(485nm)  Em(520nm)",l488,*i,&tirf));
			channels.back().addTIRFangle(EPIangle488obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(EPIangle488obj63x,Magnification::getObjective("63xOil"));
			cont.EPI473=chanNum++;
			channels.push_back(Channel("TIRF488","FITC, Alexa 488 Ex(485nm)  Em(520nm)",l488,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle488obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle488obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF488=chanNum++;continue;
		}
		if (i->name=="TIRF532"){
			channels.push_back(Channel("EPI532","Cy3, Cy3.5, Texas Red, Alexa 568, Alexa 594 Ex(562nm) Em(624nm)",l532,*i,&tirf));
			channels.back().addTIRFangle(EPIangle532obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(EPIangle532obj63x,Magnification::getObjective("63xOil"));
			cont.EPI532=chanNum++;
			channels.push_back(Channel("TIRF532","Cy3, Cy3.5, Texas Red, Alexa 568, Alexa 594 Ex(562nm) Em(624nm)",l532,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle532obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle532obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF532=chanNum++;continue;
		}
		if (i->name=="TIRF532_EX"){
			channels.push_back(Channel("TIRF532_EX","2nd dye",l532,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle532obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle532obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF532_EX=chanNum++;continue;
		}
		if (i->name=="TIRF642"){
			channels.push_back(Channel("EPI642","Cy5, Alexa 647",l642,*i,&tirf));
			channels.back().addTIRFangle(EPIangle642obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(EPIangle642obj63x,Magnification::getObjective("63xOil"));
			cont.EPI642=chanNum++;
			channels.push_back(Channel("TIRF642","Cy5, Alexa 647",l642,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle642obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle642obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF642=chanNum++;continue;
		}
		if (i->name=="TIRF642_EX"){
			channels.push_back(Channel("TIRF642_EX","2nd dye",l642,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle642obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle642obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF642_EX=chanNum++;continue;
		}
		if (i->name=="TIRF660"){
			channels.push_back(Channel("EPI660","Cy5.5, Alexa 660, Alexa 680",l660,*i,&tirf));
			channels.back().addTIRFangle(EPIangle660obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(EPIangle660obj63x,Magnification::getObjective("63xOil"));
			cont.EPI660=chanNum++;
			channels.push_back(Channel("TIRF660","Cy5.5, Alexa 660, Alexa 680",l660,*i,&tirf));
			channels.back().addTIRFangle(TIRFangle660obj100x,Magnification::getObjective("100xOil"));
			channels.back().addTIRFangle(TIRFangle660obj63x,Magnification::getObjective("63xOil"));
			cont.TIRF660=chanNum++;continue;
		}
		if (j==currentCube)
			currentChan=0;
		logFile.write(string("Unused cube: ")+i->name,false);
	}
	this->noChannels=false;
//	noChannels=false;
	if (channels.size()<1){
		logFile.write("No channels found.",true);
		noChannels=true;
		//system("pause");
		//exit(0);
	}
	if (mags.size()<1){
		logFile.write("No objectives found.",true);
		noChannels=true;
		//system("pause");
		//exit(0);
	}
	if (cont.outPorts.size()<1){
		logFile.write("No output ports defined.",true);
		noChannels=true;
		//system("pause");
		//exit(0);
	}

	for(vector<Channel>::iterator i=cont.channels.begin();i!=cont.channels.end();i++){
		currentChannels.push_back(vector<AcquisitionChannel*>(cont.outPorts.size(),NULL));
	}

	if (!noChannels){
//		logFile.write(string("Current chan is: ")+toString(currentChan)+" . Current out is: "+toString(currentOut),true);
		addDefaultAC(AcquisitionChannel(&(cont.channels[currentChan]),cont.channels[currentChan].getDefaultOutport(),&(cont.mags[currentMag])),currentChan,currentOut);
		//currentChannels.at(currentChan).at(currentOut)=new AcquisitionChannel(&(cont.channels[currentChan]),&(cont.outPorts[currentOut]),&(cont.mags[currentMag]));
	//	AcquisitionChannel ac=currentChannel();
		currentFocus=Focus(currentChannel());
		if (currentChannel().m->obj.isOil){
		currentChannel().m->obj.needsOil=false;
		currentChannel().m->obj.cleanOil=true;
		}
	}else{
		currentChannels.push_back(vector<AcquisitionChannel*>(1,new AcquisitionChannel()));
		//currentChannels.at(0)=new AcquisitionChannel();
	}
	//a1->testMultipleCameras();
	//a0->testMultipleCameras();
	//delete a0;
	//a0=new Andor885(&daqpci,1,0);
	//a0->testMultipleCameras();
	//system("pause");
	//CImg<unsigned short>* img0=a0->testGetPic();
	//CImg<unsigned short>* img1=a1->testGetPic();
	//a0->testStop();
	//a1->testStop();
	//delete img0;
	//delete img1;
	//((Camera*)a1)->startLiveView();
	//system("pause");
	//((Camera*)a0)->startLiveView();
	//system("pause");
	//a0->stopLiveView();
	//a1->stopLiveView();
#endif
//cout<<"Maximum unsigned 64bit integer is"<<_UI64_MAX<<endl;
}

AcquisitionChannel& Controller::currentChannel(){
return *currentChannels.at(currentChan).at(currentOut);
}

void Controller::addDefaultAC(AcquisitionChannel& acq,int &chan, int &out){
	int j=0;
	for(vector<Channel>::iterator i=channels.begin();i!=channels.end();i++,j++){
		if (acq.chan->name==i->name){
			int k=0;
			for(vector<OutPort>::iterator o=outPorts.begin();o!=outPorts.end();o++,k++){
				if (o->s.name==acq.out->s.name){
					if (currentChannels.at(j).at(k)==NULL)
						currentChannels.at(j).at(k)=new AcquisitionChannel(acq);
					else
						*currentChannels.at(j).at(k)=acq;
					chan=j;
					out=k;
					return;
				}
			}
		}
	}
}

void Controller::setCurrentChannel(OutPort* out){
	AcquisitionChannel* ac=getAcquisitionChannel(currentChannel().chan,out);
	if (ac==NULL)
		return addDefaultAC(AcquisitionChannel(currentChannel().chan,out,currentChannel().m,currentChannel().ap),currentChan,currentOut);
	ac->m=currentChannel().m;
	addDefaultAC(*ac,currentChan,currentOut);
}

void Controller::setCurrentChannel(Channel* chan){
	AcquisitionChannel* ac=getAcquisitionChannel(chan,NULL);
	if (ac==NULL)
		return addDefaultAC(AcquisitionChannel(chan,chan->getDefaultOutport(),currentChannel().m),currentChan,currentOut);
	ac->m=currentChannel().m;
	addDefaultAC(*ac,currentChan,currentOut);
}

void Controller::setCurrentChannel(AcquisitionChannel& acq){
	addDefaultAC(acq,currentChan,currentOut);
}

AcquisitionChannel* Controller::getAcquisitionChannel(Channel* chan,OutPort* out){
	int j=0;
	for(vector<Channel>::iterator i=channels.begin();i!=channels.end();i++,j++){
		if (chan->name==i->name){
			int k=0;
			for(vector<OutPort>::iterator o=outPorts.begin();o!=outPorts.end();o++,k++){
				if (out==NULL){
					if (currentChannels.at(j).at(k)!=NULL)
						return currentChannels.at(j).at(k);
				}
				else if (o->s.name==out->s.name){
					//if (currentChannels.at(j).at(k)==NULL)
					//	currentChannels.at(j).at(k)=new AcquisitionChannel(chan,out,currentChannel().m);
					//currentChannels.at(j).at(k)->m=currentChannel().m;
					return currentChannels.at(j).at(k);
				}
			}
		}
	}
	if (out==NULL) return NULL;
	logFile.write("Error: channel and or outport unknown");
	return NULL;
}

//do not include directory structure, just a folder name
void Controller::setWorkingDir(string d){
	string temp=WORKINGDIR+"\\"+d+"\\";
	if (workingDir==temp) return;
	workingDir=temp;
	logFile.write("Controller: Switching working directory to "+workingDir,true);
	logFile.open(workingDir+"logFile.txt");
	focusLogFile.write("Controller: Switching working directory to "+workingDir);
	focusLogFile.open(workingDir+"focusLogFile.txt");
	HANDLE pumpLogFile=CreateFile(string(workingDir+"PumpLog.txt").c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);
	if (pmp.isPresent) pmp.pumpServer->PumpSetLogFileID((long) pumpLogFile);
	CloseHandle(pmp.pumpLog);
	pmp.pumpLog=pumpLogFile;
	cont.te.TELogFile->write("Switching working directory to "+workingDir);
	cont.te.TELogFile->open(workingDir+"TELogFile.txt");
}
void Controller::stop(){
	threads.waitAll();
	te.powerOff();
	displays.closeAll();
	delete stg, focus;
	stg=0;focus=0;
	vector<Camera*>::const_iterator j;
	vector<Camera*>::const_iterator i;
	for(i=cameras.begin();i!=cameras.end();){	
		j=cameras.end();
		delete (*i);
		i=i+1;
	}
	
	cameras.clear();
	for(vector<Channel>::iterator i=channels.begin();i!=channels.end();i++){
		(*i).off();
	}
	for(vector<vector<AcquisitionChannel*>>::iterator i=currentChannels.begin();i!=currentChannels.end();i++){
		for(vector<AcquisitionChannel*>::iterator j=i->begin();j!=i->end();j++){
			delete *j;
		}
	}
}

Controller::~Controller(){
}

//can call other object abort methods if necessary (i.e. Stages and Pumps)
void Controller::abort(){
	threads.stopAll();
	stg->stop();
	focus->stop();
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->abort();
	}
	pmp.abort();
	stop();
}

void Controller::DAQControl(){
	char c;
	while(true){
		cout<<"Please select a task"<<endl;
		cout<<"1: Trigger Camera"<<endl;
		cout<<"2: Trigger DG5"<<endl;
		cout<<"3: Trigger Both"<<endl;
		cout<<"e: Exit DAQ Control"<<endl;
		cin>>c;
		switch(c){
			case '1':
				outPorts[Controller::OUT_CAM].cam->trigger();
				break;
		
			case '3':
				currentChannel().chan->lite().ls->trigger();
				currentChannel().out->cam->trigger();
				break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
}
void Controller::TEModuleControl(){
	char c;
	string n,temp;
	int I=1,J=1,K=1;
	int propBand[]={8};
	double integGain[]={.0711};
	double derGain[]={.2027};
	ostringstream name;
	while(true){
		cout<<"Please select a task"<<endl;
		cout<<"1: Go to Fixed Temperature"<<endl;
		cout<<"2: Turn TEModule Off"<<endl;
		cout<<"3: Linearly Ramp Temperature"<<endl;
		cout<<"4: Apply Fixed Output"<<endl;
		cout<<"5: Get Current Temperature"<<endl;
		cout<<"6: Get Temperature Setting"<<endl;
		cout<<"7: Get Output Power"<<endl;
		cout<<"8: Get Current Temperature at Heatsink"<<endl;
		cout<<"9: Set PID parameters"<<endl;
		cout<<"0: Turn TEModule On"<<endl;
		cout<<"a: Clear Alarm Latch"<<endl;
		//cout<<"c: Change TEModule Multipler (currently "<<cont.te.getMultiplier()<<")"<<endl;
		cout<<"d: Change directory of TELogFile"<<endl;
		cout<<"l: display control on"<<endl;
		cout<<"m: display control off"<<endl;
		cout<<"q: Stop recording"<<endl;
		cout<<"r: Start recording periodically"<<endl;
		cout<<"s: stop temp ramp"<<endl;//Get Step response for PID Tuning in MATLAB"<<endl;
		cout<<"t: Test PID parameters"<<endl;
		cout<<"e: Exit TEModule Control"<<endl;
		c=getChar();
		switch(c){
			case '1':
				double temperature;
				cout<<"Please enter desired temperature (in degrees C)"<<endl;
				cin>>temperature;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				te.setFixedTemp(temperature,false);
				te.powerOn();
				//te.wait();
				break;
			case '2':
				te.powerOff();
				break;
			case '3':
					double startTemp, endTemp, time;
					//cout<<"Please enter desired start temperature (in degrees C)"<<endl;
					//startTemp=getDouble();
					//te.setFixedTemp(startTemp,false);
					te.periodicRecording(5);
					//te.powerOn();
					cout<<"Please enter desired end temperature (in degrees C)"<<endl;
					endTemp=getDouble();
					cout<<"Please enter desired time (in minutes)"<<endl;
					time=getDouble();
					//te.wait();
					//te.wait();
					//logFile.write("Reached starting temp for ramp.  Beginning ramp",true);
					te.linearTempRamp(endTemp, time, false);
					//logFile.write("Ramp complete, waiting 30 secs for recording stabilization");
					//Timer::wait(30*1000);//wait 30 secs for recording
					//te.record=false;
					//te.powerOff();
				break;
			case '4':
				double power;
				cout<<"Please enter desired output power (between -100 and 100)"<<endl;
				cin>>power;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				te.setOutputPower(power);
				te.powerOn();
				break;
			case '5':
				cout<<"Current temperature is "<<te.getTemp()<<" degree(s) Celsius"<<endl;
				break;
			case '6':
				cout<<"Current temperature setting is "<<te.getFixedTemp()<<" degree(s) Celsius"<<endl;
				break;
			case '7':
				cout<<"Current power output is "<<te.getOutputPower()<<"%"<<endl;
				break;
			case '8':
				cout<<"Current temperature at heat sink is "<<te.getTemp2()<<" degree(s) Celsius"<<endl;
				break;
			case '9':
				int bw;
				double ig, dg;
				cout<<"Enter proportional bandwidth"<<endl;
				bw=getInt();
				cout<<"Enter integral gain"<<endl;
				ig=getDouble();
				cout<<"Enter derivative gain"<<endl;
				dg=getDouble();
				te.setPropBand(bw);
				te.setIntegGain(ig);
				te.setDerGain(dg);
				break;
			case '0':
				te.powerOn();
				break;
			case 'a':
				te.clearAlarmLatch();
			break;
			/*case 'c':{
				double mult=cont.te.getMultiplier();
				if (mult==100)
					cont.te.setMultiplier(10);
				else
					cont.te.setMultiplier(100);
					 }
				break;
				*/
			case 'd':
				cout<<"Please enter the directory to save the TELogFile\n"
					<<"Enter 0 if you want to save it in the current working directory\n";
				cin>>temp;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				if (temp=="0") temp=cont.workingDir;
				te.changeLogFile(temp);
				break;
			case 'l':
				cont.te.displayControlOn();
				break;
			case 'm':
				cont.te.displayControlOff();
				break;
			case 'q':
				te.record=false;
				break;
			case 'r':
				double sTime;
				cout<<"Please enter the desired recording period (sec)\n";
				cin>>sTime;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				te.periodicRecording(sTime);
				break;
			case 's':
				te.stopRamp();
				break;
			case 't'://determine PID parameters
				name.str("");
				name<<"D:\\TEtest\\PIDparams.prop"<<propBand[0];
				for (int i=1;i<I;i++){
					name<<"-"<<propBand[i];
				}
				name<<".integ"<<integGain[0];
				for (int j=1;j<J;j++){
					name<<"-"<<integGain[j];
				}
				name<<".der"<<derGain[0];
				for (int k=1;k<K;k++){				
					name<<"-"<<derGain[k];
				}
				name<<".dat";
				cout<<"Saving data to: "<<name.str()<<endl;
				te.periodicRecording(500);
				te.powerOn();
				for (int i=0;i<I;i++){
					for (int j=0;j<J;j++){
						for (int k=0;k<K;k++){
							//name<<"D://TEtest//"<<propBand[i]<<"propBand"<<integGain[j]<<"integGain"<<derGain[k]<<"derGain.dat"<<flush;
							//n="D://TEtest//linearRamp.dat";
							//const char* cname=name.str().c_str();
							//const char* cn=n.c_str();
							//cout<<"File name is:"<<name.str().c_str()<<endl;
							//cout<<"File name is:"<<n.c_str()<<endl;
							
							te.setFixedTemp(25);
							//
							Timer::wait(1*60*1000);
							te.linearTempRamp(30,2,true,propBand[i],integGain[j],derGain[k]);
							Timer::wait(1*60*1000);
							
							//Sleep(100);
							//name.str("");
						}
					}
				}
				te.record=false;
				te.powerOff();
				break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
}

void Controller::liveView(){
	char c;
	string t;
	//unsigned threadID;
	if (noChannels) return;
	
	/*static Chamber cham;
	if (currentChamber.maxZ!=0 && !(cham==currentChamber)){
		logFile.write("New chamber maxZ and focusZ defined, moving Z to specified focus position",true);
		system("pause");
		cont.axio.setMaxZ(currentChamber.maxZ);
		cont.focus->move(currentChamber.focusZ);
		cham=currentChamber;
	}
*/
	currentChannel().out->cam->startLiveView();
	while(true){
		//Timer::wait(1000);'
		currentChannel().out->cam->waitLiveViewStart();
		showConsole();
		currentChannel().on(true);
		//currentChannel().wait();
		cout<<"Please select a task"<<endl;
		cout<<"1: Take Picture"<<endl;
		cout<<"2: Set Exposure (currently "<<currentChannel().ap.exp*1000<<"ms)\n";
		cout<<"3: Set Gain (currently "<<currentChannel().ap.getGains()<<")"<<endl;
		cout<<"4: Set Binning (currently "<<currentChannel().ap.bin<<")"<<endl;
		cout<<"5: Autofocus"<<endl;
		cout<<"6: Select Channel (currently "<<currentChannel().chan->toString()<<")\n";
		cout<<"7: Get Camera Temperature\n";
		cout<<"8: Change viewing port (currently to the "<<currentChannel().out->toString()<<")\n";
		cout<<"9: Change magnification (currently "<<currentChannel().m->toString()<<")\n";
		cout<<"0: Change Light Intensity (currently at "<<currentChannel().chan->lite().ls->intensityToString(currentChannel().intensity)<<")\n";
		cout<<"f: Stepwise Autofocus"<<endl;
		if (currentChannel().chan->tirf){
			cout<<"a: Change TIRF angle (currently "<<currentChannel().TIRFangle<<" Volts"<<endl;
		}
		cout<<"b: change current channel to focus channel"<<endl;
		cout<<"c: Close Displays"<<endl;
		cout<<"m: Measure Power"<<endl;
		cout<<"p: Pump Control"<<endl;
		cout<<"s: Stage Control\n";
		cout<<"t: Test autofocus and record"<<endl;
		cout<<"v: Auto adjust Gain and Exposure"<<endl;
		cout<<"z: Focus Control\n";
		cout<<"D: Definite Focus Control"<<endl;
		cout<<"e: Exit Live View"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		string filename;
		switch(c){
			case '1':
				//string filename;
				cout<<"Please enter a filename (no directories) without an extension"<<endl;
				cin>>filename;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().out->cam->saveLiveImage(workingDir+filename);
				break;
			case '2':
				double exp;
				cout<<"Please enter the exposure time (ms)"<<endl;
				cin>>exp;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().out->cam->stopLiveView();
				currentChannel().ap.exp=exp/1000;
				currentChannel().out->cam->startLiveView();
				break;
			case '3':{
				string gains;
				int gain;
				currentChannel().out->cam->stopLiveView();
				cout<<"Please enter the gain (use a comma for multiple gains)"<<endl;
				gains=::getString();
				stringstream ss;
				ss<<gains;
				vector<int> gains2;
				while(getline(ss,gains,',')){
					gains2.push_back(toInt(gains));
				}
				currentChannel().ap.setGain(gains2);
				currentChannel().out->cam->startLiveView();
				break;
					 }
			case '4':
				int bin;
				cout<<"Please enter the binning"<<endl;
				cin>>bin;
				currentChannel().out->cam->stopLiveView();
				currentChannel().ap.bin=bin;
				currentChannel().out->cam->startLiveView();
				break;
			case '5':{
				currentChannel().off();
				//currentChannel().out->cam->stopLiveView();
				currentFocus.modify();
				currentFocus.getFocus(true);
				//currentChannel().out->cam->startLiveView();
				break;
					 }
			case '6':{
				currentChannel().off(false);
				currentChannel().out->cam->stopLiveView();
				setCurrentChannel(Channel::select());	
				currentChannel().out->cam->startLiveView();
				break;}
			case '7':
				currentChannel().out->cam->stopLiveView();
				cout<< "Camera is currently cooled to " << currentChannel().out->cam->getTemp() << "C\n";
				currentChannel().out->cam->startLiveView();
				break;
			case '8':{
				currentChannel().out->cam->stopLiveView();
				setCurrentChannel(OutPort::select());
				currentChannel().out->cam->startLiveView();
				break;
					 }
			case '9':
				currentChannel().off(false);
				currentChannel().m=Magnification::select();
				break;
			case '0':{
				double temp;
				string units;
				cout<<"Enter the desired light intensity: \n";
				cin >>temp;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Enter the desired intensity units (e.g. %, f, V)\n";
				cin>>units;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().intensity=currentChannel().chan->lite().ls->getIntensity(temp,units);
				break;
					 }
			case 'f':{
				currentChannel().off(false);
				currentChannel().out->cam->stopLiveView();
				currentFocus.modify();
				bool b;
				cout<<"Select mode"<<endl;
				int i;
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentFocus.getFocus(true,-1,true,i);
				currentChannel().out->cam->startLiveView();
				break;
					 }
			case 'a':
				double angle;
				cout<<"Enter the desired TIRF angle in Volts (-2 to 10)"<<endl;
				cin>>angle;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().TIRFangle=angle;
				break;
			case 'b':
				currentChannel().off(false);
				currentChannel().out->cam->stopLiveView();
				currentChannel()=cont.currentFocus.a;
				currentChannel().out->cam->startLiveView();
				break;
			case 'm':{
				currentChannel().off(false);
				currentChannel().out->cam->stopLiveView();
				double val=pm.getPower(&currentChannel());
				currentChannel().out->cam->startLiveView();
				string units;
				double des=currentChannel().chan->lite().ls->getPower(currentChannel().intensity,units);
				logFile.write("Measured intensity is "+toString(val)+" mW. Selected intensity is "+toString(des)+" "+units+". Difference of "+toString(100.0*(des-val)/des)+"%",true);
					 }
					 break;
			case 'c':
				cont.displays.closeAll();
				break;
			case 'p':
				cont.pmp.pumpControl();
				break;
			case 's':
				stg->stageControl();
				break;
			case 't':{
				currentChannel().out->cam->stopLiveView();
				currentFocus.modify();//cout<< "Enter the number of steps and range for autofocusing:\n";
				//cin>>step>>range;
				//AcquisitionChannel myChan=selectAcquisitionChannel();
				//Focus f(myChan,range,step);
				currentFocus.getFocus(true);
				logFile.write("Temp:"+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString(stg->getX())+" Y:" +toString(stg->getY()) +" Zpos:" +toString(focus->getZ())+" Steps:"+toString(currentFocus.step)+" Range:"+toString(currentFocus.range)+" Exp:"+toString(currentFocus.a.ap.exp)+" Gain"+toString(currentFocus.a.ap.getGain())+" Bin:"+toString(currentFocus.a.ap.bin),DEBUGCONTROLLER);		
				currentChannel().out->cam->startLiveView();
				char temp;
				cout<< "\nPlease check the focus-> If not in focus, manually find the focus-> Enter 1 when done.\n";
				cin >>temp;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString(stg->getX())+" Y:"+toString(stg->getY())+ " ZPos:"+toString( focus->getZ()),DEBUGCONTROLLER);
				currentChannel().out->cam->startLiveView();
				break;
					 }
			case 'v':
				currentChannel().out->cam->stopLiveView();
				double percentSaturation;
				cout<<"Please enter desired percent saturation (e.g. 0.9)"<<endl;
				cin>>percentSaturation;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().out->cam->adjustIntensity(currentChannel(),percentSaturation);
				currentChannel().out->cam->startLiveView();
				break;
			case 'z':
				focus->focusControl();
				break;
			case 'D':
				//currentChannel().off();
				//currentChannel().out->cam->stopLiveView();
				cont.df.definiteFocusControl();
				//currentChannel().out->cam->startLiveView();
				break;
			case 'e':
				//cout<<"Turn off lights (y or n)?"<<endl;
				currentChannel().out->cam->stopLiveView();
				/*char g;
				cin>>g;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				if (g=='y' || g=='Y')*/ currentChannel().off();
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
			}
	}
	
}

void Controller::snapMode(){
	char c;
	while(true){
		cout<<"Please select a task"<<endl;
		cout<<"1: Take Picture"<<endl;
		cout<<"2: Set Exposure (currently "<<currentChannel().ap.exp<<"sec)"<<endl;
		cout<<"3: Set Gain (currently "<<currentChannel().ap.getGain()<<")"<<endl;
		cout<<"4: Set Binning (currently "<<currentChannel().ap.bin<<")"<<endl;
		cout<<"5: Fine Autofocus"<<endl;
		cout<<"6: Select Channel (currently "<<currentChannel().chan->toString()<<")"<<endl;
		cout<<"7: Change Acquisition Channel"<<endl;
		cout<<"8: Save Current Image"<<endl;
		cout<<"9: Take Multiple Pics"<<endl;
		cout<<"v: Auto adjust gain and exposure"<<endl;
		cout<<"e: Exit Snap Mode"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		string filename;
		switch(c){
			case '1':
				currentChannel().showOnScreen=true;
				//currentChannel().on(true);
				currentChannel().takePicture("snap");
				//currentChannel().off();
				break;
			case '2':
				float exp;
				cout<<"Please enter the exposure time in milliseconds"<<endl;
				cin>>exp;
				exp=exp/1000.0;
				currentChannel().ap.exp=exp;
				break;
			case '3':
				{
				string gains;
				int gain;
				cout<<"Please enter the gain (use a comma for multiple gains)"<<endl;
				gains=::getString();
				stringstream ss;
				ss<<gains;
				vector<int> gains2;
				while(getline(ss,gains,',')){
					gains2.push_back(toInt(gains));
				}
				currentChannel().ap.setGain(gains2);
				break;
					 }
				break;
			case '4':
				int bin;
				cout<<"Please enter the binning"<<endl;
				cin>>bin;
				currentChannel().ap.bin=bin;
				break;

			case '5':{
				currentFocus.modify();
				currentFocus.getFocus(true);
				break;}
			case '6':
				currentChannel().chan=Channel::select();
				break;
			case '7':
				currentChannel().modify();
				break;
			case '8':{
				string fileName;
				cout<<"Please enter a file name (no directory or extension)"<<endl;
				cin>>fileName;
				MoveFile((this->workingDir+"snap.tif").c_str(),(this->workingDir+fileName+".tif").c_str());
				logFile.write("Snap Mode: saved file "+this->workingDir+fileName+".tif",DEBUGCONTROLLER," "+currentChannel().toString());
				break;}
			case '9':{
				string prefix;
				string af;
				string range;
				cout<<"Please enter filename Prefix:"<<endl;
				cin>>prefix;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				AcquisitionChannel::takeMultiplePics(AcquisitionChannel::getAcquisitionChannels(),prefix,"");
			 }
				break;
			case 'e'://only exit point
				remove((this->workingDir+"snap.tif").c_str());
				return;
				break;
			case 'v':
				double percent;
				cout<<"Please enter desired percent saturation"<<endl;
				cin>>percent;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				currentChannel().out->cam->adjustIntensity(currentChannel(),percent);
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
}





//maxZ must be VERY accurate...and conservative to protect the objective
int main(int argc, char* argv[]){
	//AllocConsole();
	//hInput=GetStdHandle(STD_INPUT_HANDLE);
//	FlushConsoleInputBuffer(hInput);
	/*ReadConsoleInput(hInput, r, 512, &read);
	HANDLE proc=GetCurrentProcess();
	if (hInput==INVALID_HANDLE_VALUE){
		logFile.write("Could not get handle to standard input",true); 
		hInput=NULL;
	}
	HANDLE hInputTemp;
	DWORD res=1;//=DuplicateHandle(GetCurrentProcess(),hInput,GetCurrentProcess(),&hInputTemp,0,false,DUPLICATE_SAME_ACCESS);
	if (res==0){
		logFile.write("Duplicate Handle to std input failed.",true);
		LPVOID lpMsgBuf;
				DWORD dw=GetLastError();
		FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

		cout<<"duplicate handle error:" <<(char*)lpMsgBuf<<endl;
	}
	PACL ppDacl;
	PSECURITY_DESCRIPTOR ppSecurityDescriptor;
	ACCESS_MASK AccessRights;
	PSID ppsidGroup;
	DWORD result=GetSecurityInfo(hInput,SE_KERNEL_OBJECT,DACL_SECURITY_INFORMATION&GROUP_SECURITY_INFORMATION,NULL,&ppsidGroup,&ppDacl,NULL,&ppSecurityDescriptor);
	if (result!=ERROR_SUCCESS){
		LPVOID lpMsgBuf;
				DWORD dw=GetLastError();
		FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

		cout<<"error in get security descirptor:" <<(char*)lpMsgBuf<<endl;

	}
	else{
	TRUSTEE t;
	BuildTrusteeWithSid(&t,ppsidGroup);
	GetEffectiveRightsFromAcl(ppDacl,&t,&AccessRights);
	if ((AccessRights)&SYNCHRONIZE){
		logFile.write("console input synchronization is allowed",true);
	}else{
		logFile.write("console input synchronization is NOT allowed",true);
	}
	LocalFree(ppSecurityDescriptor);
	}

	*/
	unsigned int threadID;
	HANDLE mainThread=(HANDLE) _beginthreadex(NULL, 0, &Controller::mainThread, NULL, 0, &threadID);

	MessageBox(NULL,"Press OK to terminate application","ABORT",MB_ICONSTOP);
	bool manualClick=false;
	if (WAIT_TIMEOUT==WaitForSingleObject(abortEvent,1))
		manualClick=true;
	SetEvent(abortEvent);	
	SetForegroundWindow(console);
	keybd_event(VK_RETURN, NULL, NULL, NULL);
	keybd_event(VK_RETURN, NULL, KEYEVENTF_KEYUP, NULL);
	bool mainThreadLock=false;
	if (WAIT_TIMEOUT==WaitForSingleObject(mainThread,1000)){
			mainThreadLock=true;
			logFile.write("Program main thread will be terminated. This is usually undesireable, consider checking for the abortEvent and throwing an abortException. If you already do this then think of a way to make it respond quicker to the abort event",true);
	}
//	Timer::wait(3000);
	//clear();//<<"/n";
	if (manualClick){
		cont.abort();
		logFile.write("Program forcefully aborted by user",true);
	}else if (internalAbort){
		cont.abort();
		logFile.write("Program forcefully aborted internally due to error in thread",true);
	}else {
		cont.stop();
		logFile.write("Program done executing...Good bye",true);
	}
	if (mainThreadLock){
		TerminateThread(mainThread,0);
	}
	system("pause");
	CloseHandle(mainThread);
	
}

unsigned __stdcall Controller::mainThread(void* param)
{	
	RECT area;
	HWND hwndFound;
	while(true){
		hwndFound=FindWindow(NULL, "ABORT");
		if (hwndFound!=NULL) break;
	}
	SystemParametersInfo(SPI_GETWORKAREA,0,&area,0);
	int widthPixels=area.right;//GetSystemMetrics(SM_CXMAXIMIZED);
	GetWindowRect(hwndFound,&area);
	MoveWindow(hwndFound,widthPixels-(area.right-area.left),0,area.right-area.left,area.bottom-area.top,true);
	showConsole();
	string sTemp;
	char c;
	cout<<endl<<endl<<"Welcome to the Microautomation Application"<<endl;
	while(true){
		try{
		cout<<endl;
		cout<<"Please choose a task"<<endl;
		cout<<"1:  Live View (you can save images and do autofocusing)"<<endl;
		cout<<"2:  Eric Roller's Protocols"<<endl;
		cout<<"3:  Walsh's Protocols"<<endl;
		cout<<"4:  Trigger Control"<<endl;
		cout<<"9:  Snap Mode"<<endl;
		cout<<"c:  Close All windows"<<endl;
		cout<<"m:  Power Meter Control"<<endl;
		cout<<"p:  Pump Control"<<endl;
		cout<<"s:  Stage Control"<<endl;
		cout<<"t:  TEModule Control"<<endl;
		cout<<"w:  Select Working Directory"<<endl;
		cout<<"z:  Z Focus Control"<<endl;
		cout<<"e:  Exit"<<endl;
		//cout<<"E:  Exit using click Abort"<<endl;
		//WaitForMultipleObjects(2,
		c=getChar();
		//std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			
			case '1':
				cont.liveView();
				break;
			case '2':
				ProtocolEric::runProtocol();
				break;
			case '3':
				ProtocolWalsh::runProtocol();
				break;
			case '4':
				cont.currentChannel().out->cam->t->triggerControl();
				break;
			case '9':
				cont.snapMode();
				break;
			case 'c':
				cont.displays.closeAll();
				break;
			case 'm':
				cont.pm.powerMeterControl();
				break;
			case 'p':
				cont.pmp.pumpControl();
				break;
			case 's':
				cont.stg->stageControl();
				break;
			case 't':
				cont.TEModuleControl();
				break;
			case 'w':
				cont.modifyWorkingDir();
				break;
			case 'z':
				cont.focus->focusControl();
				break;
			case 'e':{
				cout<<"Are you sure you want to quit (y or n)?"<<endl;
				char v=getChar();
				if (v!='y')
					break;
				cout<<"Keep cameras cooled (y or n)?"<<endl;
				v=getChar();
				if (v=='n'){
					for(vector<Camera*>::iterator i=cont.cameras.begin();i!=cont.cameras.end();i++){
						(*i)->stopCooling();
					}
				}else if (v!='y')
					break;
				//system("pause");
				::clickAbort(false);
				return 0;
				break;
				}
			//case 'E':
			//	//cout<<"Program Done Executing...Goodbye"<<endl;
			//	::clickAbort(true);
			//	return 0;
			//	break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
				
		}
		}catch(std::ios_base::failure e)
			{logFile.write("Bad input, check code and handle error earlier if you want",true);
				continue;
		}catch(abortException abe){
			logFile.write(string(abe.what()),true);
			return 0;
		}catch(exception& e){
			logFile.write(string(e.what()),true);
			continue;
		}
	}

}

void Controller::modifyWorkingDir(){
	string sTemp;
	cout<<"Enter desired working folder name"<<endl;
	cin>>sTemp;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cont.setWorkingDir(sTemp);
}
void Controller::UVExpose(double seconds){
	channels[Controller::UV].on(100);
	Timer::wait(1000.0*seconds);
	channels[Controller::UV].off();
}



