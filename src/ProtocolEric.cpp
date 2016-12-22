// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Wednesday, June 01, 2011</created>
// <lastedit>Wednesday, April 04, 2012</lastedit>
// ===================================
// ===================================
#include "ProtocolEric.h"
#include <iostream>
#include <limits>
#include "Ludl.h"
#include <iostream>
#include <windows.h>
#include <vector>
#include "SpotSeriesScan.h"
#include "DefiniteFocus.h"
#include "AndorCam.h"
#define _USE_MATH_DEFINES
#include "Math.h"
#include "Reagent.h"
#include "FlowChannel.h"
using namespace std;
extern string WORKINGDIR;
extern int protocolIndex;
extern vector<string> protocolFiles;

int omnicureStart=0;
int omnicureEnd=0;
bool omnicureOscillating=true;
//global functions not specific to a microscope


extern Chamber currentChamber;
extern customProtocol scan;

void ProtocolEric::expose(int numFOVs, double numFOVspacingY, double seconds,AcquisitionChannel* channel){
	cont.stg->setYstep(channel->getFOVHeight()*numFOVspacingY/cont.stg->getStepSize());

	channel->on();
	Timer::wait(1000*seconds);
	channel->off();

	for (int j=1;j<numFOVs;j++){
		cont.stg->stepUp();
		cont.stg->wait();
	}

	for (int j=1;j<numFOVs;j++){
		cont.stg->stepDown();
		cont.stg->wait();
	}


}

int DefiniteFocusScan::numSpotsGlobal(){
	return num;
}
DefiniteFocusScan::~DefiniteFocusScan(){
		wait();
	}
void DefiniteFocusScan::move2NextSpotGlobal(ScanFocus *sf,int spotNum){
	Timer t(true);
	if (direction==0 || direction==1)
		cont.stg->setYstep(this->getChan(0).getFOVHeight()*FOVspacing/cont.stg->getStepSize());
	else
		cont.stg->setXstep(this->getChan(0).getFOVWidth()*FOVspacing/cont.stg->getStepSize());
	if (direction==0)//+y
		cont.stg->stepUp();
	else if (direction==1)//-y
		cont.stg->stepDown();
	else if (direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	cont.stg->wait();
	this->scanLogFile->write(string("Step took: ")+toString(t.getTime())+" ms");
	if (sf) {
		sf->updateFocus(true);
	}
}

void DefiniteFocusScan::move2NextSpotLocal(ScanFocus *sf,int spotNum){
	Timer t(true);
	if (direction==0 || direction==1)
		cont.stg->setYstep(this->getChan(0).getFOVHeight()*FOVspacing/cont.stg->getStepSize());
	else
		cont.stg->setXstep(this->getChan(0).getFOVWidth()*FOVspacing/cont.stg->getStepSize());
	if (direction==0)//+y
		cont.stg->stepUp();
	else if (direction==1)//-y
		cont.stg->stepDown();
	else if (direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	
	this->scanLogFile->write(string("Step took: ")+toString(t.getTime())+" ms");
	if (sf) {
		cont.stg->wait();
		sf->updateFocus(true);
	}
}

DefiniteFocusScan::DefiniteFocusScan(std::vector<AcquisitionChannel>& acquisitionChannels,int num,double FOVspacing,int direction,string namePrefix):dfs(),Scan(acquisitionChannels,"Scan",&dfs),num(num),FOVspacing(FOVspacing),direction(direction),namePrefix(namePrefix){
	cont.stg->setYstep(1*acquisitionChannels.front().getFOVHeight()/cont.stg->getStepSize());
}




void ProtocolEric::DefiniteFocus(){
	Ludl* ludl= (Ludl*) cont.stg;

	//ludl->setAccel(1);
	//ludl->setSpeed(2764800);//2764800 this is the maximum value we can send
	//ludl->disableServo();
	cont.stg->chamberMove(0,1,0);
	int numSpots=50;
	double FOVspacing=1;
	AcquisitionChannel fitc(cont.FITC,cont.OUT_CAM,cont.MAG_100xOil,AcquisitionParameters(.04,24,1,ImageRegion(),true));
	vector<AcquisitionChannel> channels;
	channels.push_back(fitc);channels.push_back(fitc);channels.push_back(fitc);channels.push_back(fitc);
	DefiniteFocusScan definiteFocus(channels,numSpots,FOVspacing,1);
	cont.setWorkingDir("TestingTrigger");
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	Focus f(fitc,20);
/*	f.getFocus();

#ifdef OBSERVER
	if (!cont.axio.definiteFocus->InitOnCurrentFocusPosition(MTBApi::MTBCmdSetModes_Default)){
		logFile.write("Could not initialize definite focus system",true);
	}

	cont.axio.definiteFocus->StabilizePeriodically_2(MTBApi::MTBOnOff_On,0);
#endif	
	*/
	definiteFocus.runScan();
}

#ifdef AXIOVERT
void ProtocolEric::runProtocol(){
	char c;
	while(true){
		cout<<"Please select a protocol to h run:"<<endl;
		cout<<"1: (m)nSBS 1, 2 and 3 base homopolymer runs"<<endl;
		cout<<"2: Amplified Protocols"<<endl;
		cout<<"3: Definite Focus Test"<<endl;
		cout<<"F: test focus wait:"<<endl;
		cout<<"O: test objective turret wait:"<<endl;
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':
				nSBSHomopolymer();
				break;
			case '2':
				AmplifiedProtocols();
				break;
			case '3':
				DefiniteFocus();
			break;
			case 'F':
				cont.focus->move(0);
				Timer::wait(10000);
				cout<<"begin monitoring"<<endl;
				system("pause");
				cont.focus->move(5000);
				cout<<"begin monitoring"<<endl;
				cont.focus->wait();
				break;
			case 'O':
				//cont.axio->;
				Timer::wait(10000);
				cout<<"begin monitoring"<<endl;
				system("pause");
				cont.focus->move(5000);
				cout<<"begin monitoring"<<endl;
				cont.focus->wait();
				break;
			case 'e':
				return;
				break;
		}
	}
}


void ProtocolEric::photocleave(double numFOVSpacingY,double numFOVSpacingX,Focus *f1,Focus*f2,double seconds, vector<vector<AcquisitionChannel>> &FOVChannels){

	AcquisitionChannel uvTL(cont.UV,cont.OUT_BLOCKED,cont.MAG_100xOil);
	
	expose(FOVChannels.size(),numFOVSpacingY,seconds,&uvTL);

}

void ProtocolEric::AmplifiedProtocols(){
	double numFOVSpacing=4.0;

	//CY55 Channels	
	AcquisitionChannel Cy55_0_0(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,3,1),1,false);
	AcquisitionChannel Cy55_1_0(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,6,1),1,false);
	AcquisitionChannel Cy55_0_1(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,3,1),1,false);
	AcquisitionChannel Cy55_1_1(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,6,1),1,false);
	AcquisitionChannel Cy55_0_2(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,3,1),1,false);
	AcquisitionChannel Cy55_1_2(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,6,1),1,false);
	AcquisitionChannel Cy55_0_3(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,12,1),1,false);
	AcquisitionChannel Cy55_1_3(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,24,1),1,false);	
	AcquisitionChannel Cy55_0_4(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,24,1),1,false);	
	AcquisitionChannel Cy55_1_4(cont.CY5,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,48,1),1,false);
	
	vector<AcquisitionChannel> channelsCy55_0;
	channelsCy55_0.push_back(Cy55_0_0);
	channelsCy55_0.push_back(Cy55_0_1);
	channelsCy55_0.push_back(Cy55_0_2);
	channelsCy55_0.push_back(Cy55_0_3);
	channelsCy55_0.push_back(Cy55_0_4);
	vector<AcquisitionChannel> channelsCy55_1;
	channelsCy55_1.push_back(Cy55_1_0);
	channelsCy55_1.push_back(Cy55_1_1);
	channelsCy55_1.push_back(Cy55_1_2);
	channelsCy55_1.push_back(Cy55_1_3);
	channelsCy55_1.push_back(Cy55_1_4);


	//CY35 Channels
	AcquisitionChannel Cy35_0_0(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,3,1),1,false);
	AcquisitionChannel Cy35_1_0(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,6,1),1,false);
	AcquisitionChannel Cy35_0_1(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,3,1),1,false);
	AcquisitionChannel Cy35_1_1(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,6,1),1,false);
	AcquisitionChannel Cy35_0_2(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,3,1),1,false);
	AcquisitionChannel Cy35_1_2(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,6,1),1,false);
	AcquisitionChannel Cy35_0_3(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,12,1),1,false);
	AcquisitionChannel Cy35_1_3(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,24,1),1,false);	
	AcquisitionChannel Cy35_0_4(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,24,1),1,false);	
	AcquisitionChannel Cy35_1_4(cont.CY3,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,48,1),1,false);
	
	vector<AcquisitionChannel> channelsCy35_0;
	channelsCy35_0.push_back(Cy35_0_0);
	channelsCy35_0.push_back(Cy35_0_1);
	channelsCy35_0.push_back(Cy35_0_2);
	channelsCy35_0.push_back(Cy35_0_3);
	channelsCy35_0.push_back(Cy35_0_4);
	vector<AcquisitionChannel> channelsCy35_1;
	channelsCy35_1.push_back(Cy35_1_0);
	channelsCy35_1.push_back(Cy35_1_1);
	channelsCy35_1.push_back(Cy35_1_2);
	channelsCy35_1.push_back(Cy35_1_3);
	channelsCy35_1.push_back(Cy35_1_4);

	//FITC Channels
	AcquisitionChannel FITC_0_0(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,3,1),1,false);
	AcquisitionChannel FITC_1_0(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,6,1),1,false);
	AcquisitionChannel FITC_0_1(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,3,1),1,false);
	AcquisitionChannel FITC_1_1(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,6,1),1,false);
	AcquisitionChannel FITC_0_2(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,3,1),1,false);
	AcquisitionChannel FITC_1_2(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,6,1),1,false);
	AcquisitionChannel FITC_0_3(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,12,1),1,false);
	AcquisitionChannel FITC_1_3(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,24,1),1,false);	
	AcquisitionChannel FITC_0_4(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,24,1),1,false);	
	AcquisitionChannel FITC_1_4(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,48,1),1,false);
	
	vector<AcquisitionChannel> channelsFITC_0;
	channelsFITC_0.push_back(FITC_0_0);
	channelsFITC_0.push_back(FITC_0_1);
	channelsFITC_0.push_back(FITC_0_2);
	channelsFITC_0.push_back(FITC_0_3);
	channelsFITC_0.push_back(FITC_0_4);
	vector<AcquisitionChannel> channelsFITC_1;
	channelsFITC_1.push_back(FITC_1_0);
	channelsFITC_1.push_back(FITC_1_1);
	channelsFITC_1.push_back(FITC_1_2);
	channelsFITC_1.push_back(FITC_1_3);
	channelsFITC_1.push_back(FITC_1_4);

	//DAPI Channels
	AcquisitionChannel DAPI_0_0(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,3,1),1,false);
	AcquisitionChannel DAPI_1_0(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.010,6,1),1,false);
	AcquisitionChannel DAPI_0_1(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,3,1),1,false);
	AcquisitionChannel DAPI_1_1(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.040,6,1),1,false);
	AcquisitionChannel DAPI_0_2(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,3,1),1,false);
	AcquisitionChannel DAPI_1_2(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,6,1),1,false);
	AcquisitionChannel DAPI_0_3(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,12,1),1,false);
	AcquisitionChannel DAPI_1_3(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.160,24,1),1,false);	
	AcquisitionChannel DAPI_0_4(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,24,1),1,false);	
	AcquisitionChannel DAPI_1_4(cont.DAPI,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.320,48,1),1,false);
	
	vector<AcquisitionChannel> channelsDAPI_0;
	channelsDAPI_0.push_back(DAPI_0_0);
	channelsDAPI_0.push_back(DAPI_0_1);
	channelsDAPI_0.push_back(DAPI_0_2);
	channelsDAPI_0.push_back(DAPI_0_3);
	channelsDAPI_0.push_back(DAPI_0_4);
	vector<AcquisitionChannel> channelsDAPI_1;
	channelsDAPI_1.push_back(DAPI_1_0);
	channelsDAPI_1.push_back(DAPI_1_1);
	channelsDAPI_1.push_back(DAPI_1_2);
	channelsDAPI_1.push_back(DAPI_1_3);
	channelsDAPI_1.push_back(DAPI_1_4);


	//FOVS
	vector<AcquisitionChannel> FOV0=channelsCy55_0;
	FOV0.insert(FOV0.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV0.insert(FOV0.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV0.insert(FOV0.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV1=channelsCy55_0;
	FOV1.insert(FOV1.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV1.insert(FOV1.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV1.insert(FOV1.end(),channelsDAPI_1.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV2=channelsCy55_0;
	FOV2.insert(FOV2.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV2.insert(FOV2.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV2.insert(FOV2.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV3=channelsCy55_0;
	FOV3.insert(FOV3.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV3.insert(FOV3.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV3.insert(FOV3.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV4=channelsCy55_0;
	FOV4.insert(FOV4.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV4.insert(FOV4.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV4.insert(FOV4.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV5=channelsCy55_0;
	FOV5.insert(FOV5.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV5.insert(FOV5.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV5.insert(FOV5.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV6=channelsCy55_0;
	FOV6.insert(FOV6.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV6.insert(FOV6.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV6.insert(FOV6.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV7=channelsCy55_0;
	FOV7.insert(FOV7.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV7.insert(FOV7.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV7.insert(FOV7.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV8=channelsCy55_1;
	FOV8.insert(FOV8.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV8.insert(FOV8.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV8.insert(FOV8.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV9=channelsCy55_1;;
	FOV9.insert(FOV9.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV9.insert(FOV9.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV9.insert(FOV9.end(),channelsDAPI_1.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV10=channelsCy55_1;
	FOV10.insert(FOV10.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV10.insert(FOV10.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV10.insert(FOV10.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV11=channelsCy55_1;
	FOV11.insert(FOV11.end(),channelsCy35_0.begin(),channelsCy35_0.end());
	FOV11.insert(FOV11.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV11.insert(FOV11.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV12=channelsCy55_1;
	FOV12.insert(FOV12.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV12.insert(FOV12.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV12.insert(FOV12.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV13=channelsCy55_1;
	FOV13.insert(FOV13.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV13.insert(FOV13.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV13.insert(FOV13.end(),channelsDAPI_0.begin(),channelsDAPI_0.end());
	vector<AcquisitionChannel> FOV14=channelsCy55_1;
	FOV14.insert(FOV14.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV14.insert(FOV14.end(),channelsFITC_0.begin(),channelsFITC_0.end());
	FOV14.insert(FOV14.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	vector<AcquisitionChannel> FOV15=channelsCy55_1;
	FOV15.insert(FOV15.end(),channelsCy35_0.begin(),channelsCy35_1.end());
	FOV15.insert(FOV15.end(),channelsFITC_0.begin(),channelsFITC_1.end());
	FOV15.insert(FOV15.end(),channelsDAPI_0.begin(),channelsDAPI_1.end());
	
	vector<vector<AcquisitionChannel>> FOVChannels;
	
	FOVChannels.push_back(FOV0);
	FOVChannels.push_back(FOV1);
	FOVChannels.push_back(FOV2);
	FOVChannels.push_back(FOV3);
	FOVChannels.push_back(FOV4);
	FOVChannels.push_back(FOV5);
	FOVChannels.push_back(FOV6);
	FOVChannels.push_back(FOV7);
	FOVChannels.push_back(FOV8);
	FOVChannels.push_back(FOV9);
	FOVChannels.push_back(FOV10);
	FOVChannels.push_back(FOV11);
	FOVChannels.push_back(FOV12);
	FOVChannels.push_back(FOV13);
	FOVChannels.push_back(FOV14);
	FOVChannels.push_back(FOV15);

	AcquisitionChannel fluo(cont.FITC,cont.OUT_CAM,cont.MAG_10x,AcquisitionParameters(.035,3,1),1,false);
	
	Focus focus1(fluo,20,.1);
	Focus focus2(fluo,10,.1);

	char c;
	while(true){
		cout<<"Please select an Amplified protocol to run:"<<endl;
		cout<<"3: Take images"<<endl;
		cout<<"4: Wash and Image"<<endl;
		cout<<"5: Cleave"<<endl;
		cout<<"6: TIRF BLEACH AND IMAGE"<<endl;
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		
		switch(c){
			case '1':
				
				break;
			case '2':
				//intensityAdjust();
				break;
			case '3':
				//takeTIRFImages(numFOVSpacing,&focus1,&focus2,FOVChannels);
				break;
				
			case '4':
				{		
				string root;
				cout<<"Please enter filename root"<<endl;
				cin>>root;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				//washAndImage(numFOVSpacing,numFOVSpacing,&focus1,&focus2,root,FOVChannels);
				break;
				}
			case '5':{
				
				double time;
				cout<<"Please enter time for cleavage (seconds)"<<endl;
				cin>>time;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				photocleave(numFOVSpacing,numFOVSpacing,&focus1,&focus2,time,FOVChannels);
				break;}
			case '6':{
				double totalSec,sampleSec;
				cout<<"Please enter total time for bleach (seconds)"<<endl;
				cin>>totalSec;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Please enter sample time (seconds)"<<endl;
				cin>>sampleSec;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
//				takeTIRFBleachImages(sampleSec,totalSec,numFOVSpacing,&focus1,&focus2,FOVChannels);
				
					 }
			case 'e':
				return;
				break;
		}
	}
}

void ProtocolEric::nSBSHomopolymer(){
	AcquisitionChannel a(cont.CY5,cont.OUT_CAM,cont.MAG_20x,AcquisitionParameters(.1,4,1,ImageRegion(ImageRegion::FULL)),1);
	Focus f(AcquisitionChannel(cont.FITC,cont.OUT_CAM,cont.MAG_20x,AcquisitionParameters(.1,4,1,ImageRegion(ImageRegion::CENTER)),1),10);
	int numCycles=3;
//	cont.pmp.setWashValves("B1","B3");
	Solution WashSBSBuffer(0,false,1,0);
	int WashNaOH=1;
//	cont.pmp.setReagentValves("B2","B4","B5","B6","B7");
	int ReagentBead=0;
	int ReagentTemplate=1;
	int ReagentDNAPoly=2;
	int ReagentACG=3;
	int ReagentU=4;
	cont.te.setFixedTemp(40,true);
	cont.setWorkingDir("mnSBS123Homopolymer");
	//wash chamber before experiment
	cont.pmp.wash(WashSBSBuffer,2);
	//bring in beads
//	cont.pmp.fillWithReagent(ReagentBead);
//	cont.pmp.washChamberFullStroke(1);
	//bring in template and incubate for 5 minutes
//	cont.pmp.fillWithReagent(ReagentTemplate);
	Timer::wait(5*60*1000);
	//denature template with NaOH for 10minutes
//	cont.pmp.washChamberFullStroke(WashNaOH);
	Timer::wait(10*60*1000);
	//wash away NaOH
//	cont.pmp.washChamberFullStroke(WashSBSBuffer,2);
	//bring in BST DNA polymerase
//	cont.pmp.fillWithReagent(ReagentDNAPoly,true);
	for(int n=0;n<numCycles;n++){
		//bring in 3 nucleotide solution for 60 seconds
	//	cont.pmp.fillWithReagent(ReagentACG,false);
		Timer::wait(60*1000);
		//wash out 3 nucleotide solution
	//	cont.pmp.washChamberFullStroke();
		//bring in labeled nucleotide solution
	//	cont.pmp.fillWithReagent(ReagentU,false);
		//cont.pmp.wait();//dont wait. start imaging immediately
		//image every 5 seconds for 60 seconds and autofocus before every picture
		for(int t=0;t<60/5;t++){
			f.getFocus(true,-1,true);
			a.takePicture(string("Homo")+toString(n,1)+string("t")+toString(t,2));
		}
	//	cont.pmp.washChamberFullStroke();
	}
}

/////////////////////////////////////////////////////
//////END AXIOVERT CODE//////////////////////////////
/////////////////////////////////////////////////////

////////////////////////////////////////////////////
//////START OBSERVER CODE///////////////////////////
////////////////////////////////////////////////////
#endif
#ifdef OBSERVER
void ProtocolEric::runProtocol(){
	char c;
	while(true){
		cout<<"Please select a protocol to run:"<<endl;
		cout<<"1: Custom Scan"<<endl; //Definite Focus test"<<endl;
		//cout<<"3: test Observer Focus"<<endl;
		//cout<<"4: test observer position"<<endl;
		//cout<<"5: test autofocus"<<endl;
		cout<<"2: Cyclic on Amplified Template"<<endl;
		//cout<<"8: Take 4 channel pics"<<endl;
		cout<<"3: Ligation Kinetics Complex Probes"<<endl;
		cout<<"4: TIRF angle optimization"<<endl;
		cout<<"5: SBD Experiment Test"<<endl;
		cout<<"6: Custom pump protocols"<<endl;
		cout<<"P: Photocleavage Kinetics"<<endl;
		//cout<<"A: Ligation Kinetics with Accumulation"<<endl;
		cout<<"B: Bleach Spots"<<endl;
		cout<<"O: Oscillate for Omnicure bleaching/photocleavage"<<endl;
		cout<<"D: Definite Focus control"<<endl;
		cout<<"T: DG Control"<<endl;
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		
		switch(c){
			case '1':
				customScan();//DefiniteFocus();
				break;
			/*case '5':{
				double range,step;
				cout<<"Please enter range"<<endl;
				cin>>range;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Please enter step"<<endl;
				cin>>step;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				getFocus(range,step);
					 }
				break;*/
			case '2':
				//cyclicAmplified();
				break;
			/*case '7':
				pumpTest();
				break;*/
			/*case '8':
				channelPics();
				break;*/
			case '3':
				ligationKinetics();
				break;
			case '4':
				TIRFadjust();
				break;
				case '5':
				testExperiment();
				break;
				case '6':
					pumpProtocols();
					break;
			case 'P':
				photocleavageKinetics();
				break;
			/*case 'A':
				ligationKineticsAccum();
				break;*/
			case 'B':
				{
				//	AcquisitionChannel rl(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_BLOCKED],&cont.mags[cont.MAG_100xOil],AcquisitionParameters(.030,4,1),1,true);
				AcquisitionChannel chan(cont.FITC,cont.OUT_BLOCKED,cont.MAG_100xOil);
				cont.stg->setYstep(4.0*cont.currentChannel().getFOVHeight()/cont.stg->getStepSize());
				int i=0;
				double d;
				cout<<"Enter time(seconds) to bleach"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Enter number of fields to bleach"<<endl;
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				for (int j=0;j<i;j++){
					chan.on();
					Timer::wait(1000*d);
					chan.off();
					cont.stg->stepUp();cont.stg->wait();
				}
				cout<<"Returning to start position"<<endl;
				for(int j=0;j<i;j++){
					cont.stg->stepDown();cont.stg->wait();
				}
				
	
					break;
				}
			case 'O':{
				double start=cont.focus->getZ();
				int startX=cont.stg->getX();
				int startY=cont.stg->getY();
				cont.outPort.set(5);
				cont.focus->move(0);cont.focus->wait();
				cout<<"Enter oscillation width (mm)"<<endl;
				double x=getDouble();
				omnicureStart=cont.stg->getX()-x*1000/2/cont.stg->getStepSize();
				omnicureEnd=cont.stg->getX()+x*1000/2/cont.stg->getStepSize();
				cout<<"Enter oscillation time (seconds)"<<endl;
				double t=getDouble();
				unsigned int threadID;
				HANDLE omnicureOscCopy;
				HANDLE omnicureOsc=(HANDLE) _beginthreadex(NULL,0,&omnicureThread,NULL,0,&threadID);
				omnicureOscillating=true;
				DuplicateHandle(GetCurrentProcess(),omnicureOsc,GetCurrentProcess(),&omnicureOscCopy,0,false,DUPLICATE_SAME_ACCESS);
				cont.threads.addThread(omnicureOsc);
				Timer::wait(1000*t);
				omnicureOscillating=false;
				WaitForSingleObject(omnicureOscCopy,INFINITE);
				CloseHandle(omnicureOscCopy);
				cont.stg->move(startX,startY);
				cont.stg->wait();
				cont.focus->move(start);
				cont.focus->wait();
				break;
					 }
			/*case 'O':{
				double start=cont.focus->getZ();
				cont.outPort.set(5);
				cont.focus->move(0);cont.focus->wait();
ASKSECTION:		cout<<"Select channel section (0 bottom, 1 middle, 2 top, or 3 entire channel"<<endl;
				int x=getInt();
				switch(x){
					case 0:
						omnicureStart=-cont.stg->chamberHeight/2;
						omnicureEnd=-cont.stg->chamberHeight/6;
						break;
					case 1:
						omnicureStart=-cont.stg->chamberHeight/6;
						omnicureEnd=cont.stg->chamberHeight/6;
						break;
					case 2:
						omnicureStart=cont.stg->chamberHeight/6;
						omnicureEnd=cont.stg->chamberHeight/2;
						break;
					case 3:
						omnicureStart=-cont.stg->chamberHeight/2;
						omnicureEnd=cont.stg->chamberHeight/2;
						break;
					default:
						goto ASKSECTION;
				}
				unsigned int threadID;
				HANDLE omnicureOscCopy;
				HANDLE omnicureOsc=(HANDLE) _beginthreadex(NULL,0,&omnicureThread,NULL,0,&threadID);
				omnicureOscillating=true;
				DuplicateHandle(GetCurrentProcess(),omnicureOsc,GetCurrentProcess(),&omnicureOscCopy,0,false,DUPLICATE_SAME_ACCESS);
				cont.threads.addThread(omnicureOsc);
				system("pause");
				omnicureOscillating=false;
				WaitForSingleObject(omnicureOscCopy,INFINITE);
				CloseHandle(omnicureOsc);
				cont.focus->move(start);
				cont.focus->wait();
				break;
					 }*/
			case 'D':
				cont.df.definiteFocusControl();
				break;
			case 'T':
				cont.dg5.DGControl();
				break;
			case 'e':
				return;
				break;

		}
	}
}

void ProtocolEric::testExperiment(){
	cont.setWorkingDir("MattSBD\\2010-03-12\\1 base 20min");
	//AcquisitionChannel alx405(cont.DAPILASER,cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.08,10),1);
	//Record temps405(cont.workingDir+"\\tempsChan0.txt");
	AcquisitionChannel alx488(cont.FITC,cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.06,6),1);
	Record temps488(cont.workingDir+"\\tempsChan1.txt");
	AcquisitionChannel alx546(cont.CY3,cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.080,5),33,"%");
	Record temps546(cont.workingDir+"\\tempsChan2.txt");
	AcquisitionChannel alx647(cont.CY5,cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.08,20),1);
	Record temps647(cont.workingDir+"\\tempsChan3.txt");
	AcquisitionChannel alx750(cont.CY7,cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.05,3),1);
	Record temps750(cont.workingDir+"\\tempsChan4.txt");
	vector<AcquisitionChannel> chans;
	chans.push_back(alx750);
	chans.push_back(alx647);
	chans.push_back(alx546);
	chans.push_back(alx488);
//	chans.push_back(alx405);
	vector<Record*> temps;	
	temps.push_back(&temps750);
	temps.push_back(&temps647);
	temps.push_back(&temps546);
	temps.push_back(&temps488);
	//temps.push_back(&temps405);
	double tStart=30;
	double tFinal=80;
	double totalTime=60*20;
	cont.te.setDerGain(0);
	cont.te.setIntegGain(.2);
	cont.te.setPropBand(75);
	//cont.te.linearTempRamp(tStart,0,false,75,.2,0);
	cont.te.setFixedTemp(tStart);
	
	
	int j=0,k=0;
	Timer swatch(true);
	
	cont.te.linearTempRamp(tFinal,totalTime/60.0,false);
	while(swatch.getTime()<totalTime*1000.0){
		k=0;
		
		for(vector<AcquisitionChannel>::iterator i=chans.begin();i!=chans.end();i++){
			
			cont.df.getDefiniteFocus(1,true);
			i->takePicture(string("Image")+toString(j,3)+"Chan"+toString(k,2));
			temps[k]->write(toString(cont.te.getTemp())+","+toString(cont.focus->getZ())+","+toString(swatch.getTime())+",",true);
			k++;
		}
		j++;
	}
	cont.te.setFixedTemp(20,false);
}

void ProtocolEric::focusEvaluation(){
	AcquisitionChannel acq(cont.FITC, cont.OUT_CAM, cont.MAG_20x, AcquisitionParameters(.03,3),1);
	Record temps(cont.workingDir+"\\temps.txt");
	temps.write("Temp\tFocus\t");
	double tStart=20;
	double tFinal=70;
	double totalTime=60*20;
	double period=totalTime/50;
	cont.te.setDerGain(0);
	cont.te.setIntegGain(.2);
	cont.te.setPropBand(75);
	//cont.te.linearTempRamp(tStart,0,false,75,.2,0);
	cont.te.setFixedTemp(tStart);
		
	cont.te.linearTempRamp(tFinal,totalTime/60.0,false);
	Timer t(true);
	int i=0;
	while(t.getTime()<totalTime*1000.0){
		t.waitTotal(period*1000.0*i);
		cont.df.getDefiniteFocus(1,true);
		acq.takePicture(string("Image")+toString(i,2));
		temps.write(toString(cont.te.getTemp())+"\t"+toString(cont.focus->getZ())+"\t",true);
		i++;
	}
	cont.te.powerOff();
}

//arbitrary time points
void ProtocolEric::timeSeries(customProtocol& scan){
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();

	cont.stg->wait();
	stringstream ss;
	string line,selection;
	vector<double> times;
	int numScans;
	string folder;
	cout<<"enter times in seconds separated by commas"<<endl;
	getline(cin,line);
	ss<<line;
	getline(ss,line,',');
	while(!ss.fail()){
		times.push_back(toDouble(line));
		getline(ss,line,',');
	}

	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	::DefiniteFocus df;
	ScanFocus* sf;
	if (scan.coarse.range==0)
		sf=NULL;
	else
		sf=&df;
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",times,sf);
	s.runScan(1,false);
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>selection;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

	if (selection!="c") {
		s.abort();
		cont.stg->move(x,y);
		return;
	}
	//cont.df.getDefiniteFocus(protocolIndex,true); //this is done by the SpotSeriesScan
	s.sendStartEvent();
	s.wait();
	cont.stg->move(x,y);
}

//totalTime and period
void ProtocolEric::timeSeries2(customProtocol& scan){
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();

	cont.stg->wait();
	stringstream ss;
	string line,selection;
	double totalTime;
	double period;
	int numScans;
	string folder;
	cout<<"enter period between each image"<<endl;
	period=getDouble();
	cout<<"enter total time in seconds for this scan"<<endl;
	totalTime=getDouble();

	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	::DefiniteFocus df;
	ScanFocus* sf;
	if (scan.coarse.range==0)
		sf=NULL;
	else
		sf=&df;
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",totalTime,period,sf);
	s.runScan(1,false);
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>selection;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

	if (selection!="c") {
		s.abort();
		cont.stg->move(x,y);
		return;
	}
	//cont.df.getDefiniteFocus(protocolIndex,true); this is done automagically
	s.sendStartEvent();
	s.wait();
	cont.stg->move(x,y);
}

//totalTime and period with reagent push
void ProtocolEric::reagentKinetics(customProtocol& scan){
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();

	cont.stg->wait();
	stringstream ss;
	string line,selection;
	double totalTime;
	double period;
	int numScans;
	string folder;
	cout<<"select Reagent to load"<<endl;
	Solution* r=scan.fs.selectReagent();
	FlowChannel* f=scan.fs.selectChannel();
	cout<<"enter period between each image"<<endl;
	period=getDouble();
	cout<<"enter total time in seconds for this scan"<<endl;
	totalTime=getDouble();

	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	::DefiniteFocus df;
	::DefiniteFocus df2(false);
	ScanFocus* sf=&df2;
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",totalTime,period,sf);
	s.runScan(1,false);
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>selection;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

	if (selection!="c") {
		s.abort();
		cont.stg->move(x,y);
		return;
	}
	r->load(f,0);
	df.updateFocus(true);//cont.df.getDefiniteFocus(protocolIndex,true); this is done automagically
	s.sendStartEvent();
	r->load(NULL,-1,totalTime);
	s.wait();
	cont.stg->move(x,y);
}

//two acquisition Groups...initial images captured with no definite focus
//arbitrary time points
void ProtocolEric::timeSeries3(customProtocol& scan){
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();

	cont.stg->wait();
	stringstream ss;
	string line,selection;
	vector<double> timesFirst;
	vector<double> timesSecond;
	int numScans;
	string folder;
	cout<<"enter initial times in seconds separated by commas (no focusing performed between images"<<endl;
	getline(cin,line);
	ss<<line;
	getline(ss,line,',');
	while(!ss.fail()){
		timesFirst.push_back(toDouble(line));
		getline(ss,line,',');
	}
	ss=stringstream();
	cout<<"enter final times in seconds separated by commas (focusing performed before each image"<<endl;
	getline(cin,line);
	ss<<line;
	getline(ss,line,',');
	while(!ss.fail()){
		timesSecond.push_back(toDouble(line));
		getline(ss,line,',');
	}


	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	::DefiniteFocus df;
	ScanFocus* sf;
	if (scan.coarse.range==0)
		sf=NULL;
	else
		sf=&df;
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",timesFirst,timesSecond,sf);
	s.runScan(1,false);
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>selection;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

	if (selection!="c") {
		s.abort();
		cont.stg->move(x,y);
		return;
	}
	//cont.df.getDefiniteFocus(protocolIndex,true); //this is done by the SpotSeriesScan
	s.sendStartEvent();
	s.wait();
	cont.stg->move(x,y);
}
void ProtocolEric::electrophoresis3(customProtocol& scan){
	string folder;
	string answer;
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	cout<<"Enter total time for imaging (seconds)"<<endl;
	double imageSecs=getDouble();
	cout<<"Enter delay before E-field (seconds)"<<endl;
	double delaySecs=getDouble();
	cout<<"Enter duration of E-field (seconds)"<<endl;
	double durationSecs=getDouble();
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>answer;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (answer!="c") return;
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	cont.stg->wait();
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",imageSecs);
	HANDLE electroThreadCopy;
	unsigned int threadID;
	logFile.write(string("Initial Delay:")+toString(delaySecs)+" Voltage Duration:"+toString(durationSecs)+" Total Time:"+toString(imageSecs));

	//continuous definite focus
	//cont.df.startDefiniteFocus(protocolIndex,0);

	//focus once before
	cont.df.getDefiniteFocus(protocolIndex,true);
	cont.df.getDefiniteFocus(protocolIndex,true);
	
	HANDLE electroThread=(HANDLE) _beginthreadex(NULL,0,&electrophoresisThread,&s,0,&threadID);
	DuplicateHandle(GetCurrentProcess(),electroThread,GetCurrentProcess(),&electroThreadCopy,0,false,DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(electroThread);
	s.getChan(0).out->cam->t->waitGenerationStart();
	cout<<"System Time At Generation:"<<Timer::getSysTime()<<endl;
	Timer t(true);
	t.waitTotal(1000.0*(delaySecs-1));
	//focus one second before voltage applied
	//cont.df.getDefiniteFocus(protocolIndex,false);

	
	//focus and apply voltage
	t.waitTotal(1000.0*(delaySecs));
	cont.daqpci.setVoltage(6,10);
	cont.df.getDefiniteFocus(protocolIndex,false);
	
	logFile.write(string("Voltage Applied at ")+t.toString());

	//focus one second after voltage applied
	t.waitTotal(1000.0*(delaySecs+1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	//focus one second before voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs-1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	t.waitTotal(1000.0*(delaySecs+durationSecs));
	cont.daqpci.setVoltage(6,0);
	cont.df.getDefiniteFocus(protocolIndex,false);
	

	//focus one second after voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs+1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	WaitForSingleObject(electroThreadCopy,INFINITE);

	cout<<"Total time was "<<t.toString()<<endl;
	cont.stg->move(x,y);
	CloseHandle(electroThreadCopy);
}
void ProtocolEric::electrophoresis2(customProtocol& scan){
	string folder;
	string answer;
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	cout<<"Enter total time for imaging (seconds)"<<endl;
	double imageSecs=getDouble();
	cout<<"Enter delay before E-field (seconds)"<<endl;
	double delaySecs=getDouble();
	cout<<"Enter duration of E-field (seconds)"<<endl;
	double durationSecs=getDouble();
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>answer;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (answer!="c") return;
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	cont.stg->wait();
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",imageSecs);
	HANDLE electroThreadCopy;
	unsigned int threadID;
	logFile.write(string("Initial Delay:")+toString(delaySecs)+" Voltage Duration:"+toString(durationSecs)+" Total Time:"+toString(imageSecs));

	//continuous definite focus
	//cont.df.startDefiniteFocus(protocolIndex,0);

	//focus once before
	cont.df.getDefiniteFocus(protocolIndex,true);
	cont.df.getDefiniteFocus(protocolIndex,true);
	HANDLE electroThread=(HANDLE) _beginthreadex(NULL,0,&electrophoresisThread,&s,0,&threadID);
	DuplicateHandle(GetCurrentProcess(),electroThread,GetCurrentProcess(),&electroThreadCopy,0,false,DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(electroThread);
	s.getChan(0).out->cam->t->waitGenerationStart();
	cout<<"System Time At Generation:"<<Timer::getSysTime()<<endl;
	Timer t(true);
	t.waitTotal(1000.0*(delaySecs-1));
	//focus one second before voltage applied
	//cont.df.getDefiniteFocus(protocolIndex,false);

	
	//focus before applying voltage
	t.waitTotal(1000.0*(delaySecs));
	//cont.df.getDefiniteFocus(protocolIndex,true);
	cont.daqpci.setVoltage(6,10);
	logFile.write(string("Voltage Applied at ")+t.toString());

	//focus one second after voltage applied
	t.waitTotal(1000.0*(delaySecs+.5));
	cont.df.getDefiniteFocus(protocolIndex,false);

	//focus one second before voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs-1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	t.waitTotal(1000.0*(delaySecs+durationSecs));
	cont.daqpci.setVoltage(6,0);

	//focus one second after voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs+0.5));
	cont.df.getDefiniteFocus(protocolIndex,false);

	WaitForSingleObject(electroThreadCopy,INFINITE);
	cout<<"Total time was "<<t.toString()<<endl;
	cont.stg->move(x,y);
	CloseHandle(electroThreadCopy);
}

unsigned __stdcall ProtocolEric::dfThread(void* param){
	cont.df.getDefiniteFocus(protocolIndex,true);
	return 0;
}

unsigned __stdcall ProtocolEric::kineticsScanThread(void* param){
	SpotSeriesScan* s=(SpotSeriesScan*) param;
	Timer t(true);
	s->runScan(1,true);
	t.stopTimer();
	logFile.write(string("Scan completed in ")+toString(t.getTime()/1000)+"sec.",true);
	return 0;
}

void ProtocolEric::kineticsScan(customProtocol& scan){
	string folder;
	string answer;
	if (!scan.cham.isX)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	if (scan.FOVChannels.front().front().m->obj.isOil)
		cont.stg->setSpeed(OILVELOCITY);
	scan.cham.move(scan.channelNum,0.5,scan.fractionLength);
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	string file=WORKINGDIR+"\\customScans\\"+protocolFiles[protocolIndex-1];
	string newFile=cont.workingDir+protocolFiles[protocolIndex-1];
	CopyFile(file.c_str(),newFile.c_str(),false);
	::DefiniteFocus dfs;
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",scan.imagingTime,0,(ScanFocus*)&dfs);
	s.enableStartEvent();
	unsigned int threadID;
	
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>answer;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (answer!="c") {
		//go back
		cont.stg->move(x,y);
		cont.stg->setSpeed(MAXVELOCITY);
		return;
	}
	//live view to select focus...press n for new field, press y to initialize DF
	int numFieldMoves=1;//starting is 1 because we will always move at least one space away to do the experiment
	cont.stg->wait();
	cont.setCurrentChannel(scan.coarse.a);
	cont.currentChannel().out->cam->startLiveView();
	bool dfSuccess=false;
NEXT:	cout<<"Enter 'y' to initialize Definite Focus or 'n' for new field.....('a' to abort)"<<endl;
	while(true){
		char c=::getChar();
		if (c=='y'){
			int i=0;
			while(i<5){
				Timer dfInit(true);
				cont.df.initializeDefiniteFocus(protocolIndex);
				dfInit.stopTimer();
				i++;
				if (dfInit.getTime()<scan.DFInitTimeout*1000.0){//less than 1 second
					int j=0;
					while(j<5){
						j++;
						Timer dfTest(true);
						cont.df.getDefiniteFocus(protocolIndex,true);
						dfTest.stopTimer();
						if (dfTest.getTime()>scan.DFDelay*1000.0){
							logFile.write("Definite Focus initialization doesn't seem to be that great...focusing is taking too long, maybe try a new field?",true);
							goto NEXT;
						}
					}
					dfSuccess=true;
					break;
				}
			}
			if (dfSuccess)
				break;
			logFile.write("Definite Focus Initialization took too long, maybe try moving to a new field?",true);
			goto NEXT;
		} else if (c=='n'){
			if (!scan.cham.isX)//-y
				cont.stg->stepDown();
			else 
				cont.stg->stepLeft();
			numFieldMoves++;
		} else if (c=='a'){
			cont.stg->move(x,y);
			cont.stg->setSpeed(MAXVELOCITY);
			return;
		}
	}
	cont.currentChannel().out->cam->stopLiveView();
	if (!scan.cham.isX)//-y
		cont.stg->stepDown();
	else 
		cont.stg->stepLeft();
	cont.stg->wait();
	//pull wash solution in case we need to recycle reagent
	double loadSpeeduLps=(scan.loadingVolume.at(scan.channel-1)-scan.cham.getChannelVolume())/scan.loadTimeSec;
	HANDLE scanThread=(HANDLE) _beginthreadex(NULL,0,&kineticsScanThread,&s,0,&threadID);
	cout<<"Prepare to pull 1mL of wash solution in case we need to recycle reagent"<<endl;
	system("pause");
	//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(scan.channelNum-1),1000,loadSpeeduLps*10),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
	//pull air gap
	cout<<"Prepare to pull air gap"<<endl;
	system("pause");
	double airGapVolume=scan.airGapmmChannel*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000;
//	cont.pmp.sendCommandNoExecute(cont.pmp.s.pull(cont.pmp.reagents.at(scan.channelNum-1),airGapVolume,loadSpeeduLps),cont.pmp.s.deviceNum);
	Timer pmpDelayAir(true);
	//cont.pmp.executeCommand(cont.pmp.s.deviceNum);
//	cont.pmp.wait();
	pmpDelayAir.stopTimer();
	double t1=pmpDelayAir.getTime();
	double t2=0;//cont.pmp.s.vol2stepsInt(airGapVolume)*1000/cont.pmp.s.flow2HzInt(loadSpeeduLps);
//	logFile.write(string("Pump communication delay for airGap command was ")+toString(pmpDelayAir.getTime()-cont.pmp.s.vol2stepsInt(airGapVolume)*1000/cont.pmp.s.flow2HzInt(loadSpeeduLps))+"ms",true);
	//load reagent
	cout<<"Prepare to pull reagent"<<endl;
	system("pause");
	double startCushionVol=scan.startCushion*scan.mmpsChannelSpeed*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000;
	double DFDelayVol=scan.DFDelay*scan.mmpsChannelSpeed*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000;
	double channelFractionVol=(scan.cham.getChannelLength()*scan.fractionLength+numFieldMoves*scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/1000)*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000;
	double loadingVol;
	if (startCushionVol>airGapVolume){
		logFile.write("Start Cushion Volume is greater than Air Gap Volume so imaging will begin in wash solution rather than air",true);
		loadingVol=scan.loadingVolume.at(scan.channelNum-1)+channelFractionVol-DFDelayVol-startCushionVol;
	}else{
		logFile.write("Air Gap Volume is greater than Start Cushion Volume so imaging will begin in air",true);
		loadingVol=scan.loadingVolume.at(scan.channelNum-1)+channelFractionVol-DFDelayVol-airGapVolume;
	}	
	
	//cont.pmp.sendCommandNoExecute(cont.pmp.s.pull(cont.pmp.reagents.at(scan.channelNum-1),loadingVol,loadSpeeduLps),cont.pmp.s.deviceNum);
	Timer pmpDelayReagent(true);
	//cont.pmp.executeCommand(cont.pmp.s.deviceNum);
	//cont.pmp.wait();
	pmpDelayReagent.stopTimer();
	double t3=pmpDelayReagent.getTime();
	double t4=0;//cont.pmp.s.vol2stepsInt(loadingVol)*1000/cont.pmp.s.flow2HzInt(loadSpeeduLps);
//	logFile.write(string("Pump communication delay for Reagent loading command was ")+toString(pmpDelayReagent.getTime()-cont.pmp.s.vol2stepsInt(loadingVol)*1000/cont.pmp.s.flow2HzInt(loadSpeeduLps))+"ms",true);
	//send pull command to start experiment;
	double totalFlowVol=scan.totalReagentFlow*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000+channelFractionVol-(loadingVol-scan.loadingVolume.at(scan.channelNum-1));
	double reagentSpeeduLps=scan.mmpsChannelSpeed*scan.cham.getChannelWidth()*scan.cham.getChannelHeight()/1000;
	//cont.pmp.sendCommandNoExecute(cont.pmp.s.pull(cont.pmp.reagents.at(scan.channelNum-1),totalFlowVol,reagentSpeeduLps),cont.pmp.s.deviceNum);
	//execute command
	int start=0;//cont.pmp.s.getPosition();
	//cont.pmp.executeCommand(cont.pmp.s.deviceNum);
	Timer swatch(true);
	//perform DF focusing
	
	
	
	
	HANDLE dfT=(HANDLE) _beginthreadex(NULL,0,&dfThread,NULL,0,&threadID);
	if (WAIT_TIMEOUT==WaitForSingleObject(dfT,scan.DFDelay*1000.0)){
		//stop pump immediately
		cont.pmp.stop();
		cont.df.stopDefiniteFocus();
		//cont.pmp.wait();
		int end=0;//cont.pmp.s.getPosition();
		//logFile.write(string("DF did not finish in specified time. Reagent is ")+toString(cont.pmp.s.steps2vol(start-end)*1000.0/scan.cham.channelWidth/scan.cham.channelHeight)+"mm from FOV. Recovering reagent.", true);
		system("pause");
		//push back to state just after loading
		//cont.pmp.sendCommand(cont.pmp.s.push(cont.pmp.reagents.at(scan.channelNum-1),cont.pmp.s.steps2vol(start-end),loadSpeeduLps),cont.pmp.s.deviceNum);
		//push back to state just before loading
		//cont.pmp.sendCommand(cont.pmp.s.push(cont.pmp.reagents.at(scan.channelNum-1),loadingVol,loadSpeeduLps),cont.pmp.s.deviceNum);
		cout<<"Pushing out 1mL of wash solution"<<endl;
		system("pause");
		//push air gap
		//cont.pmp.sendCommand(cont.pmp.s.push(cont.pmp.reagents.at(scan.channelNum-1),airGapVolume,loadSpeeduLps),cont.pmp.s.deviceNum);
		//push 1mL wash
		//cont.pmp.sendCommand(cont.pmp.s.push(cont.pmp.reagents.at(scan.channelNum-1),1000,loadSpeeduLps),cont.pmp.s.deviceNum);
		//cont.pmp.wait();
		cont.stg->move(x,y);
		cont.stg->setSpeed(MAXVELOCITY);
		return;
	}
	//start scan with startCushion
	double time;//in sec
	if (startCushionVol>airGapVolume){
		time=scan.DFDelay;
	}else{
		time=(DFDelayVol+airGapVolume-startCushionVol)/reagentSpeeduLps;
	}	
	swatch.waitTotal(time*1000);
	s.sendStartEvent();
	WaitForSingleObject(scanThread,INFINITE);
	//pump should stop automatically if it was set to finish before scan completes
	//if not we will stop it now
	cont.pmp.stop();
	logFile.write("Kinetics Scan complete",true);
	cont.stg->move(x,y);
	cont.stg->setSpeed(MAXVELOCITY);
}

void ProtocolEric::electrophoresis(customProtocol& scan){
	string folder;
	string answer;
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	cout<<"Enter total time for imaging (seconds)"<<endl;
	double imageSecs=getDouble();
	cout<<"Enter delay before E-field (seconds)"<<endl;
	double delaySecs=getDouble();
	cout<<"Enter duration of E-field (seconds)"<<endl;
	double durationSecs=getDouble();
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>answer;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (answer!="c") return;
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	cont.stg->wait();
	SpotSeriesScan s(scan.FOVChannels.front().front(),"Scan",imageSecs);
	HANDLE electroThreadCopy;
	unsigned int threadID;
	logFile.write(string("Initial Delay:")+toString(delaySecs)+" Voltage Duration:"+toString(durationSecs)+" Total Time:"+toString(imageSecs));

	//continuous definite focus
	cont.df.startDefiniteFocus(protocolIndex,0);

	//focus once before
	//cont.df.getDefiniteFocus(protocolIndex,true);
	
	HANDLE electroThread=(HANDLE) _beginthreadex(NULL,0,&electrophoresisThread,&s,0,&threadID);
	DuplicateHandle(GetCurrentProcess(),electroThread,GetCurrentProcess(),&electroThreadCopy,0,false,DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(electroThread);
	s.getChan(0).out->cam->t->waitGenerationStart();
	cout<<"System Time At Generation:"<<Timer::getSysTime()<<endl;
	Timer t(true);
	t.waitTotal(1000.0*(delaySecs-1));
	//focus one second before voltage applied
	//cont.df.getDefiniteFocus(protocolIndex,false);

	
	//focus before applying voltage
	t.waitTotal(1000.0*(delaySecs));
	//cont.df.getDefiniteFocus(protocolIndex,true);
	
	logFile.write(string("Voltage Applied at ")+t.toString());

	//focus one second after voltage applied?
	t.waitTotal(1000.0*(delaySecs+1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	//focus one second before voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs-1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	t.waitTotal(1000.0*(delaySecs+durationSecs));
	cont.daqpci.setVoltage(6,0);

	//focus one second after voltage turned off
	t.waitTotal(1000.0*(delaySecs+durationSecs+1));
	//cont.df.getDefiniteFocus(protocolIndex,false);

	WaitForSingleObject(electroThreadCopy,INFINITE);
	cont.df.stopDefiniteFocus();
	cout<<"Total time was "<<t.toString()<<endl;
	cont.stg->move(x,y);
	CloseHandle(electroThreadCopy);
}

unsigned __stdcall ProtocolEric::electrophoresisThread(void* param){
	SpotSeriesScan* ssc=(SpotSeriesScan*) param;
	ssc->runScan(1,true);
	return 0;
}

unsigned __stdcall ProtocolEric::omnicureThread(void* param){

	Ludl* lud=(Ludl*) cont.stg;
	lud->setSpeed(HOMINGVELOCITYFINE);
	while(omnicureOscillating){
	cont.stg->moveX(omnicureStart);
	cont.stg->wait();
	if (!omnicureOscillating) break;
	cont.stg->moveX(omnicureEnd);
	cont.stg->wait();
	}
	lud->setSpeed(MAXVELOCITY);
	return 0;
}


bool getAcquisitionChannel(string line,AcquisitionChannel& channel){
	stringstream ss;
	int chan;
	ss<<line;
	//channel
	getline(ss,line,',');
	removeWhite(line);
	if (line=="BF")
		chan=cont.BF;
	else if (line=="DAPI")
		chan=cont.DAPI;
	else if (line=="FITC")
		chan=cont.FITC;
	else if (line=="Cy3")
		chan=cont.CY3;
	else if (line=="Cy3.5")
		chan=cont.CY35;
	else if (line=="Cy5")
		chan=cont.CY5;
	else if (line=="Cy5.5")
		chan=cont.CY55;
	else if (line=="Qdot705")
		chan=cont.QDOT705;
	else if (line=="Cy7")
		chan=cont.CY7;
	else if (line=="BFPINKEL")
		chan=cont.BFPINKEL;
	else if (line=="DAPIPINKEL")
		chan=cont.DAPIPINKEL;
	else if (line=="FITCPINKEL")
		chan=cont.FITCPINKEL;
	else if (line=="Cy3PINKEL")
		chan=cont.CY3PINKEL;
	else if (line=="Cy5PINKEL")
		chan=cont.CY5PINKEL;
	else if (line=="FRET405488")
		chan=cont.FRET405488;
	else if (line=="FRET425594")
		chan=cont.FRET425594;
	else if (line=="FRETFITCCy3.5")
		chan=cont.FRETFITCCy35;
	else if (line=="FRETCy3Cy5")
		chan=cont.FRETCy3Cy5;
	else if (line=="FRETCy3Cy5Sem")
		chan=cont.FRETCy3Cy5Sem;
	else if (line=="FRETCy3Cy5Thor")
		chan=cont.FRETCy3Cy5Thor;
	else if (line=="TIRF405")
		chan=cont.TIRF405;
	else if (line=="TIRF488")
		chan=cont.TIRF488;
	else if (line=="TIRF532")
		chan=cont.TIRF532;
	else if (line=="TIRF532_EX")
		chan=cont.TIRF532_EX;
	else if (line=="TIRF642")
		chan=cont.TIRF642;
	else if (line=="TIRF642_EX")
		chan=cont.TIRF642_EX;
	else if (line=="TIRF660")
		chan=cont.TIRF660;
	else if (line=="EPI405")
		chan=cont.EPI405;
	else if (line=="EPI488")
		chan=cont.EPI473;
	else if (line=="EPI532")
		chan=cont.EPI532;
	else if (line=="EPI642")
		chan=cont.EPI642;
	else if (line=="EPI660")
		chan=cont.EPI660;
	else {
		logFile.write(string("Error parsing protocol file.  Unsupported channel. Input was ")+line,true);
		return false;
	}
	if (chan<0 || chan>cont.channels.size()){
		logFile.write(string("Active Configuration does not support this channel. Run MTBConfig.exe to change configuration. Input was ")+line,true);
		return false;
	}
	//mag
	int mag;
	getline(ss,line,',');
	mag=Magnification::getMagnification(line);
	if (mag==-1){
		logFile.write(string("Error parsing protocol file.  Unsupported magnification.  Input was ")+line,true);
		return false;
	}
	if (mag<0 || chan>cont.channels.size()){
		logFile.write(string("Active Configuration does not support this magnification. Check objective position. Input was ")+line,true);
	}
	//exp
	double exp;
	getline(ss,line,',');
	exp=toDouble(line);
	if (ss.fail()){
		logFile.write("Must specify an exposure time for this acquisition channel",true);
		return false;
	}
	//gain
	vector<int> gains;
	stringstream gs;
	if (getline(ss,line,',')){
		gs<<line;
		string gainStr;
		while(getline(gs,gainStr,':')){
		gains.push_back(toInt(gainStr));
		}
	}else{
		logFile.write("Must specify a gain for this acquisition channel",true);
		return false;
	}
	//bin
	int bin;
	if (getline(ss,line,','))
		bin=toInt(line);
	else
		bin=1;
	//intensity
	double intensity;
	string unit;
	if (getline(ss,line,',') && getline(ss,unit,','))
		intensity=toDouble(line);		
	else{
		intensity=cont.channels.at(chan).lite().ls->getIntensity();
		unit="";
	}
	//showOnScreen
	bool showOnScreen;
	if (getline(ss,line,',')){
		if (line=="true")
			showOnScreen=true;
		else if (line=="false")
			showOnScreen=false;
		else {
			logFile.write(string("Error parsing protocol file.  ShowOnScreen must be either true or false.  Input was ")+line,true);
			return false;
		}
	}else
		showOnScreen=false;
	double TIRFangle;
	if (getline(ss,line,',')){
		TIRFangle=toDouble(line);
	}else
		TIRFangle=-20;
	getline(ss,line,',');
	int out=cont.OUT_CAM;
	if (ss.fail())
		channel=AcquisitionChannel(chan,cont.OUT_CAM,mag,AcquisitionParameters(exp,gains,bin),intensity,unit,showOnScreen,false,TIRFangle);
	else{
		if (line=="CAM")
			out=cont.OUT_CAM;
		else if (line=="CAM_LEFT")
			out=cont.OUT_CAM_LEFT;
		else if (line=="CAM_RIGHT")
			out=cont.OUT_CAM_RIGHT;
		else if (line=="CAM_RIGHTTRANS")
			out=cont.OUT_CAM_RIGHTTRANS;
		else if (line=="CAM_RIGHTREFL")
			out=cont.OUT_CAM_RIGHTREFL;
		else if (line=="CAM_DUAL")
			out=cont.OUT_CAM_DUAL;
		else if (line=="OCULARS")
			out=cont.OUT_OCULARS;
		else if (line=="BLOCKED")
			out=cont.OUT_BLOCKED;
		else if (line=="CAM_OCULAR")
			out=cont.OUT_CAM_OCULAR;
		else{
			logFile.write(string("Error parsing protocol file.  OUTPORT was not valid. Input was ")+line,true);
			return false;
		}
	}
	getline(ss,line,',');
	
	if (ss.fail())
		channel=AcquisitionChannel(chan,out,mag,AcquisitionParameters(exp,gains,bin),intensity,unit,showOnScreen,false,TIRFangle);
	else{
		double minX=toDouble(line);
		getline(ss,line,',');
		double minY=toDouble(line);
		getline(ss,line,',');
		double maxX=toDouble(line);
		getline(ss,line,',');
		double maxY=toDouble(line);
		if (ss.fail()){
			logFile.write(string("Error parsing protocol file.  image region was not valid. Input was ")+line,true);
			return false;
		}
		channel=AcquisitionChannel(chan,out,mag,AcquisitionParameters(exp,gains,bin,ImageRegion(minX,minY,maxX,maxY)),intensity,unit,showOnScreen,false,TIRFangle);
	}
	return true;
}


bool ProtocolEric::loadProtocol(string filename,customProtocol& scanReal){
	customProtocol scan;
	scan.FOVChannels.clear();
	std::ifstream protocol;
	protocol.open(filename.c_str(),ifstream::in);
	if (protocol.fail()){
		logFile.write(string("Error could not find protocol file: ")+filename,true);
		protocol.close();
		return false;
	}
	string line;
	stringstream ss;
	bool b=false;
/*	//directory
	line=getRealLine(protocol);
	directory=line;
*/

	//chamber or cham
	//Chamber cham;
	double maxX, maxY, maxZ, focusZ, mmChannelSpacing,mmChannelLength,mmChannelWidth,uLChannelVolume,tubingDiameter;
	vector<double> loadingVolume;
	int numChannels;
	line=getRealLine(protocol);
	ss=stringstream();//=stringstream();//clear();
	ss<<line;
	getline(ss,line,',');
	if (line=="Left")
		scan.chamber=0;
	else if (line=="Right")
		scan.chamber=1;
	else {
		//maxX
		maxX=::toDouble(line,b);
		if (!b){
			logFile.write(string("Error loading protocol:  chamber must be Left or Right or defined by multiple double parameters.  Input was ")+line,true);
			protocol.close();
			return false;
		}
		//maxY
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			maxY=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber maxY could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//maxZ
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			maxZ=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber maxZ could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//focusZ
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			focusZ=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber focusZ could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//mmChannnelSpacing
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			mmChannelSpacing=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber mmChannelSpacing could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//mmChannelLength
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			mmChannelLength=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber mmChannelLength could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//mmChannelWidth
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			mmChannelWidth=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber mmChannelWidth could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//uLChannelVolume
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{

			uLChannelVolume=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber uLChannelVolume could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//numChannels
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			numChannels=(int) ::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  chamber numChannels could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		scan.cham=Chamber(maxX,maxY,maxZ,focusZ,mmChannelSpacing,mmChannelLength,mmChannelWidth,uLChannelVolume,numChannels,true);
	 }
	
	//channel or pump setup parameters
	line=getRealLine(protocol);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	if (line=="Left")
		scan.channel=0;
	else if (line=="Center")
		scan.channel=1;
	else if (line=="Right")
		scan.channel=2;
	else {
		//loading Volume
		loadingVolume=vector<double>(numChannels,::toDouble(line,b));
		if (!b){
			logFile.write(string("Error loading protocol:  channel or pump setup parameters are incorrect.  Input was ")+line,true);
			protocol.close();
			return false;
		}
		int i=0;
		bool only2=false;
		while (true){
			i++;
			if (i>=numChannels) break;
			getline(ss,line,',');
			if (ss.fail())
				if (i!=2){
					logFile.write(string("Error loading protocol:  not enough loading volumes"),true);
					protocol.close();
					return false;
				}else{ 
					only2=true;
					tubingDiameter=loadingVolume.at(1);
					loadingVolume.at(1)=loadingVolume.at(0);
					break;
				}
			loadingVolume.at(i)=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  channel or pump setup parameters are incorrect.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}

		//tubing diameter
		if (!only2){
			getline(ss,line,',');
			if (ss.fail()){
				logFile.write("Protocol file error: no tubing diameter specified",true);
				protocol.close();
				return false;
			}else{
				tubingDiameter=::toDouble(line,b);
				if (!b){
					logFile.write(string("Error loading protocol:  tubing diameter could not be parsed.  Input was ")+line,true);
					protocol.close();
					return false;
				}
			}
		}
	}
		
	
	
	//section or pump experiment parameters
	line=getRealLine(protocol);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	if (line=="Top")
		scan.section=2;
	else if (line=="Middle")
		scan.section=1;
	else if (line=="Bottom")
		scan.section=0;
	else if (line=="Current")
		scan.section=3;
	else{
		//totalReagentFlow
		scan.totalReagentFlow=::toDouble(line,b);
		if (!b){
			logFile.write(string("Error loading protocol:  channel or pump experiment paramters are incorrect.  Input was ")+line,true);
			protocol.close();
			return false;
		}
		//totalImagingTime
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in pump experiment parameters not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.imagingTime=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  startCushion could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//startCushion
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.startCushion=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  startCushion could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//DFDelay
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.DFDelay=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  DFDelay could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//DFInitTimeout
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.DFInitTimeout=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  DFInitTimeout could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//airGap
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.airGapmmChannel=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  AirGap could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//mmpsChannelSpeed
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.mmpsChannelSpeed=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol: ChannelSpeed could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//loadTimeSec
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.loadTimeSec=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  loadTimeSec could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//wash volume
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.washNumChannelVolumes=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  washNumChannelVolumes could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//wash speed
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.washSpeedmmpsChannel=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  washSpeedmmpsChannel could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//loadNumChannelVolumes
		getline(ss,line,',');
		if (ss.fail()){
			logFile.write("Protocol file error in chamber definition not enough parameters",true);
			protocol.close();
			return false;
		}else{
			scan.loadNumChannelVolumes=::toDouble(line,b);
			if (!b){
				logFile.write(string("Error loading protocol:  loadNumChannelVolumes could not be parsed.  Input was ")+line,true);
				protocol.close();
				return false;
			}
		}
		//optional change of syringe default volume
		bool v=true;
		double syringeVolume;
		double devNum;
		while (v){
			getline(ss,line,':');
			if (!ss.fail()) {
				//specified syringe volum
				devNum=toInt(line);
				getline(ss,line,',');
				if(!ss.fail())
					syringeVolume=toDouble(line,b);
				else{
					logFile.write(string("Error loading protocol: syringe device number specified but no volume given"),true);
					protocol.close();
					return false;
				}
				cont.pmp.getSyringe(devNum)->volume=syringeVolume;
			}else
				v=false;	
		}
	}
	
	//scan direction and experiment location
	line=getRealLine(protocol);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	if (line=="+y"){
		scan.direction=0;
		scan.cham.isX=false;
	}
	else if (line=="-y"){
		scan.direction=1;
		scan.cham.isX=false;
	}
	else if (line=="+x"){
		scan.direction=2;
		scan.cham.isX=true;
	}
	else if (line=="-x"){
		scan.direction=3;
		scan.cham.isX=true;
	}
	else{
		logFile.write(string("Error loading protocol: Scan direction must be +y, -y, +x, or -x.  Input was ")+line, true);
		protocol.close();
		return false;
	}
	getline(ss,line,',');
	if (ss.fail())
		scan.moveDelay=3000;
	else{
		scan.moveDelay=toDouble(line);
		getline(ss,line,',');
		if (ss.fail()){
			scan.channelNum=-1;
			scan.fractionLength=-1;
		}else{
			scan.channelNum=toInt(line);
			getline(ss,line,',');
			if (ss.fail()){
				scan.fractionLength=-1;
			}else{
				scan.fractionLength=toDouble(line);
			}
			
		}
	}
	

	//FOVfocus
	line=getRealLine(protocol);
	scan.FOVfocus=toDouble(line);
	//FOVscan
	line=getRealLine(protocol);
	scan.FOVscan=toDouble(line);
	//FocusAcquisitionChannel
	line=getRealLine(protocol);
	AcquisitionChannel chan;
	if(!getAcquisitionChannel(line,chan)){
		protocol.close();
		return false;
	}

	//coarse focus
	line=getRealLine(protocol);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	double foc1,foc2,foc3;
	foc1=toDouble(line);
	scan.coarse=Focus(chan,foc1);
	//initial focus
	getline(ss,line,',');
	foc2=toDouble(line);
	scan.initial=Focus(chan,foc2);
	//final focus
	getline(ss,line,',');
	foc3=toDouble(line);
	scan.final=Focus(chan,foc3);
	//focus Bleach compensation
	getline(ss,line,',');
	if (line=="true")
		scan.focusBleachCompensation=true;
	else if (line=="false")
		scan.focusBleachCompensation=false;
	else{
		logFile.write(string("Error loading protocol:  Focus bleach compensation must be true or false.  Input was ")+line,true);
		protocol.close();
		return false;
	}
	//Spots
	string lineHeader;
	vector<AcquisitionChannel> spot;
	while(!protocol.eof()){
		line=getRealLine(protocol);
		ss=stringstream();
		ss<<line;
		getline(ss,lineHeader,':');
		if (line==""|| lineHeader=="FluidicsSetup" || lineHeader=="DaisyValve"||lineHeader=="Solution"||lineHeader=="Reagent"||lineHeader=="FlowChannel")
			break;
		if (line=="New Spot"){
			//so we can move without taking pic//if (spot.size()==0) continue;
			scan.FOVChannels.push_back(spot);
			spot.clear();
			continue;
		}else {
			if (!getAcquisitionChannel(line,chan)){
				protocol.close();
				return false;
			}
			spot.push_back(chan);
		}
	}
	


	//parse and create fluidics setup
	
	DaisyValve dvPull(&cont.pmp.dummyValve,&cont.pmp.dummyValve);
	DaisyValve dvPush(&cont.pmp.dummyValve,&cont.pmp.dummyValve);
	SolutionData sTempPull,rTempPull,sTempPush,rTempPush;
	FlowChannel fTemp(cont.pmp.getSyringe(),scan.cham,&cont.pmp.dummyValve);
	string defaultWashPull="PullWash";
	string defaultWashPush="PushWash";

	if (lineHeader=="FluidicsSetup"){
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default solution flow rate specified
			sTempPull.uLps=toDouble(line);
			sTempPush.uLps=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default solution inletTubingVol specified
			sTempPull.inletTubingVol=toDouble(line);
			sTempPush.inletTubingVol=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default solution loadFactor specified
			sTempPush.loadFactor=toDouble(line);
			sTempPull.loadFactor=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default reagent flow rate specified
			rTempPush.uLps=toDouble(line);
			rTempPull.uLps=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default reagent inletTubingVol specified
			rTempPush.inletTubingVol=toDouble(line);
			rTempPull.inletTubingVol=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default reagent loadFactor specified
			rTempPush.loadFactor=toDouble(line);
			rTempPull.loadFactor=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default flowChannel injectTubingVolumePull specified
			fTemp.injectTubingVolumePull=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default flowChannel injectTubingVolumePush specified
			fTemp.injectTubingVolumePush=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default dedicated washpull specified
			defaultWashPull=line;
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default dedicated washpush specified
			defaultWashPush=line;
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default flowChannel pull valve devNum specified
			fTemp.vPull=cont.pmp.getValve(toInt(line));
			if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(fTemp.vPull)){
				logFile.write("parse error: cannot find channel multiplexing pull valve",true);
				protocol.close();
				return false;
			}
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default flowChannel push valve devNum specified
			fTemp.vPush=cont.pmp.getValve(toInt(line));
			if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(fTemp.vPush)){
				logFile.write("parse error: cannot find channel multiplexing push valve",true);
				protocol.close();
				return false;
			}
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default syringe devNum specified
			fTemp.s=cont.pmp.getSyringe(toInt(line));
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default syringe outport specified
			fTemp.syringePos=toInt(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
			//default syringe volume specified
			fTemp.s->volume=toDouble(line);
		}
		getline(ss,line,',');
		if (!ss.fail() && removeWhite(line)!=""){
				logFile.write("parse error: FluidicsSetup has too many inputs",true);
				protocol.close();
				return false;
			}
	}
	
	if (lineHeader=="FluidicsSetup"){
		line=getRealLine(protocol);
		ss=stringstream();
		ss<<line;
		getline(ss,lineHeader,':');
	}

		//fix default daisy valve
	if (!cont.pmp.isDummyValve(fTemp.vPull) && fTemp.vPull!=cont.pmp.get2ndValve()){
		if (fTemp.vPull == dvPull.v1){
			dvPull.v1=NULL;
			dvPull.v2=NULL;
		}
		if (fTemp.vPull == dvPull.v2){
			dvPull.v2=NULL;
		}
	}

	int numdvPush=0;
	int numdvPull=0;
	while(lineHeader=="DaisyValvePush" || lineHeader=="DaisyValvePull"){
		if (lineHeader=="DaisyValvePush"){
			numdvPush++;
			getline(ss,line);
			dvPush.parse(line);
		}else if (lineHeader=="DaisyValvePull"){
			numdvPull++;
			getline(ss,line);
			dvPull.parse(line);
		}
			line=getRealLine(protocol);
			ss=stringstream();
			ss<<line;
			getline(ss,lineHeader,':');
	}
	if (numdvPush>1){
				logFile.write("Only one Push-type daisy valve is allowed",true);
				protocol.close();
				return false;
	}
	if (numdvPull>1){
				logFile.write("Only one Pull-type daisy valve is allowed",true);
				protocol.close();
				return false;
	}
	if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(dvPull.v1)){
		dvPull.v1=NULL;
		dvPull.v2=NULL;
	}
	if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(dvPull.v2)){
		dvPull.v2=NULL;
	}
	if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(dvPush.v1)){
		dvPush.v1=NULL;
		dvPush.v2=NULL;
	}
	if (!cont.pmp.isDummySyringe(fTemp.s) && cont.pmp.isDummyValve(dvPush.v2)){
		dvPush.v2=NULL;
	}
	if ((!cont.pmp.isDummyValve(dvPull.v1) && dvPull.v1!=NULL && (dvPull.v1==dvPull.v2 || dvPull.v1==fTemp.vPull)) || (!cont.pmp.isDummyValve(dvPull.v2) && dvPull.v2!=NULL && dvPull.v2==fTemp.vPull)){
		logFile.write("Conflict between device numbers for daisy valves and/or channel valve",true);
		protocol.close();
		return false;
	}
	if (lineHeader=="DaisyValve"){
		line=getRealLine(protocol);
		ss=stringstream();
		ss<<line;
		getline(ss,lineHeader,':');
	}
	SolutionData temp;
	try{
		dvPull=DaisyValve(dvPull.v1,dvPull.v2,dvPull.daisyPort,dvPull.daisyInjectTubingVol,dvPull.outPort,dvPull.injectTubingVol);
		dvPush=DaisyValve(dvPush.v1,dvPush.v2,dvPush.daisyPort,dvPush.daisyInjectTubingVol,dvPush.outPort,dvPush.injectTubingVol);
		sTempPull.dv=dvPull;
		rTempPull.dv=dvPull;
		sTempPush.dv=dvPush;
		sTempPush.syringePort=fTemp.syringePos;
		sTempPush.s=fTemp.s;
		rTempPush.dv=dvPush;
		rTempPush.syringePort=fTemp.syringePos;
		rTempPush.s=fTemp.s;
		fTemp.dvPull=dvPull;
		fTemp.dvPush=dvPush;
		while(line!=""){
			getline(ss,line);//get remaining line
			if (lineHeader=="PullSolution"){
				SolutionData temp(sTempPull);
				temp.parsePullSolution(line);
				if (!scan.fs.addSolution(PullSolution(temp))){
					logFile.write("Cannot add solution",true);
					protocol.close();
					return false;
				}
			}
			if (lineHeader=="PullReagent"){
				SolutionData temp(rTempPull);
				temp.wash=scan.fs.getSolution(defaultWashPull);
				temp.parsePullReagent(line,scan.fs);
				if (!scan.fs.addReagent(PullReagent(temp))){
					logFile.write("Cannot add reagent",true);
					protocol.close();
					return false;
				}
			}
			if (lineHeader=="PushSolution"){
				SolutionData temp(sTempPush);
				temp.parsePushSolution(line);
				if (!scan.fs.addSolution(PushSolution(temp))){
					logFile.write("Cannot add solution",true);
					protocol.close();
					return false;
				}
			}
			if (lineHeader=="PushReagent"){
				SolutionData temp(rTempPush);
				temp.wash=scan.fs.getSolution(defaultWashPush);
				temp.parsePushReagent(line,scan.fs);
				if (!scan.fs.addReagent(PushReagent(temp))){
					logFile.write("Cannot add reagent",true);
					protocol.close();
					return false;
				}
			}
			if (lineHeader=="FlowChannel"){
				if (!scan.fs.addChannel(FlowChannel(fTemp).parseLine(line))){
					logFile.write("Cannot add FlowChannel",true);
					protocol.close();
					return false;
				}
			}
			line=getRealLine(protocol);
			ss=stringstream();
			ss<<line;
			getline(ss,lineHeader,':');			
		}
	}catch(std::ios_base::failure e){
		logFile.write("Protocol Parse Error: Bad input",true);
		protocol.close();
		return false;
	}
	catch(exception& e){
		logFile.write("Protocol Parse Error: "+string(e.what()),true);
		protocol.close();
		return false;
	}
	if (!spot.empty())
		scan.FOVChannels.push_back(spot);
	scan.loadingVolume=loadingVolume;
	scan.tubingDiameterInches=tubingDiameter;
	scanReal=scan;//so we dont corrupt a good customProtocol if the new one doesn't parse properly
	return true;
}

bool ProtocolEric::reloadProtocolList(){
	protocolIndex=-1;
	protocolFiles.clear();
	cont.df.clearIndexedInitializationData();
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE; 
	string DirSpec=WORKINGDIR+"\\customScans\\*.txt";
	hFind = FindFirstFile(DirSpec.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE){
		protocolFiles.push_back(string(FindFileData.cFileName));
		while(FindNextFile(hFind, &FindFileData) != 0){
			protocolFiles.push_back(string(FindFileData.cFileName));
		}

		FindClose(hFind);
	}
	if (protocolFiles.empty()){
		logFile.write(string("Custom Scan: No protocols (*.txt) located in the folder")+WORKINGDIR+"\\customScans",true);
		return false;
	}
	return true;
}

bool ProtocolEric::selectProtocol(customProtocol& scan){
	if (protocolFiles.empty()){
		if (!reloadProtocolList())
			return false;
	}
	int n;
	while(true){
			cout<<"Please select a custom scan"<<endl;
			n=1;
			for(vector<string>::const_iterator i=protocolFiles.begin();i<protocolFiles.end();i++){
				cout<<n<<": "<<(*i)<<endl;
				n++;
			}
			cout<<"r: Reload protocol list (all definite focus stabilization data will be lost)"<<endl;
			cout<<"e: Exit"<<endl;
			string s;
			getline(cin,s);
			if (s=="e"){
				if (protocolIndex!=-1 && protocolIndex>0 && protocolIndex<=protocolFiles.size()){
					if (!loadProtocol(WORKINGDIR+"\\customScans\\"+protocolFiles[protocolIndex-1],scan)){
						protocolIndex=-1;
						return false;
					}
					return true;
				}
				protocolIndex=-1;
				return false;
			}
			if (s=="r"){
				reloadProtocolList();
				continue;
			}
			int sel=toInt(s);
			if (sel<1 || sel>n){
				cout<<"Invalid selection...try again"<<endl;
				continue;
			}	
			if (loadProtocol(WORKINGDIR+"\\customScans\\"+protocolFiles[sel-1],scan)){
				protocolIndex=sel;
				return true;
			}
		}
}



void ProtocolEric::customScan(){
	

	//string workingDir="default";
	if (protocolIndex==-1 || !loadProtocol(WORKINGDIR+"\\customScans\\"+protocolFiles[protocolIndex-1],scan)){
		if (!selectProtocol(scan))
			return;
	}
	int m;
	AcquisitionChannel getIt=cont.currentChannel();
	for(vector<vector<AcquisitionChannel>>::reverse_iterator i=scan.FOVChannels.rbegin();i!=scan.FOVChannels.rend();i++){
		for(vector<AcquisitionChannel>::reverse_iterator j=i->rbegin();j!=i->rend();j++){
			getIt=*j;
			cont.addDefaultAC(*j,m,m);
		}
	}
	cont.setCurrentChannel(getIt);
	cont.currentFocus=scan.coarse;
	if (currentChamber.maxZ==-1 && scan.cham.maxZ!=0){
		logFile.write(string("Chamber configuration has been defined, z focus is ")+::toString(scan.cham.focusZ)+" and specified channel is"+::toString(scan.channelNum),true);
		logFile.write("Do you want to move there (y or n)?",true);
		cont.axio.setMaxZ(scan.cham.maxZ);
		string yORn;
			while(true){
				yORn=::getString();
				if (yORn=="y"){
					cont.focus->move(scan.cham.focusZ);
					scan.cham.move(scan.channelNum,0.5,scan.fractionLength);
					break;
				}
				if (yORn=="n")
					break;
			}
	}
	currentChamber=scan.cham;
	
	//END Load Protocol//
	///////////////////////
	//logFile.write(string("Configuration file ")+WORKINGDIR+"\\customScans\\"+protocolFiles[protocolIndex-1]+" loaded successfully",true);
	char c;
	while(true){
		cout<<"Please select a task for this custom scan:"<<endl;
		cout<<"0: changed loaded configuration file (currently "<<protocolFiles[protocolIndex-1]<<")"<<endl;
		cout<<"1: change working directory (currently "<<cont.workingDir<<")"<<endl;
		cout<<"2: step forward in scan "<<scan.FOVscan<<" FOVs (";
		if (scan.direction==0 || scan.direction ==1)//y direction for scan
			getIt.getFOVHeight();
		else
			getIt.getFOVWidth();
		cout<<" microns)"<<endl;
		cout<<"3: step back in scan "<<scan.FOVscan<<" FOVs (";
		if (scan.direction==0 || scan.direction ==1)//y direction for scan
			cout<<scan.FOVscan*getIt.getFOVHeight();
		else
			cout<<scan.FOVscan*getIt.getFOVWidth();
		cout<<" microns)"<<endl;
		cout<<"4: Show FluidicsSetup"<<endl;
		cout<<"5: Run custom scan protocol (total move ";
		if (scan.direction==0 || scan.direction ==1)//y direction for scan
			cout<<scan.FOVscan*scan.FOVChannels.size()*getIt.getFOVHeight();
		else
			cout<<scan.FOVscan*scan.FOVChannels.size()*getIt.getFOVWidth();
		cout<<" microns)"<<endl;
		cout<<"6: Run fast kinetics (fixed period)"<<endl;
		cout<<"7: Run fast kinetics (arbitrary time points)"<<endl;
		cout<<"8: Run fast kinetics (arbitrary time points with no focusing for initial images)"<<endl;
		cout<<"9: Run fast kinetics (fixed period) with focus followed by imaging and then reagent load"<<endl; 
		cout<<"D: Erin's Electrophoresis Time Series (continuous Definite Focusing)"<<endl;
		cout<<"E: Erin's Electrophoresis Time Series 2 (JIT Definite Focusing 1 sec before and after voltage is turned on and turned off)"<<endl;
		cout<<"F: Erin's Electrophoresis Time Series 3 (JIT Definite Focusing immediately when voltage is turned on and turned off)"<<endl;
		cout<<"G: Run custom scan with E field on"<<endl;
		cout<<"K: Kinetics Scan"<<endl;
		cout<<"B: Bleach Spots"<<endl;
		cout<<"P: Photocleave Spots"<<endl;
		cout<<"R: step perpendicular to scan direction (90deg clockwise) "<<scan.FOVfocus<<" FOVs (";
		if (scan.direction==0 || scan.direction ==1)//y direction for scan
			cout<<scan.FOVscan*getIt.getFOVHeight();
		else
			cout<<scan.FOVscan*getIt.getFOVWidth();
		cout<<" microns)"<<endl;
		cout<<"L: step perpendicular to scan direction (90deg counterclockwise) "<<scan.FOVfocus<<" FOVs (";
		if (scan.direction==0 || scan.direction ==1)//y direction for scan
			cout<<scan.FOVscan*getIt.getFOVHeight();
		else
			cout<<scan.FOVscan*getIt.getFOVWidth();
		cout<<" microns)"<<endl;
		cout<<"M: Chamber move (specify channel number and position)"<<endl;
		cout<<"T: Run scan of multiple channels"<<endl;
		cout<<"Z: Load maxZ "<<scan.cham.maxZ<<" and move to focusZ "<<scan.cham.focusZ<<endl;
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

		switch(c){
			case '0':
				if (!selectProtocol(scan))
					return;
				for(vector<vector<AcquisitionChannel>>::reverse_iterator i=scan.FOVChannels.rbegin();i!=scan.FOVChannels.rend();i++){
		for(vector<AcquisitionChannel>::reverse_iterator j=i->rbegin();j!=i->rend();j++){
			getIt=*j;
			cont.addDefaultAC(*j,m,m);
		}
	}
	cont.setCurrentChannel(getIt);
	cont.currentFocus=scan.coarse;
				break;
				//		case '1':
				//			TIRFadjust();
				//			break;
				//		case '2':
				//			intensityAdjust();
				//			break;
			case '1':{
				string folder;
				cout<<"enter working folder (will be placed under "<<WORKINGDIR<<")"<<endl;
				cin>>folder;
				cin.clear();
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.setWorkingDir(folder);
				break;
					 }
			case '2':{
				if (scan.direction==0 || scan.direction ==1){//y direction for scan
					cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==0)//+y
						cont.stg->stepUp();
					else//-y
						cont.stg->stepDown();
				}else {//x direction for scan
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==2)//+x
						cont.stg->stepRight();
					else//-x
						cont.stg->stepLeft();
				}
				cont.stg->wait();
				break;
					 }
			case '3':{
				if (scan.direction==0 || scan.direction ==1){//y direction for scan
					cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==0)//+y
						cont.stg->stepDown();
					else//-y
						cont.stg->stepUp();
				}else {//x direction for scan
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==2)//+x
						cont.stg->stepLeft();
					else//-x
						cont.stg->stepRight();
				}
				cont.stg->wait();
				break;
					 }
					 case 'R':{//90 deg clockwise to scan direction
				if (scan.direction==0 || scan.direction ==1){//y direction for scan
					cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==0)//+y so +x is 90deg clockwise
						cont.stg->stepRight();
					else//-y so -x is 90deg clockwise
						cont.stg->stepLeft();
				}else {//x direction for scan
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
					if (scan.direction==2)//+x so 90deg clockwise is -y
						cont.stg->stepDown();
					else//-x so 90 deg clockwise is +y
						cont.stg->stepUp();
				}
				cont.stg->wait();
				break;
					 }
			case 'L':{//90 deg counterclockwise to scan direction
				if (scan.direction==0 || scan.direction ==1){//y direction for scan
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVfocus/cont.stg->getStepSize());
					if (scan.direction==0)//+y so 90deg counter clockwise is -x
						cont.stg->stepLeft();
					else//-y so 90 deg counter clockwise is +x
						cont.stg->stepRight();
				}else {//x direction for scan
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVfocus/cont.stg->getStepSize());
					if (scan.direction==2)//+x so 90 deg counter clockwise is +y
						cont.stg->stepUp();
					else//-x so 90 deg counter clockwise is -y
						cont.stg->stepDown();
				}
				cont.stg->wait();
				break;
					 }
			case '4':
				cout<<endl<<endl;
				logFile.write(scan.fs.toString(),true);
				cout<<endl<<endl;
				break;
			case '5':{
				string folder;
				cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
				std::getline(std::cin,folder);
				cont.setWorkingDir(folder);
				cout<<"press (c) to continue or (b) to go back"<<endl;
				cin>>folder;
				cin.clear();
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				if (folder!="c"){
					cont.setWorkingDir("default");
					break;
				}
				imageMultipleFOVs(scan);
				break;
					 }

			case '6':{
				timeSeries2(scan);
				}
				break;
			case '7':{
				timeSeries(scan);
				}
				break;
			case '8':{
				timeSeries3(scan);
				}
				break;
			case '9':{
				reagentKinetics(scan);
					 }
					 break;
			case 'D':
				electrophoresis(scan);
				break;
				case 'E':
				electrophoresis2(scan);
				break;
				case 'F':
				electrophoresis3(scan);
				break;
				case 'G':{
			string folder;
	string answer;
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	double initialDelay,totalDelay;
	cout<<"enter delay before Definite Focus (seconds)"<<endl;
	initialDelay=::getDouble();
	cout<<"enter minimum delay before imaging (seconds)"<<endl;
	totalDelay=::getDouble();
	logFile.write(string("position before move is X:")+toString(cont.stg->getX())+" Y:"+toString(cont.stg->getY()));
	logFile.write(string("Delay before DF is ")+toString(initialDelay)+" Total delay before imaging is "+toString(totalDelay));
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>answer;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (answer!="c") return;
	if (scan.direction==0)//+y
		cont.stg->stepUp();
	else if (scan.direction==1)//-y
		cont.stg->stepDown();
	else if (scan.direction==2)//+x
		cont.stg->stepRight();
	else//-y
		cont.stg->stepLeft();
	cont.stg->wait();
	cont.df.getDefiniteFocus(protocolIndex,true);
	cont.df.getDefiniteFocus(protocolIndex,true);
	cont.daqpci.setVoltage(6,10);
	Timer t(true);
	t.waitTotal(1000*initialDelay);
	cont.df.getDefiniteFocus(protocolIndex,true);
	cont.df.getDefiniteFocus(protocolIndex,true);
	t.waitTotal(1000*totalDelay);
	scan.FOVChannels.at(0).at(0).takePicture("efield"+scan.FOVChannels.at(0).at(0).chan->toString()+"e"+::toString(scan.FOVChannels.at(0).at(0).ap.exp)+"g"+::toString(scan.FOVChannels.at(0).at(0).ap.getGain()));
	cont.daqpci.setVoltage(6,0);
	cont.stg->move(x,y);
						break; }
			case 'K':
				kineticsScan(scan);
				break;
				case 'P':{
				cout<<"Enter exposure time in seconds"<<endl;
				double sec=getDouble();
				AcquisitionChannel ac=AcquisitionChannel(cont.UV,cont.OUT_BLOCKED,cont.MAG_20x);
				if (scan.direction==0 || scan.direction==1)
					cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
				else
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
				int x=cont.stg->getX();
				int y=cont.stg->getY();
				ac.on();
				for(vector<vector<AcquisitionChannel>>::const_iterator i=scan.FOVChannels.begin();i!=scan.FOVChannels.end();i++){
					if (scan.direction==0)//+y
						cont.stg->stepUp();
					else if (scan.direction==1)//-y
						cont.stg->stepDown();
					else if (scan.direction==2)//+x
						cont.stg->stepRight();
					else//-y
						cont.stg->stepLeft();
					cont.stg->wait();
					Timer::wait(sec*1000);
				}
				ac.off();
				cont.stg->move(x,y);
				cont.stg->wait();
				break;}
			case 'B':{
				cout<<"Enter exposure time in seconds"<<endl;
				double sec=getDouble();
				AcquisitionChannel ac=scan.FOVChannels.front().front();
				ac.out=&cont.outPorts[cont.OUT_BLOCKED];
				if (scan.direction==0 || scan.direction==1)
					cont.stg->setYstep(scan.FOVChannels.at(0).at(0).getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
				else
					cont.stg->setXstep(scan.FOVChannels.at(0).at(0).getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
				int x=cont.stg->getX();
				int y=cont.stg->getY();
				ac.on();
				for(vector<vector<AcquisitionChannel>>::const_iterator i=scan.FOVChannels.begin();i!=scan.FOVChannels.end();i++){
					if (scan.direction==0)//+y
						cont.stg->stepUp();
					else if (scan.direction==1)//-y
						cont.stg->stepDown();
					else if (scan.direction==2)//+x
						cont.stg->stepRight();
					else//-y
						cont.stg->stepLeft();
					cont.stg->wait();
					Timer::wait(sec*1000);
				}
				ac.off();
				cont.stg->move(x,y);
				cont.stg->wait();
				break;}
			case 'Z':{
				currentChamber=scan.cham;
				cont.axio.setMaxZ(currentChamber.maxZ);
				cont.focus->move(currentChamber.focusZ);
				break;
					 }
			case 'M':{
				int chan; double fractionLength,fractionWidth;
				cout<<"Please enter desired channel ("<<1<<"-"<<currentChamber.numChannels<<")"<<endl;
				chan=getInt();
				cout<<"Please enter desired fractional position (0 to 1) along the channel length"<<endl;
				fractionLength=getDouble();
				cout<<"Please enter desired fractional position (0 to 1) along the channel width"<<endl;
				fractionWidth=getDouble();
				if (cont.axio.getCurrentObjective()->isOil)
					cont.stg->setSpeed(OILVELOCITY);
					scan.cham.move(chan,fractionWidth,fractionLength);
					cont.stg->setSpeed(MAXVELOCITY);
				}
				break;
		case 'T':{
				string folder;
				int numFOVs=0;
				cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
				std::getline(std::cin,folder);
				if (scan.FOVChannels.size()==1){
				cout<<"enter number of FOVs to scan"<<endl;
					numFOVs=getInt();
				}
				cout<<"enter first channel number to scan"<<endl;
				int first=getInt();
				scan.channelNum=first;
				if (first<1 || first>scan.cham.numChannels){
					logFile.write("invalid first channel",true);
					break;
				}
				cout<<"enter final channel number to scan"<<endl;
				int final=getInt();
				if (final<scan.channelNum || final>scan.cham.numChannels){
					logFile.write("invalid final channel",true);
					break;
				}
				
				cout<<"enter length fraction for start of scan"<<endl;
				double lenFrac=getDouble();
				if (lenFrac<0||lenFrac>1){
					logFile.write("Invalid length fraction",true);
					break;	
				}
				
				cout<<"enter width fraction for start of scan"<<endl;
				double widFrac=getDouble();
				if (widFrac<0||widFrac>1){
					logFile.write("Invalid width fraction",true);
					break;	
				}

				string c;
				cout<<"press (c) to continue or (b) to go back"<<endl;
				cin>>c;
				cin.clear();
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				if (c!="c"){
					cont.setWorkingDir("default");
					break;
				}
				int curr=scan.channelNum;
				
				while(curr<=final){
					cont.setWorkingDir(folder+"\\Channel"+::toString(curr,2));
					scan.cham.move(curr,widFrac,lenFrac);
					cont.stg->wait();
					if (numFOVs==0){//DefiniteFocusScan
						imageMultipleFOVs(scan);
					}else{
						DefiniteFocusScan dfs(scan.FOVChannels.front(),numFOVs,scan.FOVscan,scan.direction);
						dfs.runScan();
					}
					curr++;
				}
				break;
				 }
			case 'e':
				return;
				break;
		}
	}
}


void ProtocolEric::imageMultipleFOVs(customProtocol& scan){
	AcquisitionChannel getIt=cont.currentChannel();
	for(vector<vector<AcquisitionChannel>>::reverse_iterator i=scan.FOVChannels.rbegin();i!=scan.FOVChannels.rend();i++){
		for(vector<AcquisitionChannel>::reverse_iterator j=i->rbegin();j!=i->rend();j++){
			getIt=*j;
		}
	}
	if (scan.FOVChannels.size()==0) return;
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setYstep(getIt.getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
	else
		cont.stg->setXstep(getIt.getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
	int x,y;
	x=cont.stg->getX();
	y=cont.stg->getY();
	//cont.stg->chamberMove(scan.chamber,scan.channel,scan.section);
	cont.stg->wait();
	logFile.write("");
	logFile.write("");
	logFile.write("Image Multiple FOVs Stage start position: ("+toString(cont.stg->getX())+","+toString(cont.stg->getY())+")",true);
	logFile.write("");
	logFile.write("");
	bool b1,b2,b3;
	//b2=cont.df.isInitialized(protocolIndex);
	//b1=cont.df.getDefiniteFocus(protocolIndex,true);
	double start=cont.focus->getZ();
#ifdef OBSERVER
	if (scan.coarse.range!=0 && (scan.initial.range==-2 || scan.initial.range==-1)){
		if (!cont.df.isInitialized(protocolIndex)){
			scan.coarse.getFocus(true,start,true,1,b1);
			if (!b1){
				logFile.write("Error: Image Multiple FOVs could not autofocus to initialize Definite Focus...exiting",true);
				cont.stg->move(x,y);
				return;
			}
			if (!cont.df.initializeDefiniteFocus(protocolIndex)){
				logFile.write("Error: Definite Focus could not be initialized for JIT (just-in-time) focusing...exiting",true);
				cont.stg->move(x,y);
				return;
			}
		}
		if (scan.initial.range==-1){//continuous definite focusing
			if (!cont.df.startDefiniteFocus(protocolIndex)){
				logFile.write("Error: Definite Focus could not be initialized for continuous focusing...exiting",true);
			}
		}
	}else
#endif
		if (scan.coarse.range!=0){

			double start1,start2,start3;
			if (scan.direction==0 || scan.direction==1){
				cont.stg->setXstep(getIt.getFOVWidth()*scan.FOVfocus/cont.stg->getStepSize());
				if (scan.direction==0)//+y
					cont.stg->stepLeft();
				else//-y
					cont.stg->stepRight();
			}else{
				cont.stg->setYstep(getIt.getFOVHeight()*scan.FOVfocus/cont.stg->getStepSize());
				if (scan.direction==2)//+x
					cont.stg->stepUp();
				else//-y
					cont.stg->stepDown();
			}
			
			cont.stg->wait();
			Timer::wait(scan.moveDelay);
			start1=scan.coarse.getFocus(false,start,false,1,b1);
			if (scan.direction==0)//+y
				cont.stg->stepRight();
			else if (scan.direction==1)//-y
				cont.stg->stepLeft();
			else if (scan.direction==2)//+x
				cont.stg->stepDown();
			else//-y
				cont.stg->stepUp();
			cont.stg->wait();
			Timer::wait(3000);
			if (b1) start2=scan.initial.getFocus(false,start1,false,1,b2);
			else start2=scan.coarse.getFocus(false,start,false,1,b2);
			if (scan.direction==0)//+y
				cont.stg->stepRight();
			else if (scan.direction==1)//-y
				cont.stg->stepLeft();
			else if (scan.direction==2)//+x
				cont.stg->stepDown();
			else//-y
				cont.stg->stepUp();
			cont.stg->wait();
			Timer::wait(scan.moveDelay);
			if (b2) start3=scan.initial.getFocus(false,start2,false,1,b3);
			else if (b1) start3=scan.initial.getFocus(false,start1,false,1,b3);
			else start3=scan.coarse.getFocus(false,start,false,1,b3);
			if (scan.direction==0)//+y
				cont.stg->stepLeft();
			else if (scan.direction==1)//-y
				cont.stg->stepRight();
			else if (scan.direction==2)//+x
				cont.stg->stepUp();
			else//-y
				cont.stg->stepDown();
			int num=0;
			start=0;
			if (b1==false && b2==false && b3==false){
				logFile.write("Error: Image Multiple FOVs failed to find coarse focus...exiting",true);
				cont.stg->move(x,y);
				return;
			}
			if (b1){
				start+=start1;
				num++;
			}else{
				logFile.write(string("First Coarse focus failed"),true);
			}
			if (b2){
				start+=start2;
				num++;
			}else{
				logFile.write(string("Second Coarse focus failed"),true);
			}
			if (b3){
				start+=start3;
				num++;
			}else{
				logFile.write(string("Third Coarse focus failed"),true);
			}
			start=start/num;//average focus
		}
		bool isSameFOV=true;
		for(vector<vector<AcquisitionChannel>>::iterator i=scan.FOVChannels.begin()+1;i!=scan.FOVChannels.end();i++){
			if (*i!=scan.FOVChannels.front()){
				isSameFOV=false;
				break;
			}
		}
		if (scan.coarse.range!=0 && scan.initial.range==-2 && isSameFOV){//JIT definite focus
			DefiniteFocusScan dfs(scan.FOVChannels.front(),scan.FOVChannels.size(),scan.FOVscan,scan.direction);
			dfs.runScan();
		}else{
			for(int j=0;j<scan.FOVChannels.size();j++){
				if (scan.direction==0 || scan.direction==1)
					cont.stg->setYstep(getIt.getFOVHeight()*scan.FOVscan/cont.stg->getStepSize());
				else
					cont.stg->setXstep(getIt.getFOVWidth()*scan.FOVscan/cont.stg->getStepSize());
				cont.stg->wait();
				if (scan.direction==0)//+y
					cont.stg->stepUp();
				else if (scan.direction==1)//-y
					cont.stg->stepDown();
				else if (scan.direction==2)//+x
					cont.stg->stepRight();
				else//-y
					cont.stg->stepLeft();
				if (scan.coarse.range!=0){
					//cont.focus->move(start);
					//cont.focus->wait();
				}
				cont.stg->wait();
				//b1=cont.df.getDefiniteFocus(protocolIndex,true);

				imageSingleFOV(j,scan);
			}
		}
		cont.stg->move(x,y);
#ifdef OBSERVER
		if (scan.coarse.range!=0 && scan.initial.range==-1)//continuous DF
			cont.df.stopDefiniteFocus();
		if (scan.coarse.range!=0 && scan.initial.range==-2);//JIT definite focus
			//cont.df.getDefiniteFocus(true);
		else
#endif
			;//cont.focus->move(start);
	/*
	logFile.write("Returning to Start Position",true);
	for(int j=1;j<channels.size();j++){
		cont.stg->stepDown();
		cont.stg->wait();
	}*/
}
/*
void ProtocolEric::takeBleachImages(double sampleSec,double totalSec,double numFOVSpacing,Focus*f1,Focus*f2,std::string fileName,std::vector<vector<AcquisitionChannel>> &channels){
	if (channels.size()==0) return;
	cont.stg->setYstep(channels.at(0).at(0).getFOVHeight()*numFOVSpacing/cont.stg->getStepSize());
	logFile.write("");
	logFile.write("");
	logFile.write("Take Bleach Images Stage start position: ("+toString(cont.stg->getX())+",("+toString(cont.stg->getY())+")",true);
	logFile.write("");
	logFile.write("");
	
	double start=f1->getFocus();
	takeBleachImagesFOV(sampleSec,totalSec,numFOVSpacing,f1,f2,fileName+"-"+toString(0,2),channels.at(0));

	for(int j=1;j<channels.size();j++){
		cont.focus->move(start);
		cont.stg->stepUp();
		cont.stg->wait();
		cont.focus->wait();
		takeBleachImagesFOV(sampleSec,totalSec,numFOVSpacing,f1,f2,fileName+"-"+toString(j,2),channels.at(j));
	}
	cout<<"Returning to Start Position"<<endl;
	for(int j=1;j<channels.size();j++){
		cont.stg->stepDown();
		cont.stg->wait();
	}
}
*/

void ProtocolEric::photocleaveAndImage(double seconds, customProtocol& scan){
	
	double tritonVolume=10;//mL
	
	//Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");

	AcquisitionChannel uvTL(cont.UV,cont.OUT_BLOCKED,cont.MAG_100xOil);
	
	expose(scan.FOVChannels.size(),scan.FOVscan,seconds,&uvTL);

	//cont.pmp.washAbsolute(highSalt,tritonVolume);

	imageMultipleFOVs(scan);
				
}

void ProtocolEric::pumpProtocols(){
	char c;
	while(true){
		cout<<"Please select a protocol to run:"<<endl;
		cout<<"1: Load Channels with corresponding reagents"<<endl; //Definite Focus test"<<endl;
		cout<<"2: Unclog channels"<<endl;
		cout<<"3: Load fraction dUTP-FITC into correct channels"<<endl;
		cout<<"e: Exit"<<endl;
		c=getChar();
		switch(c){
			case '1':{
				vector<FlowChannel*> fs=scan.fs.selectChannels();
				vector<Solution*> rs=scan.fs.selectReagents();
				if (fs.size()!=rs.size())
					break;
				vector<Solution*>::iterator r=rs.begin();
				for(vector<FlowChannel*>::iterator f=fs.begin();f!=fs.end();f++){
					(*r)->load(*f);
					r++;
				}
				break;}
			case '2':{
				cout<<"Put waste into wash and wash into waste (enter to continue)"<<endl;
				getString();
				scan.fs.solutions.front()->valveSelect();
				for(vector<FlowChannel*>::iterator i=scan.fs.channels.begin();i!=scan.fs.channels.end();i++){
					(*i)->s->pull((*i)->s->wastePos,(*i)->s->volume*2/3,((*i)->s->volume*2/3)/10);
					(*i)->push((*i)->s->volume*2/3,((*i)->s->volume*2/3)/30);
				}
				break;}
			case '3':{
				Solution* r65=scan.fs.getReagent("R07_65%");
				Solution* r70=scan.fs.getReagent("R08_70%");
				Solution* r75=scan.fs.getReagent("R09_75%");
				Solution* r80=scan.fs.getReagent("R10_80%");
				Solution* r85=scan.fs.getReagent("R11_85%");
				Solution* r90=scan.fs.getReagent("R12_90%");
				Solution* r100=scan.fs.getReagent("R13_100%");

				FlowChannel* c65=scan.fs.getChannel("Chan1_65%");
				FlowChannel* c70=scan.fs.getChannel("Chan6_70%");
				FlowChannel* c75=scan.fs.getChannel("Chan8_75%");
				FlowChannel* c80=scan.fs.getChannel("Chan2_80%");
				FlowChannel* c85=scan.fs.getChannel("Chan9_85%");
				FlowChannel* c90=scan.fs.getChannel("Chan5_90%");
				//FlowChannel* c100_1=scan.fs.getChannel("Chan4_100%");
				FlowChannel* c100_2=scan.fs.getChannel("Chan7_100%");

				r65->load(c65);
				r70->load(c70);
				r75->load(c75);
				r80->load(c80);
				r85->load(c85);
				r90->load(c90);
				//r100->load(c100_1);
				r100->load(c100_2);
				
				break;}
			case 'e':
				return;
				break;

		}
	}
}

void ProtocolEric::testAirLoad(int start,int end){
	for(int i=start;i<end+1;i++){
		//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(i-1),scan.loadingVolume.at(i),(scan.loadingVolume.at(i-1)-scan.cham.getChannelVolume())/scan.loadTimeSec),cont.pmp.s.deviceNum);
		//cont.pmp.wait();
		Timer::wait(1000);
	}
	//cont.pmp.wasteSyringe();
}

void ProtocolEric::loadChannelsAirGap(int startChan,int endChan){
	
	if (endChan<startChan){
		logFile.write("LoadChannelsAirGap: end Channel cannot be less than start channel",true);
		return;
	}
	if (endChan==startChan)
		cout<<"Press enter to load air gap"<<endl;
	else
		cout<<"Press enter to load ALL air gaps"<<endl;
	getString();
	//load air gaps
	for(int i=startChan;i<endChan+1;i++){	
		airGap(i);
		Timer::wait(1000);
	}
	//cont.pmp.wasteSyringe();
	//bring in reagents
	for(int i=startChan;i<endChan+1;i++){	
		cout<<"press enter to load channel "<<i<<endl;
		getString();
		loadChannel(i);
		Timer::wait(1000);
	}
}

void ProtocolEric::washChannels(int startChan,int endChan){
	if (endChan<startChan){
		logFile.write("washChannels: end Channel cannot be less than start channel",true);
		return;
	}
	//wash channels
	for(int i=startChan;i<endChan+1;i++){	
		washChannel(i);
		Timer::wait(1000);
	}
	//cont.pmp.wasteSyringe();
}


void ProtocolEric::airGap(int chan){
	if (chan<1 || chan>scan.cham.numChannels){
		logFile.write("invalid channel number",true);
		return;
	}
	//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(chan-1),mmChannel2uL(scan.airGapmmChannel),Pump::MINSPEED),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
}

void ProtocolEric::loadChannel(int chan){
	if (chan<1 || chan>scan.cham.numChannels){
		logFile.write("invalid channel number",true);
		return;
	}
	//first pull at loadSpeed
	//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(chan-1),scan.loadingVolume.at(chan-1)-3*scan.cham.getChannelVolume()-mmChannel2uL(scan.airGapmmChannel),(scan.loadingVolume.at(chan-1)-scan.cham.getChannelVolume())/scan.loadTimeSec),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
	Timer::wait(1000);
	//next pull at channel speed volume will be 1.5*channelVolume to get us to the end of the channel plus half the excess which is 0.5*(numvolumes-1)*channelVolume)
	//an equivalent way of say that is take the middle of the reagentvolume and put it in the middle of the channel
	//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(chan-1),(3+0.5*scan.loadNumChannelVolumes)*scan.cham.getChannelVolume()+mmChannel2uL(scan.airGapmmChannel),mmChannel2uL(scan.mmpsChannelSpeed)),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
}

void ProtocolEric::washChannel(int chan){
	if (chan<1 || chan>scan.cham.numChannels){
		logFile.write("invalid channel number",true);
		return;
	}
	//cont.pmp.sendCommand(cont.pmp.s.pull(cont.pmp.reagents.at(chan-1),scan.cham.getChannelVolume()*scan.washNumChannelVolumes+scan.loadingVolume.at(chan-1),mmChannel2uL(scan.washSpeedmmpsChannel)),cont.pmp.s.deviceNum);
	//cont.pmp.wait();
}

double ProtocolEric::mmTubing2uL(double mm){
	return mm*M_PI*square(scan.tubingDiameterInches*25.4)/4;
}

double ProtocolEric::mmChannel2uL(double mm){
	return mm*scan.cham.getChannelHeight()*scan.cham.getChannelWidth();
}

void ProtocolEric::washAndImage(customProtocol& scan){
	double tritonVolume=10;//mL
	double SDSVolume=10;//mL
	
	//Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");
	//Solution lowSalt(1,false,true,false,false,false,2,1,"Low Salt");
	//Solution SDS(2,false,true,false,false,false,1,2,"SDS");

	cont.te.setFixedTemp(50,true);
	//cont.pmp.washAbsolute(lowSalt,1000*tritonVolume);
	cont.te.setFixedTemp(20,true);
	//cont.pmp.washAbsolute(SDS,1000*SDSVolume);
	//cont.pmp.wash(highSalt,1);
	
	imageMultipleFOVs(scan);
}
/*
void ProtocolEric::takeBleachImagesFOV(double sampleSec,double totalSec,double numFOVSpacing,Focus*f1,Focus*f2,std::string fileName,vector<AcquisitionChannel> &channels){
	cont.stg->setXstep(channels.at(0).getFOVWidth()*numFOVSpacing/cont.stg->getStepSize());
	cont.stg->stepLeft();cont.stg->wait();

	f1->adjustIntensity(0.5);
	double foc1=f1->getFocus(true);
	f1->adjustIntensity(0.5);
	
	cont.stg->stepRight();cont.stg->wait();
	cont.stg->stepRight();cont.stg->wait();

	f2->adjustIntensity(0.5);
	double foc2=f2->getFocus(false);
	f2->adjustIntensity(0.5);

	cont.stg->stepLeft();

	cont.focus->move((foc1+foc2)/2.0);
	cont.focus->wait();
	cont.stg->wait();
	::Timer t(true);
	int i=0;
	while(t.getTime()<1000.0*totalSec){
		channels.at(0).on();
		t.waitTotal(i*1000.0*sampleSec);
		AcquisitionChannel::takeMultiplePics(channels,"",fileName+"-"+toString((int)(t.getTime()/1000.0),3));
		i++;
	}

}
	*/

void ProtocolEric::imageSingleFOV(int FOVNum,customProtocol& scan){
	if (scan.FOVChannels.at(FOVNum).size()==0)
		return;
	double foc1=0,foc2=0;
	bool b1,b2;
	double start=cont.focus->getZ();
	if (scan.direction==0 || scan.direction==1)
		cont.stg->setXstep(scan.FOVChannels.at(FOVNum).at(0).getFOVWidth()*scan.FOVfocus/cont.stg->getStepSize());
	else
		cont.stg->setYstep(scan.FOVChannels.at(FOVNum).at(0).getFOVHeight()*scan.FOVfocus/cont.stg->getStepSize());
#ifdef OBSERVER
	if (scan.coarse.range!=0 && scan.initial.range==-2){//JIT definite focus
		b1=true;//cont.df.getDefiniteFocus(protocolIndex,true);
		if (!b1){
			logFile.write("Error Definite Focus: JIT (just-in-time) focusing failed...exiting",true);
			return;
		}
	}
#endif
	if (scan.coarse.range!=0 && scan.initial.range!=0 && scan.initial.range!=-2 && scan.initial.range!=-1){
		if (scan.direction==0)
			cont.stg->stepLeft();
		else if (scan.direction==1)
			cont.stg->stepRight();
		else if (scan.direction==2)
			cont.stg->stepUp();
		else
			cont.stg->stepDown();
		cont.stg->wait();
		Timer::wait(scan.moveDelay);
		if (scan.focusBleachCompensation) scan.initial.adjustIntensity(0.5);
		foc1=scan.initial.getFocus(false,start,false,1,b1);
		if (scan.focusBleachCompensation) scan.initial.adjustIntensity(0.5);
		if (scan.direction==0)
			cont.stg->stepRight();
		else if (scan.direction==1)
			cont.stg->stepLeft();
		else if (scan.direction==2)
			cont.stg->stepDown();
		else
			cont.stg->stepUp();
		cont.stg->wait();
	}
	if (scan.coarse.range!=0 && scan.final.range!=0 && scan.initial.range!=-2 && scan.initial.range!=-1){
		if (scan.direction==0)
			cont.stg->stepRight();
		else if (scan.direction==1)
			cont.stg->stepLeft();
		else if (scan.direction==2)
			cont.stg->stepDown();
		else
			cont.stg->stepUp();
		cont.stg->wait();
		Timer::wait(scan.moveDelay);
		if (scan.focusBleachCompensation) scan.final.adjustIntensity(0.5);
		if (b1) foc2=scan.final.getFocus(false,foc1,false,1,b2);
		else {
			foc2=scan.final.getFocus(false,start,false,1,b2);
		}
		if (scan.focusBleachCompensation) scan.final.adjustIntensity(0.5);
		if (scan.direction==0)
			cont.stg->stepLeft();
		else if (scan.direction==1)
			cont.stg->stepRight();
		else if (scan.direction==2)
			cont.stg->stepUp();
		else
			cont.stg->stepDown();
	}
	int num=0;
	double best=0;
	if (scan.coarse.range!=0 && scan.initial.range!=0 && scan.initial.range!=-2 && scan.initial.range!=-1 && scan.final.range!=-2 && scan.final.range!=-1 ){
		if (b1){
			num++;
			best+=foc1;
		}else{
			logFile.write(string("First focus failed for FOV")+toString(FOVNum),true);
		}
	}
	if (scan.coarse.range!=0 && scan.final.range!=0 && scan.final.range!=-2 && scan.final.range!=-1 && scan.initial.range!=-2 && scan.initial.range!=-1){
		if (b2){
			num++;
			best+=foc2;
		}else{
			logFile.write(string("Second focus failed for FOV")+toString(FOVNum),true);
		}
	}
	if (scan.coarse.range!=0 && scan.initial.range!=-2 && scan.initial.range!=-1 && (scan.initial.range!=0 || scan.final.range!=0)){//we need a focus
		if (num==0){//couldn't find focus, just move to start position, it is the best we can do
			cont.focus->move(start);
			logFile.write(string("Error: Imaging FOV")+toString(FOVNum)+" could not find focus, using starting position", true);
			cont.focus->wait();
		}else{//we did find something
			cont.focus->move(best/num);
			cont.focus->wait();
		}
	}
	cont.stg->wait();
	Timer::wait(scan.moveDelay);
	if (scan.coarse.range!=0 && scan.initial.range==-2){//JIT definite focus
		//b1=cont.df.getDefiniteFocus(protocolIndex,true);
		//if (!b1){
		//	logFile.write("Error Definite Focus: JIT (just-in-time) focusing failed...exiting",true);
		//	return;
		//}
		//focus update should be performed by move2NextSpotGlobal
		//FOV spacing is 0 since the stage movement is controlled by imageMultipleFOVs
		DefiniteFocusScan dfs(scan.FOVChannels.at(FOVNum),1,0,scan.direction,string("FOV")+toString(FOVNum,3));
		dfs.runScan();
	}else
		AcquisitionChannel::takeMultiplePics(scan.FOVChannels.at(FOVNum),string("FOV")+toString(FOVNum,2),"");
}
/*
void ProtocolEric::takeTIRFBleachImages(double sampleSec,double totalSec,double numFOVSpacing,Focus*f1,Focus*f2,std::vector<vector<AcquisitionChannel>> &FOVChannels){
	


	string name;
	cout<<"Enter folder name for images"<<endl;
	cin>>name;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	
	int i=0;
ASKNUMFOV:	cout<<"Enter number of fields to take ("<<FOVChannels.size()<<" max)"<<endl;
	cin>>i;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (i<0 || i>FOVChannels.size()) goto ASKNUMFOV;
	FOVChannels.erase(FOVChannels.begin()+i,FOVChannels.end());
ASK2NDCHAMBER:		cout<<"Image second channel? (y or n)"<<endl;
	char b;
	cin>>b;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (b!='y'&&b!='n') goto ASK2NDCHAMBER;
	cont.setWorkingDir(name);
	
	takeBleachImages(sampleSec,totalSec,numFOVSpacing,f1,f2,"Left",FOVChannels);
	
	if (b=='y'){
		//int chanXdifference=140000;
		int chanXdifference=-684172;//different chambers
		int x=cont.stg->getX();
		double start=cont.focus->getZ();
		cont.focus->move(0);
		cont.focus->wait();
		cont.stg->moveX(cont.stg->getX()+chanXdifference);
		cont.stg->wait();
		cont.focus->move(start);
		cont.focus->wait();
		::Timer::wait(2000);
		cont.stg->incY(-FOVChannels.at(0).at(0).getFOVHeight()*numFOVSpacing/cont.stg->getStepSize());
		cont.stg->wait();
		double rangeTemp=f1->range;
		f1->range=50;
		f1->getFocus();
		f1->range=rangeTemp;
		cont.stg->incY(FOVChannels.at(0).at(0).getFOVHeight()*numFOVSpacing/cont.stg->getStepSize());
		takeBleachImages(sampleSec,totalSec,numFOVSpacing,f1,f2,"Right",FOVChannels);
		
		cout<<"Returning to Chamber 1 start position"<<endl;
		cont.focus->move(0);
		cont.focus->wait();
		cont.stg->moveX(x);
		cont.stg->wait();
		cont.focus->move(start);
		cont.focus->wait();
	}
	cont.setWorkingDir("default");

}
*/
/*
void ProtocolEric::takeTIRFImages(double numFOVSpacing,Focus* f1,Focus*f2,vector<vector<AcquisitionChannel>> &FOVChannels){



	string name;
	cout<<"Enter folder name for images"<<endl;
	cin>>name;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	
	int i=0;
ASKNUMFOV:	cout<<"Enter number of fields to take ("<<FOVChannels.size()<<" max)"<<endl;
	cin>>i;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (i<0 || i>FOVChannels.size()) goto ASKNUMFOV;
	FOVChannels.erase(FOVChannels.begin()+i,FOVChannels.end());
ASK2NDCHAMBER:		cout<<"Image second channel? (y or n)"<<endl;
	char b;
	cin>>b;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (b!='y'&&b!='n') goto ASK2NDCHAMBER;
	cont.setWorkingDir(name);
	
	takeFOVImages(numFOVSpacing,numFOVSpacing,f1,f2,"Left",FOVChannels);
	
	if (b=='y'){
		//int chanXdifference=140000;
		int chanXdifference=-684172;//different chambers
		int x=cont.stg->getX();
		double start=cont.focus->getZ();
		cont.focus->move(0);
		cont.focus->wait();
		cont.stg->moveX(cont.stg->getX()+chanXdifference);
		cont.stg->wait();
		cont.focus->move(start);
		cont.focus->wait();
		::Timer::wait(2000);
		cont.stg->incY(-FOVChannels.at(0).at(0).getFOVHeight()*numFOVSpacing/cont.stg->getStepSize());
		cont.stg->wait();
		double rangeTemp=f1->range;
		f1->range=50;
		f1->getFocus();
		f1->range=rangeTemp;
		cont.stg->incY(FOVChannels.at(0).at(0).getFOVHeight()*numFOVSpacing/cont.stg->getStepSize());
		takeFOVImages(numFOVSpacing,numFOVSpacing,f1,f2,"Right",FOVChannels);
		
		cout<<"Returning to Chamber 1 start position"<<endl;
		cont.focus->move(0);
		cont.focus->wait();
		cont.stg->moveX(x);
		cont.stg->wait();
		cont.focus->move(start);
		cont.focus->wait();
	}
	cont.setWorkingDir("default");

}
*/
void ProtocolEric::ligationKineticsAccum(){
	cont.setWorkingDir("LigationKineticsComplexAccum1.0");

//	Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");
//	Solution lowSalt(1,false,true,false,false,false,2,1,"Low Salt");
//	Solution probe(2,true,false,false,false,false,1,4,"Ligation Probes");

	vector<AcquisitionChannel> channels;

	AcquisitionChannel bf(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.030,3,1),1.2,"V",true);
	bf.on();
	bf.off();

	//40ms gain 5 is sweet spot?
	AcquisitionChannel cy55N1(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//0.5x sweet spot
	AcquisitionChannel cy55N2(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//sweet spot
	AcquisitionChannel cy55N3(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//2x sweet spot
	channels.push_back(cy55N1);channels.push_back(cy55N2);channels.push_back(cy55N3);

	//4ms gain 3 is sweet spot?
	AcquisitionChannel cy35N1(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//0.5x sweet spot
	AcquisitionChannel cy35N2(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//0.5x sweet spot
	AcquisitionChannel cy35N3(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,3,1));//0.5x sweet spot
	channels.push_back(cy35N1);channels.push_back(cy35N2);channels.push_back(cy35N3);

	//100ms gain 7 is sweet spot
	AcquisitionChannel fitcN(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,8,1));
	channels.push_back(fitcN);
	

	AcquisitionChannel foc(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.001,3,1));
	Focus focus(foc,20,.25);
	cont.te.periodicRecording(10,false);
	cont.te.setFixedTemp(26,true);
	//cont.pmp.wash(lowSalt,5);
	double foc1,foc2,foc3;
	foc1=focus.getFocus();
	cont.stg->stepDown();
	focus.adjustIntensity(0.3);
	bf.adjustIntensity(0.6);
	bf.takePicture("BackgroundBF");
	AcquisitionChannel::takeMultipleAccumulations(channels,20,"Background");
TRYAGAIN:	cout<<"Are these images acceptable to you? (y or n)"<<endl;
	char c;
	cin>>c;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (c=='n'){
		cout<<"Aborting Ligation Kinetics Experiment!"<<endl;
		return;
	}else if (c!='y'){
		goto TRYAGAIN;
	}
//	cont.pmp.fill(probe,&lowSalt);
	int totalMinutes=60;
	int sampleMinutes=5;
	Timer myTime;
	
	for(int i=1;i<=totalMinutes/5;i++){
		myTime.startTimer();
		myTime.waitAfterLastStart(1000*60*(sampleMinutes-1));//give te module 1 minute to heat up to 37
		
		myTime.waitAfterLastStart(1000*60*sampleMinutes);
		myTime.stopTimer();
		cont.te.setFixedTemp(37,false);
		cout<<"Imaging Chamber for minute: "<<5*i<<endl;
	//	cont.pmp.wash(lowSalt,5);
		focus.adjustIntensity(0.3);
		focus.getFocus();
		focus.adjustIntensity(0.3);
		bf.takePicture(string("BF")+string("t")+toString(i,3));
		AcquisitionChannel::takeMultipleAccumulations(channels,20,"",string("t")+toString(i,3));
		cont.te.setFixedTemp(26,true);
TRYAGAINREACTION:		cout<<"Do reaction for another 5 minutes?"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (c=='n'){
			cout<<"Aborting Ligation Kinetics experiment at minute:"<<5*i<<endl;
			return;
		}else if (c!='y'){
			goto TRYAGAINREACTION;
		}
		//cont.pmp.fill(probe,&lowSalt);
	}
	cout<<"Ligation reaction complete. Washing with high salt"<<endl;
//	cont.pmp.wash(highSalt,5);
	cout<<"Acquiring final Ligation image"<<endl;
	focus.adjustIntensity(0.5);
	focus.getFocus();
	bf.takePicture("BFLigation");
	AcquisitionChannel::takeMultipleAccumulations(channels,20,"","Ligation");
	AcquisitionChannel::takeMultiplePics(channels,"Ligation");
	cont.te.powerOff();//turn off modules
	cont.te.record=false;//stop recording
	cout<<"Ligation Kinetics with Complex Probes is done!"<<endl;
}

void ProtocolEric::SBSFirstExperiment(){
	//

}

void ProtocolEric::ligationKinetics(){
	cont.setWorkingDir("LigationKineticsComplex4.0");

	int numTimePoints=6;
	int numFOVs=5;
	vector<double> sampleMinutes;
	sampleMinutes.push_back(2.5);
	sampleMinutes.push_back(5);
	sampleMinutes.push_back(10);
	sampleMinutes.push_back(20);
	sampleMinutes.push_back(40);
	sampleMinutes.push_back(60);
	vector<Solution*> probes;
//	probes.push_back(Solution(0,true,1,2,"Labels1",-1,-1,true));
//	probes.push_back(Solution(1,true,1,3,"Labels2",-1,-1,true));
//	probes.push_back(Solution(2,true,1,4,"Labels3",-1,-1,true));
//	probes.push_back(Solution(3,true,1,5,"Labels4",-1,-1,true));
//	probes.push_back(Solution(4,true,1,6,"Labels5",-1,-1,true));
//	probes.push_back(Solution(5,true,2,9,"Labels6",-1,-1,true));
	
	assert(sampleMinutes.size()==numTimePoints && probes.size()==numTimePoints);

//	Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");
//	Solution lowSalt(1,false,true,false,false,false,2,1,"Low Salt");
	
	vector<AcquisitionChannel> channels;

	AcquisitionChannel bf(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,4,1),1.2,"V",true);
	bf.on();
	bf.off();

	//80ms gain 20 is sweet spot?
	AcquisitionChannel cy55N1(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,5,1));
	AcquisitionChannel cy55N2(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,10,1),1,"",true);
	AcquisitionChannel cy55N3(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,20,1));
	channels.push_back(cy55N1);channels.push_back(cy55N2);channels.push_back(cy55N3);

	//32ms gain 3 is sweet spot?
	AcquisitionChannel cy35N1(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.016,3,1));
	AcquisitionChannel cy35N2(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.032,3,1),1,"",true);
	AcquisitionChannel cy35N3(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.064,3,1));
	channels.push_back(cy35N1);channels.push_back(cy35N2);channels.push_back(cy35N3);

	//50ms gain 14 is sweet spot
	AcquisitionChannel fitcN1(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,4,1));
	AcquisitionChannel fitcN2(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,8,1),1,"",true);
	AcquisitionChannel fitcN3(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.200,8,1));
	channels.push_back(fitcN1);channels.push_back(fitcN2);channels.push_back(fitcN3);
	
	//vector<vector<AcquisitionChannel>> FOVChannels;
	//FOVChannels.push_back(channels);
	//no dapi because it may photocleave probes

	AcquisitionChannel focus(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.140,16,1));
	Focus f1(focus,20,.05);
	Focus f2(focus,5,.05);
	cont.te.periodicRecording(10,false);
	cont.te.setFixedTemp(35,true);
//	cont.pmp.wash(lowSalt,1);
	Timer::wait(1000*60*2);
//	cont.pmp.wash(lowSalt,2);
	char c;
	
	Timer myTime;
	
	for(int i=0;i<numTimePoints;i++){

		//imageSingleFOV(i,scanstring("t")+toString(i,3),0,2.0,f1,f2,channels,true);

TRYAGAINREACTION:		cout<<"Continue reaction to "<<sampleMinutes[i]<<" minutes?"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (c=='n'){
			cout<<"Aborting Ligation Kinetics experiment at minute:"<<sampleMinutes[i]<<endl;
			return;
		}else if (c!='y'){
			goto TRYAGAINREACTION;
		}
//		cont.pmp.fill(probes[i],&lowSalt);
		myTime.startTimer();
//		cont.pmp.startOscillation(60,2000,120);
		myTime.waitTotal(1000*60*sampleMinutes[i]-1000*30);//give 30 seconds to stop oscillation
//		cont.pmp.stopOscillation();
		myTime.waitTotal(1000*60*sampleMinutes[i]);
		myTime.stopTimer();
		cont.te.setFixedTemp(45,false);//melt nonspecific probes OFF!
		logFile.write("Imaging Chamber for minute: "+toString(sampleMinutes[i]),true);
//		cont.pmp.wash(lowSalt,2);
		cont.te.wait();
//		cont.pmp.wash(lowSalt,2);
		cont.te.setFixedTemp(26,true);
//		cont.pmp.wash(lowSalt,1);
		Timer::wait(1000*60*2);
//		cont.pmp.wash(lowSalt,2);
	}

	

	cout<<"Ligation reaction complete. Washing with high salt"<<endl;
//	cont.pmp.wash(highSalt,5);
	
	cont.te.powerOff();//turn off modules
	cont.te.record=false;//stop recording
	cout<<"Ligation Kinetics with Complex Probes is done!"<<endl;
}

void ProtocolEric::photocleavageKinetics(){
	cont.setWorkingDir("PhotocleavageKinetics3.1");
//	Solution highSalt(0,false,true,false,false,false,1,1,"High Salt",-1,120);//half default speed
	vector<AcquisitionChannel> channels;
	double foc1,foc2;
	AcquisitionChannel uv(&cont.channels[cont.UV],&cont.outPorts[cont.OUT_BLOCKED],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1),1);
	
	AcquisitionChannel bf(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,4,1),1.2,"V",true);
	bf.on();
	bf.off();

	//80ms gain 20 is sweet spot?
	AcquisitionChannel cy55N1(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,5,1));
	AcquisitionChannel cy55N2(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,10,1),1);
	AcquisitionChannel cy55N3(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,20,1));
	channels.push_back(cy55N1);channels.push_back(cy55N2);channels.push_back(cy55N3);

	//32ms gain 3 is sweet spot?
	AcquisitionChannel cy35N1(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.016,3,1));
	AcquisitionChannel cy35N2(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.032,3,1),1);
	AcquisitionChannel cy35N3(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.064,3,1));
	channels.push_back(cy35N1);channels.push_back(cy35N2);channels.push_back(cy35N3);

	//50ms gain 14 is sweet spot
	AcquisitionChannel fitcN1(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,4,1));
	AcquisitionChannel fitcN2(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,8,1),1);
	AcquisitionChannel fitcN3(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.200,8,1));
	channels.push_back(fitcN1);channels.push_back(fitcN2);channels.push_back(fitcN3);
cont.stg->setYstep(1.5*bf.getFOVHeight()/cont.stg->getStepSize());
	Focus focus(fitcN2,20,.1);
//	cont.pmp.wash(highSalt,2);
	focus.adjustIntensity(0.5);
	focus.getFocus();
	focus.adjustIntensity(0.5);
	bf.adjustIntensity(0.7);
	channels.push_back(bf);
	focus.adjustIntensity(0.5);
		foc1=focus.getFocus();
		focus.adjustIntensity(0.5);
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();
		foc2=focus.getFocus();
		//go to bottom FOV
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();
		logFile.write("Field 1 Z is:"+toString(foc2+0.5*(foc2-foc1)),true);
		cont.focus->move(foc2+0.5*(foc2-foc1));
		cont.focus->wait();
		AcquisitionChannel::takeMultiplePics(channels,"FOV1",string("t")+toString(0,3));
		cont.stg->stepUp();cont.stg->wait();cont.stg->stepUp();cont.stg->wait();
		logFile.write("Field 2 Z is:"+toString(foc2-0.5*(foc2-foc1)),true);
		cont.focus->move(foc2-0.5*(foc2-foc1));
		cont.focus->wait();
		AcquisitionChannel::takeMultiplePics(channels,"FOV2",string("t")+toString(0,3));
		cont.stg->stepUp();cont.stg->wait();cont.stg->stepUp();cont.stg->wait();
		logFile.write("Field 3 Z is:"+toString(foc1-0.5*(foc2-foc1)),true);
		cont.focus->move(foc1-0.5*(foc2-foc1));
		cont.focus->wait();
		AcquisitionChannel::takeMultiplePics(channels,"FOV3",string("t")+toString(0,3));
		cont.stg->stepDown();cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();
ASKAGAIN:	cout<<"Are Images Acceptable? (y or n)"<<endl;
	char c;
	cin>>c;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (c=='n'){
		cout<<"Aborting Photocleavage experiment! Goodbye."<<endl;
		return;
	}
	else if (c!='y') goto ASKAGAIN;
	double totalMin=.5;//total uv cleavage of 2 minutes
	int sampleSec=1;//sample every 5 seconds
	for(int i=1;i<=60.0*totalMin/sampleSec;i++){
		
//		cont.pmp.washAbsolute(highSalt,5*cont.pmp.getChamberVolume());
		focus.adjustIntensity(0.5);
		foc1=focus.getFocus();
		focus.adjustIntensity(0.5);
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();
		foc2=focus.getFocus();
		//go to bottom FOV
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();
		logFile.write("Field 1 Z is:"+toString(foc2+0.5*(foc2-foc1)),true);
		cont.focus->move(foc2+0.5*(foc2-foc1));
		cont.focus->wait();
		uv.on();
		Timer::wait(1000*sampleSec);
		uv.off();
		AcquisitionChannel::takeMultiplePics(channels,"FOV1",string("t")+toString(i,3));
		cont.stg->stepUp();cont.stg->wait();cont.stg->stepUp();cont.stg->wait();
		logFile.write("Field 2 Z is:"+toString(foc2-0.5*(foc2-foc1)),true);
		cont.focus->move(foc2-0.5*(foc2-foc1));
		cont.focus->wait();
		uv.on();
		Timer::wait(1000*sampleSec);
		uv.off();
		AcquisitionChannel::takeMultiplePics(channels,"FOV2",string("t")+toString(i,3));
		cont.stg->stepUp();cont.stg->wait();cont.stg->stepUp();cont.stg->wait();
		logFile.write("Field 3 Z is:"+toString(foc1-0.5*(foc2-foc1)),true);
		cont.focus->move(foc1-0.5*(foc2-foc1));
		cont.focus->wait();
		uv.on();
		Timer::wait(1000*sampleSec);
		uv.off();
		AcquisitionChannel::takeMultiplePics(channels,"FOV3",string("t")+toString(i,3));
		cont.stg->stepDown();cont.stg->wait();
		cont.stg->stepDown();
		cont.stg->wait();
		cont.stg->stepUp();
		cont.stg->wait();			
	}

	cout<<"Photocleavage experiment done!"<<endl;
}

void ProtocolEric::channelPics(){

	
//	Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");
	vector<AcquisitionChannel> channels;
		
	//AcquisitionChannel bf(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1),1.7);
	//channels.push_back(bf);

	AcquisitionChannel cy550(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,4,1));
	AcquisitionChannel cy55(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1));
	AcquisitionChannel cy552(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,8,1));
	AcquisitionChannel cy553(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,16,1));
	AcquisitionChannel cy554(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	AcquisitionChannel cy555(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,8,1));
	AcquisitionChannel cy556(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,16,1));
	channels.push_back(cy550);channels.push_back(cy55);channels.push_back(cy552);channels.push_back(cy553);channels.push_back(cy554);channels.push_back(cy555);channels.push_back(cy556);

	AcquisitionChannel cy350(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,4,1));
	AcquisitionChannel cy35(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1));
	AcquisitionChannel cy352(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,8,1));
	AcquisitionChannel cy353(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,16,1));
	AcquisitionChannel cy354(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	AcquisitionChannel cy355(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,8,1));
	AcquisitionChannel cy356(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,16,1));
	channels.push_back(cy350);channels.push_back(cy35);channels.push_back(cy352);channels.push_back(cy353);channels.push_back(cy354);channels.push_back(cy355);channels.push_back(cy356);

	AcquisitionChannel fitc0(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,4,1));
	AcquisitionChannel fitc(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1));
	AcquisitionChannel fitc2(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,8,1));
	AcquisitionChannel fitc3(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,16,1));
	AcquisitionChannel fitc4(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	AcquisitionChannel fitc5(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	AcquisitionChannel fitc6(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	channels.push_back(fitc0);channels.push_back(fitc);channels.push_back(fitc2);channels.push_back(fitc3);channels.push_back(fitc4);channels.push_back(fitc5);channels.push_back(fitc6);

	AcquisitionChannel dapi0(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,4,1));
	AcquisitionChannel dapi(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1));
	AcquisitionChannel dapi2(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,8,1));
	AcquisitionChannel dapi3(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,16,1));
	AcquisitionChannel dapi4(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1));
	AcquisitionChannel dapi5(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,8,1));
	AcquisitionChannel dapi6(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,16,1));
	channels.push_back(dapi0);channels.push_back(dapi);channels.push_back(dapi2);channels.push_back(dapi3);channels.push_back(dapi4);channels.push_back(dapi5);channels.push_back(dapi6);

	Focus focus(cy35,20,.25);//AcquisitionChannel(&cont.channels[cont.CY5],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,10,1,ImageRegion(ImageRegion::CENTER))));


	//begin protocol
	string name;
	cout<<"Please enter Folder name"<<endl;
	cin>>name;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cont.setWorkingDir(name);
	focus.getFocus();
	AcquisitionChannel::takeMultiplePics(channels);
	cout<<"Done!"<<endl;
}

void ProtocolEric::testProtocol(){
	AcquisitionChannel bf1(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1),.6,"V",true);
	AcquisitionChannel bf2(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1),1.2,"V",true);
	bf1.takePicture("");
	bf2.takePicture("");
	Timer::wait(5000);

	cout<<"Light done"<<endl;

	
}
/*
void ProtocolEric::pumpTest(){
	cont.pmp.current=cont.pmp.reagents[0];
	cont.pmp.current.isRecyclable=true;
	cont.pmp.primeReagent();
}
*/
void ProtocolEric::cyclicAmplified(){
	//setup
	double FOVspacing=2.0;
	double timeCleave=30;//seconds
	int minStabilize=5;
	int numCycles=8;
	int numFOVs=5;
	vector<Solution*> probes;
//	probes.push_back(Solution(0,true,1,2,"Labels1",-1,-1,true));
//	probes.push_back(Solution(1,true,1,3,"Labels2",-1,-1,true));
//	probes.push_back(Solution(2,true,1,4,"Labels3",-1,-1,true));
//	probes.push_back(Solution(3,true,1,5,"Labels4",-1,-1,true));
//	probes.push_back(Solution(4,true,1,6,"Labels5",-1,-1,true));
//	probes.push_back(Solution(5,true,1,9,"Labels6",-1,-1,true));
//	probes.push_back(Solution(6,true,1,8,"Labels7",-1,-1,true));
//	probes.push_back(Solution(7,true,1,7,"Labels8",-1,-1,true));

//	Solution lowSalt(1,false,true,false,false,false,2,1,"Low Salt");
//	Solution highSalt(0,false,true,false,false,false,1,1,"High Salt");

	vector<AcquisitionChannel> channels;

	AcquisitionChannel uv(&cont.channels[cont.UV],&cont.outPorts[cont.OUT_BLOCKED],&cont.mags[cont.MAG_20x],AcquisitionParameters(.3,4,1),1);
	
	//80ms gain 20 is sweet spot?
	AcquisitionChannel cy55N1(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,5,1));
	AcquisitionChannel cy55N2(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,10,1),1);
	AcquisitionChannel cy55N3(&cont.channels[cont.CY55],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.160,20,1));
	channels.push_back(cy55N1);channels.push_back(cy55N2);channels.push_back(cy55N3);

	//32ms gain 3 is sweet spot?
	AcquisitionChannel cy35N1(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.016,3,1));
	AcquisitionChannel cy35N2(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.032,3,1),1);
	AcquisitionChannel cy35N3(&cont.channels[cont.CY35],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.064,3,1));
	channels.push_back(cy35N1);channels.push_back(cy35N2);channels.push_back(cy35N3);

	//50ms gain 14 is sweet spot
	AcquisitionChannel fitcN1(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,4,1));
	AcquisitionChannel fitcN2(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.100,8,1),1);
	AcquisitionChannel fitcN3(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.200,8,1));
	channels.push_back(fitcN1);channels.push_back(fitcN2);channels.push_back(fitcN3);

	AcquisitionChannel dapi0(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,4,1));
	AcquisitionChannel dapi(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,4,1));
	AcquisitionChannel dapi2(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,8,1));
	AcquisitionChannel dapi3(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,16,1));
	AcquisitionChannel dapi4(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.1,32,1));
	AcquisitionChannel dapi5(&cont.channels[cont.DAPI],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.2,32,1));
	channels.push_back(dapi0);channels.push_back(dapi);channels.push_back(dapi2);channels.push_back(dapi3);channels.push_back(dapi4);channels.push_back(dapi5);
	

	AcquisitionChannel bf(&cont.channels[cont.BF],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.035,4,1),1.2);
	bf.on();
	bf.off();
	channels.push_back(bf);

	Focus f1(fitcN2,20,.05);//AcquisitionChannel(&cont.channels[cont.CY5],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_20x],AcquisitionParameters(.03,10,1,ImageRegion(ImageRegion::CENTER))));
	Focus f2(fitcN2,5,.05);
	char c;
	int t=0;
	//begin protocol
	cont.setWorkingDir("CyclicAmplifiedSBDT4.0");

	
	//prepare chamber
	cout<<"Preparing chamber"<<endl;
	cont.te.periodicRecording(10,false);
	cont.te.setFixedTemp(55,true);
//	cont.pmp.wash(highSalt,2);
	int i=0;
	for(;i<numCycles;i++){
		//acquire background images
		cout<<"Acquiring background images"<<endl;
		
		Timer::wait(1000*60*minStabilize);//wait for temperature equilibration

//		imageSingleFOV(string("t")+toString(i*2,3),0,2.0,f1,f2,channels,true);

ASK:	cout<<"Begin Ligation Cycle "<<((int)i/2+1)<<" (y or n)?";
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (c=='n')
			return;
		if (c!='y') goto ASK;
		
		//low salt wash before ligation
	//	cont.pmp.wash(lowSalt,1);
		//bring in probe
		cout<<"Bringing in ligation mixture"<<endl;
	//	cont.pmp.fill(probes[i],&lowSalt);
	//	cont.pmp.startOscillation(60,2000,120);
		cout<<"Waiting 60 minutes for ligation reaction"<<endl;
		Timer::wait(1000*60*50);
		cout<<"10 minutes remaining for ligation"<<endl;
		Timer::wait(1000*60*10);
	//	cont.pmp.stopOscillation();
		cout<<"Washing After Ligation"<<endl;
	//	cont.pmp.wash(lowSalt,20);
	//	cont.pmp.wash(highSalt,1);

		cout<<"Acquiring Ligation images"<<endl;

		Timer::wait(1000*60*minStabilize);

//		imageSingleFOV(string("t")+toString(i*2,3),0,2.0,f1,f2,channels,true);

		cout<<"Photocleaving with UV light for "<<timeCleave<<" seconds"<<endl;
		
		expose(numFOVs,2.0,30,&uv);

		cout<<"Washing After Photocleavage"<<endl;
	//	cont.pmp.wash(highSalt,20);
	}

	cout<<"Acquiring final background image"<<endl;
	Timer::wait(1000*60*minStabilize);//wait for temperature equilibration
//	imageSingleFOV(string("t")+toString(i*2,3),0,2.0,f1,f2,channels,true);

	cont.te.powerOff();
	cont.te.record=false;
	cout<<"Amplfiied Cyclic Sequencing Protocol Done!!"<<endl;
}

void getPositionChangedTime(double startIN, double& endOUT){
	Timer t(true);
	endOUT=startIN;
	while(endOUT==startIN && t.getTime()<500){
		endOUT=cont.focus->getZ();
	}
}
void ProtocolEric::getFocus(double range, double step){
	cont.displays.closeAll();
	CImg<unsigned short>* temp=0,*best=0;
	LONGLONG score=0;
	LONGLONG bestScore=0;
	double bestZ;
	bestScore=0;
	AcquisitionChannel acq(&cont.channels[cont.FITC],&cont.outPorts[cont.OUT_CAM],&cont.mags[cont.MAG_100xOil],AcquisitionParameters(.04,4,1,ImageRegion(ImageRegion::FULL)),1);
	AndorCam* andor=(AndorCam*) acq.out->cam;
	acq.modify();
	Focus f=Focus(acq,range,step);	
	double start=cont.focus->getZ()-range/2;
	double expFocus=f.getFocus();
	cont.focus->wait();
	for(double position=start;position<start+range-step;position=position+step){
		cont.focus->move(position);
		cont.focus->wait();
		andor->takePicture(acq.ap.exp,acq.ap.getGain(),acq.ap.bin);
		temp=andor->getNewPic();
		andor->getScore(temp,&score);
		if (score>bestScore){
			bestScore=score;
			bestZ=position;
			delete best;
			best=temp;
		}else{
			delete temp;
		}
	}
	andor->showImage(best,"actual focused image");
	cout<<"Exp Focus was   "<< expFocus<<endl;
	cout<<"Actual Focus was"<< bestZ<<endl;
	cout<<"Difference of "<<expFocus-bestZ<<endl;
	cont.focus->move(start);
	cont.focus->wait();
	cont.focus->move(bestZ);
	cont.focus->wait();
	cont.liveView();
}
void ProtocolEric::testPosition(){

	Record saveFile(cont.workingDir+"testPosition.txt");
	double start,end;
	int num1=100;
	int num2=10;
	double* temp=new double[num1+num2];//0,temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8,temp9,temp10,temp11,temp12,temp13;
	double* time=new double[num1+num2];//1,time2,time3,time4,time5,time6,time7,time8,time9,time10,time11,time12,time13,time14,time15;
	double tfinal;
	double speed=.35;
	cont.focus->move(1000);
	cont.focus->wait();
	
	start=cont.focus->getZ();
	Timer t(true);
	cont.focus->velocityMove(speed);
	temp[0]=cont.focus->getZ();
	time[0]=t.getTime();
	for(int i=1;i<num1-1;i++){
	getPositionChangedTime(temp[i-1],temp[i]);
	time[i]=t.getTime();
	}
	cont.focus->stop();
	temp[num1-1]=cont.focus->getZ();
	time[num1-1]=t.getTime();
	for(int i=num1;i<num1+num2;i++){
	getPositionChangedTime(temp[i-1],temp[i]);
	time[i]=t.getTime();
	}
	cont.focus->wait();
	end=cont.focus->getZ();
	tfinal=t.getTime();


	saveFile.write(toString(start)+","+toString(0)+",",false);
	for(int i=0;i<num1+num2;i++){
		saveFile.write(toString(temp[i])+","+toString(time[i])+",",false);
	}
	saveFile.write(toString(end)+","+toString(tfinal)+",",false);
	delete temp;
	delete time;
}

void ProtocolEric::testFocus(){
	Timer t;
	Timer t2;
	Timer t3;
	double speed =2.4;//um per second
	double start=0;
	double end=0;
	double temp=0;
	double num=20;
	double pause=110;
	ObserverZ* focus=(ObserverZ*)cont.focus;
	focus->move(3000);
	focus->wait();
	focus->velocityMove(speed);
	while(start>=focus->getZ()){}
	::Timer::wait(speed/10.0);//1000.0*zspeed/10000.0    acceleration=10mm/sec*sec
	temp=focus->getZ();
	start=temp;
	while(temp>=start){
		start=focus->getZ();
	}
	t.startTimer();
	::Timer::wait(800);
	temp=focus->getZ();
	end=temp;
	while(temp>=end){
		end=focus->getZ();
	}
	t.stopTimer();
	focus->stop();
	cout<<"Actual Distance traveled: "<<end-start<<" Should be: "<<t.getTime()*speed/1000.0<<" Time delay: "<<(end-start-t.getTime()*speed/1000.0)*1000.0/speed<<"ms"<<endl;


	/*


	::Timer::wait(500);
	double p1=cont.focus->getZ();
	::Timer::wait(pause);
	double p2=cont.focus->getZ();
	::Timer::wait(pause);
	double p3=cont.focus->getZ();
	cout<<"position 1 is "<<p1<<endl;
	cout<<"position 2 is "<<p2<<endl;
	cout<<"position 3 is "<<p3<<endl;
	for(int i=0;i<num;i++){
		cont.focus->move(3000);
		cont.focus->wait();
		t2.startTimer();
		//cont.focus->move(3000.25);
		//cont.focus->wait();
		start+=cont.focus->getZ();
		t2.stopTimer();
		cont.focus->velocityMove(speed);
		t.startTimer();
		::Timer::wait(1000-speed/10.0);
		t3.startTimer();
		cont.focus->stop();
		cont.focus->wait();
		t.stopTimer();
		t3.stopTimer();
		end+=cont.focus->getZ();
	}
	cout<<"Initial position:"<<start/num<<" End position:"<<end/num<<endl;
	cout<<"GetZ time was "<<t2.getTime()/num<<endl;
	//cout<<"Average GetZ time was "<<total/num<<" num was " <<num<<endl;
	cout<<"Total time was "<<t.getTime()/num<<endl;
	//cout<<"Extra time was "<<extra.getTime()<<endl;
	cout<<"Stop time was "<<t3.getTime()/num<<endl;
	*/
}

void ProtocolEric::TIRFadjust(){
BEGIN:
	cout<<"Please select TIRF channel for angle optimization"<<endl;
	int ind=0;
	for(vector<Channel>::const_iterator i=cont.channels.begin();i!=cont.channels.end();i++){
		if (i->tirf) cout<<::toString(ind,2)<<": "<<i->toString()<<endl;
		ind++;
	}
	cin>>ind;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (ind>cont.channels.size()-1 || ind<0){
		logFile.write("invalid channel selected. try again",true);
		goto BEGIN;
	}
	Channel* chan=&(cont.channels[ind]);
	//cout<<"Suggested TIRF voltage is "<<chan-tirf.position<<" Volts."<<endl<<"Select start voltage"<<endl;
	double start,end, step;
	cin>>start;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cout<<"Select end voltage"<<endl;
	cin>>end;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cout<<"Select voltage step size"<<endl;
	cin>>step;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	AcquisitionChannel acq(chan,&(cont.outPorts[cont.OUT_CAM]),&(cont.mags[cont.MAG_100xOil]),AcquisitionParameters(),chan->lite().ls->defaultIntensity(&(cont.mags[cont.MAG_100xOil].obj)));
	acq.modify();
	LONGLONG score=0;
	LONGLONG best=0;
	double bestVoltage=0;
	AndorCam* andor=(AndorCam*) acq.out->cam;
	for(;start<=end;start=start+step){
		acq.TIRFangle=start;
		//andor->setImageRegion(0);
		andor->takePicture(acq.ap.exp,acq.ap.getGain(),acq.ap.bin);
		acq.on(true);
		andor->trigger();
		Timer::wait(acq.ap.exp*1000);
		acq.off(true);
		andor->waitIdle();
		CImg<unsigned short>* image=andor->getNewPic();
		andor->getScore(image,&score);
		if (score>best) {
			bestVoltage=acq.TIRFangle;
			best=score;
			andor->showImage(image,string("V=")+toString(acq.TIRFangle,3)+" Score="+toString(score));
		}
		delete image;
	}
	cout<<"Best voltage for "<<chan->toString()<<" is "<<bestVoltage<<" Volts"<<endl;
}

void ProtocolEric::intensityAdjust(){
BEGIN:
	cout<<"Please select TIRF channel for intensity optimization"<<endl;
	int ind=0;
	for(vector<Channel>::const_iterator i=cont.channels.begin();i!=cont.channels.end();i++){
		if (i->tirf) cout<<::toString(ind,2)<<": "<<i->toString()<<endl;
		ind++;
	}
	cin>>ind;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (ind>cont.channels.size()-1 || ind<0){
		logFile.write("invalid channel selected. try again",true);
		goto BEGIN;
	}
	Channel* chan=&(cont.channels[ind]);
	cout<<"Suggested TIRF intensity is "<<chan->lite().ls->defaultIntensity(NULL)<<endl<<"Select start intensity"<<endl;
	double start,end, step;
	cin>>start;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cout<<"Select end intensity"<<endl;
	cin>>end;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	cout<<"Select intensity step size"<<endl;
	cin>>step;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	AcquisitionChannel acq(chan,&(cont.outPorts[cont.OUT_CAM]),&(cont.mags[cont.MAG_100xOil]),AcquisitionParameters(),start);
	acq.ap.modify();
	LONGLONG score;
	LONGLONG best=0;
	double bestIntensity=0;
	AndorCam* andor=(AndorCam*) acq.out->cam;
	for(;start<=end;start=start+step){
		acq.intensity=start;
		andor->getImageRegion(AcquisitionParameters(),andor->minXPixel,andor->minYPixel,andor->maxXPixel,andor->maxYPixel);
		acq.on();
		acq.wait();
		andor->takePicture(acq.ap.exp,acq.ap.getGain(),acq.ap.bin);
		acq.off();
		CImg<unsigned short>* image=andor->getNewPic();
		andor->showImage(image,string("Intensity=")+toString(acq.intensity,3));
		andor->getScore(image,&score);
		if (score>best) bestIntensity=acq.intensity;
	}
	cout<<"Best intensity for "<<chan->toString()<<" is "<<bestIntensity<<" Volts"<<endl;
}



#endif
/*

void Controller::focusStability(){
	float exp=.1;
	getScanRegion();
	int a1,a2,a3,b1,b2,b3,c1,c2,c3,d1,d2,d3,e1,e2,e3,f1,f2,f3,g1,g2,g3,h1,h2,h3,i1,i2,i3;
	double fineRange=20;
	double resolution=focus->getDOF()/4;
	double coarseRange=500;
	bool deb=false;
	//pass 1
	stg->move(scanMinX,scanMinY);
	a1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,autoFocus( ceil(coarseRange/(fineRange/2)), coarseRange, exp,false, focus->getMaxZ()-(coarseRange/2)));
	stg->moveX(scanMinX+round(diffX/2));
	b1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,a1);
	stg->moveX(scanMinX+diffX);
	c1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,b1);
	stg->moveY(scanMinY+round(diffX/2));
	d1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,c1);
	stg->moveX(scanMinX+round(diffX/2));
	e1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,d1);
	stg->moveX(scanMinX);
	f1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,e1);
	stg->moveY(scanMinY+diffY);
	g1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,f1);
	stg->moveX(scanMinX+round(diffX/2));
	h1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,g1);
	stg->moveX(scanMinX+diffX);
	i1=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,h1);

	//pass 2
	stg->move(scanMinX,scanMinY);
	a2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,a1);
	stg->moveX(scanMinX+round(diffX/2));
	b2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,b1);
	stg->moveX(scanMinX+diffX);
	c2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,c1);
	stg->moveY(scanMinY+round(diffX/2));
	d2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,d1);
	stg->moveX(scanMinX+round(diffX/2));
	e2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,e1);
	stg->moveX(scanMinX);
	f2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,f1);
	stg->moveY(scanMinY+diffY);
	g2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,g1);
	stg->moveX(scanMinX+round(diffX/2));
	h2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,h1);
	stg->moveX(scanMinX+diffX);
	i2=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,i1);

	//pass 3
	stg->move(scanMinX,scanMinY);
	a3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,a2);
	stg->moveX(scanMinX+round(diffX/2));
	b3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,b2);
	stg->moveX(scanMinX+diffX);
	c3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,c2);
	stg->moveY(scanMinY+round(diffX/2));
	d3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,d2);
	stg->moveX(scanMinX+round(diffX/2));
	e3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,e2);
	stg->moveX(scanMinX);
	f3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,f2);
	stg->moveY(scanMinY+diffY);
	g3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,g2);
	stg->moveX(scanMinX+round(diffX/2));
	h3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,h2);
	stg->moveX(scanMinX+diffX);
	i3=autoFocus(ceil(fineRange/resolution),fineRange,exp,deb,i2);

	//calculate std deviation
	double stdDev=0;
	double amean=(a1+a2+a3)/3;
	double bmean=(b1+b2+b3)/3;
	double cmean=(c1+c2+c3)/3;
	double dmean=(d1+d2+d3)/3;
	double emean=(e1+e2+e3)/3;
	double fmean=(f1+f2+f3)/3;
	double gmean=(g1+g2+g3)/3;
	double hmean=(h1+h2+h3)/3;
	double imean=(i1+i2+i3)/3;
	stdDev+=SQR(a1-amean)+SQR(a2-amean)+SQR(a3-amean);
	stdDev+=SQR(b1-bmean)+SQR(b2-bmean)+SQR(b3-bmean);
	stdDev+=SQR(c1-cmean)+SQR(c2-cmean)+SQR(c3-cmean);
	stdDev+=SQR(d1-dmean)+SQR(d2-dmean)+SQR(d3-dmean);
	stdDev+=SQR(e1-emean)+SQR(e2-emean)+SQR(e3-emean);
	stdDev+=SQR(f1-fmean)+SQR(f2-fmean)+SQR(f3-fmean);
	stdDev+=SQR(g1-gmean)+SQR(g2-gmean)+SQR(g3-gmean);
	stdDev+=SQR(h1-hmean)+SQR(h2-hmean)+SQR(h3-hmean);
	stdDev+=SQR(i1-imean)+SQR(i2-imean)+SQR(i3-imean);
	stdDev=sqrt(stdDev/(3*9));
	cout<<"stdDev of focus of 9 points measured 3 times each was"<<dec<<stdDev<<endl;
	cout<<"separation in X: "<<diffX*stg->getStepSize()<<" microns"<<endl;
	cout<<"separation in Y: "<<diffY*stg->getStepSize()<<" microns"<<endl;
	filt->closeShutter();
}

void Controller::Ligation100nMAlexaFITCfocus(){
	//R1=Anchor R2=Ligase+bio9mer R3=AlexaLabel W0=low salt W1=high salt	
	focusChannel=FITC;
	cam->setExposure(.1);
	focusExp=.04;
	float Cy5exp=.1;
	float BFexp=.1;
	cam->setGain(14);
	focusGain=4;
	focus->SetHalogenVoltage(1.8);
	focus->SetObj(Scope::OBJ_20);
	setWorkingDir("D:\\Eric\\Alexa100nMSingleStepLigationFITCfocus\\");
	te.setFixedTemp(26);
	te.powerOn();
	pmp.washChamberFullStroke(1,2,80);
	//cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65);
	te.powerOn();
	te.wait();
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,false);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,15,true);
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(1,2,62);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp,14);
	takePicture(FITC,"FITCControl.tif",focusExp,4);
	takePicture(CY5,"Cy5Control.tif",Cy5exp,14);
	system("pause");
	pmp.washWithChamberVol(0,5,62);//quick low salt wash;
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	pmp.fillWithReagent(2,false);//bring in biotin 9mer ligation mixture
	cout<<"Performing 60 minute Ligation"<<endl;
	pmp.wait();
	takePicture(FITC,"FITCt0.tif",focusExp,4,true);
	takePicture(CY5,"Cy5t0.tif",Cy5exp,14);
	takePicture(BF,"BFt0.tif",BFexp,14);
	Timer::wait(60000*30);
	focus->move(fineAutofocus(20.0));//focus 30um
	takePicture(CY5,"Cy5t1.tif",Cy5exp,14);
	takePicture(FITC,"FITCt1.tif",focusExp,4);
	takePicture(BF,"BFt1.tif",BFexp,14);
	cout<<"washing and bringing in Alexa Label"<<endl;
	pmp.washChamberFullStroke(1,4,62);//wash with high salt
	pmp.fillWithReagent(3,true);//recycle the alexa dye
	Timer::wait(60000*10);
	takePicture(CY5,"Cy5-50msBeforeHSWash.tif",Cy5exp/2,14);
	takePicture(CY5,"Cy5-100msBeforeHSWash.tif",Cy5exp,14);
	takePicture(CY5,"Cy5-200msBeforeHSWash.tif",Cy5exp*2,14);
	takePicture(FITC,"FITCBeforeHSWash.tif",focusExp,4);
	takePicture(BF,"BFBeforeHSWash.tif",BFexp,14);
	pmp.washChamberFullStroke(1,4,62);
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	focus->move(fineAutofocus(20.0));//focus 20um
	takePicture(CY5,"Cy5-50msAfterHSWash.tif",Cy5exp/2,14);
	takePicture(CY5,"Cy5-100msAfterHSWash.tif",Cy5exp,14);
	takePicture(CY5,"Cy5-200msAfterHSWash.tif",Cy5exp*2,14);
	takePicture(FITC,"FITCAfterHSWash.tif",focusExp,4);
	takePicture(BF,"BFAfterHSWash.tif",BFexp,14);
	te.record=false;
	cout<<"SBL Single Step 100nM 9mer Ligation DONE!!\n\n"<<endl;
}

void Controller::LigationPhotocleave(){
	int numCycles=3;
	vector<double> exposures;
	vector<int> gains;
	exposures.push_back(.050);exposures.push_back(.2);exposures.push_back(.5);
	gains.push_back(4);gains.push_back(8);gains.push_back(20);
	focusChannel=FITC;
	focusExp=.05;
	focusGain=12;
	float BFexp=.1;
	int BFgain=6;
	te.setFixedTemp(26);
	setWorkingDir("D:\\Eric\\PhotocleaveBiotinSatMoreTemplateBeads\\");
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();
	pmp.washChamberFullStroke(0,2,80);
	focus->SetHalogenVoltage(1.8);
	focus->SetObj(Scope::OBJ_20);
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65,true);
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,false);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,5,true);
	te.setFixedTemp(32,true);
	for (int i=1;i<=numCycles;i++){
		cout<<"High Salt Washing"<<endl;
		pmp.washChamberFullStroke(0,5,62);//high salt wash not too fast
		focus->move(fineAutofocus(20.0));//focus 20um
		cout<<"Acquiring control images"<<endl;
		takePicture(BF,"Lig"+toString(i,2)+"BFControl.tif",BFexp,BFgain);
		getMultiplePics("Lig"+toString(i,2),"Control",CY5,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"Control",CY3,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"Control",FITC,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"Control",DAPI,exposures,gains);
		cout<<"Prepare Cycle "<<i<<" Ligation Reagent"<<endl;
		//system("pause");
		pmp.washWithChamberVol(1,5,62);//quick low salt wash;
		cout<<"Bringing in 9mer Ligation mixture"<<endl;
		pmp.fillWithReagent(2,true);//bring in dendrimer 4uM and recycle it
		cout<<"Performing 60 minute Ligation"<<endl;
		Timer::wait(60000*60);
		cout<<"Ligation Step Complete"<<endl;
		pmp.washChamberFullStroke(0,10,62);
		focus->move(fineAutofocus(20.0));
		takePicture(BF,"Lig"+toString(i,2)+"BFAfterHSWash.tif",BFexp,BFgain);
		getMultiplePics("Lig"+toString(i,2),"AfterHSWash",CY5,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"AfterHSWash",CY3,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"AfterHSWash",FITC,exposures,gains);
		getMultiplePics("Lig"+toString(i,2),"AfterHSWash",DAPI,exposures,gains);
		for (int j=1;j<=10;j++){
			cout<<"Exposing to UV for 1 minute"<<endl;
			UVExpose(60*1);
			pmp.washWithChamberVol(0,1,62);
			focus->move(fineAutofocus(10.0));
			takePicture(BF,"Lig"+toString(i,2)+"BFAfterUVMin"+toString(j)+".tif",BFexp,BFgain);
			getMultiplePics("Lig"+toString(i,2),"AfterUVMint"+toString(j,3)+"c5.tif",CY5,exposures,gains);
			getMultiplePics("Lig"+toString(i,2),"AfterUVMint"+toString(j,3)+"c4.tif",CY3,exposures,gains);
			getMultiplePics("Lig"+toString(i,2),"AfterUVMint"+toString(j,3)+"c3.tif",FITC,exposures,gains);
			getMultiplePics("Lig"+toString(i,2),"AfterUVMint"+toString(j,3)+"c2.tif",DAPI,exposures,gains);
			//system("pause");
		}
	}
	te.record=false;
	te.powerOff();
	cout<<"SBL Photocleave two step Ligation DONE!!\n\n"<<endl;
}

void Controller::Ligation100nMAlexa(){
	//R1=Anchor R2=Ligase  W0=low salt W1=high salt	
	Timer t;
	focusChannel=5;//Cy5
	cam->setExposure(.2);
	focusExp=.04;
	float Cy5exp=.2;
	float BFexp=.04;
	cam->setGain(12);
	focusGain=12;
	focus->SetHalogenVoltage(1.8);
	focus->SetObj(Scope::OBJ_20);
	setWorkingDir("D:\\Eric\\Alexa100nMSingleStepLigation002\\");
	te.setFixedTemp(26);
	te.powerOn();
	pmp.washChamberFullStroke(0,2,80);
	//cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65);
	te.powerOn();
	te.wait();
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,true);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,15,true);
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(1,2,62);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp);
	takePicture(CY5,"Cy5Control.tif",Cy5exp);
	system("pause");
	pmp.washWithChamberVol(0,5,62);//quick low salt wash;
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	
	pmp.fillWithReagent(2,false);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 60 minute Ligation"<<endl;
	t.startTimer();
	for(int i=0;i<120;i++){
		Cy5exp=.05;
		t.waitAfterLastStart(i*60000);//every 1min for 60minutes
		cout<<"Acquiring images for "<<(i)<<" minute timepoint"<<endl;
		if ((i%5)==0) takePicture(CY5,"Cy5endminust"+toString(60-i,3)+"c5.tif",Cy5exp,-1,true);//focus 5um
		else takePicture(CY5,"Cy5endminust"+toString(60-i,3)+"c5.tif",Cy5exp);
		takePicture(BF,"BFendminust"+toString(60-i,3)+"c0.tif",BFexp);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(1,4,62);//wash with high salt
	pmp.wait();
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(CY5,"Cy5AfterHSWash.tif",Cy5exp,-1,true);//focus 5um
	takePicture(BF,"BFAfterHSWash.tif",BFexp);
	te.record=false;
	cout<<"SBL Single Step 100nM 9mer Ligation DONE!!\n\n"<<endl;
}

void Controller::Ligation10nM(){
	//R0=beads R1=Anchor R2=Ligase  W0=low salt W1=high salt	
	Timer t;
	focusChannel=5;//Cy5
	cam->setExposure(.04);
	focusExp=.04;
	float Cy5exp=.04;
	float BFexp=.04;
	cam->setGain(12);
	focusGain=12;
	focus->SetHalogenVoltage(1.6);
	focus->SetObj(Scope::OBJ_20);
	setWorkingDir("D:\\Eric\\10nMSingleStepLigation001\\");
	te.setFixedTemp(26);
	te.powerOn();
	pmp.washChamberFullStroke(0,2,80);
	//cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65);
	te.powerOn();
	te.wait();
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,true);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,15,true);
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(1,2,62);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp);
	takePicture(CY5,"Cy5Control.tif",Cy5exp);
	pmp.washWithChamberVol(0,5,62);//quick low salt wash;
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	system("pause");
	pmp.fillWithReagent(2);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 30 minute Ligation"<<endl;
	t.startTimer();
	for(int i=0;i<16;i++){
		t.waitAfterLastStart(i*2*60000);
		cout<<"Acquiring images for "<<(i*2.0)<<" minute timepoint"<<endl;
		takePicture(CY5,"Cy5endminust"+toString(15-i,3)+"c5.tif",Cy5exp,-1,true);//focus 5um
		takePicture(BF,"BFendminust"+toString(15-i,3)+"c0.tif",BFexp);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(1,4,62);//wash with high salt
	pmp.wait();
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(CY5,"Cy5AfterHSWash.tif",Cy5exp,-1,true);//focus 5um
	takePicture(BF,"BFAfterHSWash.tif",BFexp);
	te.record=false;
	cout<<"SBL Single Step 100nM 9mer Ligation DONE!!\n\n"<<endl;
}


void Controller::SBL1nMSingleStepLigation32DegC(){
	//R0=beads R1=Anchor R2=Ligase  W0=high salt W1=low salt
	
	vector<double> exposures;
	vector<int> gains;
	exposures.push_back(.030);exposures.push_back(.050);exposures.push_back(.1);exposures.push_back(.2);exposures.push_back(.5);
	gains.push_back(4);gains.push_back(8);gains.push_back(12);
	focusChannel=FITC;
	focusExp=.05;
	focusGain=4;
	float BFexp=.2;
	int BFgain=6;
	focus->SetHalogenVoltage(1.8);
	focus->SetObj(Scope::OBJ_10);
	setWorkingDir("D:\\Eric\\1nMSingleStepLigation32DegC002\\");
	te.setFixedTemp(26,true);
	pmp.washChamberFullStroke(0,2,62);
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65,true);
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,false);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,5,true);
	te.setFixedTemp(32,true);
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(0,2,62);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp,BFgain);
	takePicture(FITC,"FITCControl.tif",focusExp,focusGain);
	getMultiplePics("Control","",CY5,exposures,gains);
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	system("pause");
	pmp.washWithChamberVol(1,5,62);//quick low salt wash;
	pmp.fillWithReagent(2,false);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 30 minute Ligation"<<endl;
	Timer t;
	t.startTimer();
	int numMinutes=60;
	int i=0;
	
	for(i=0;i<=numMinutes;i++){
		t.waitAfterLastStart(i*60000);
		//focus->SetHalogenVoltage(1.6);
		cout<<"Acquiring images for "<<i<<" minute timepoint"<<endl;
		takePicture(Controller::FITC,"FITCMin"+toString(i,2)+"c3.tif",focusExp,focusGain,true);//focus 5um
	//	takePicture(BF,"BFMin"+toString(i,2)+"c0.tif",BFexp,BFgain);
		getMultiplePics("","t"+toString(numMinutes-i,3)+"c5",CY5,exposures,gains);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(0,4,62);//wash with high salt
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(FITC,"FITCAfterHSWash.tif",focusExp,focusGain,true);
	takePicture(BF,"BFAfterHSWash.tif",BFexp,BFgain);
	getMultiplePics("AfterHSWash","",CY5,exposures,gains);
	te.record=false;
	te.powerOff();
	cout<<"SBL Single Step 1nM 9mer Ligation DONE!!\n\n"<<endl;
}

void Controller::SBL10nMSingleStepLigation32DegC(){
	//R0=beads R1=Anchor R2=Ligase  W0=high salt W1=low salt
	Timer t;
	vector<double> exposures;
	vector<int> gains;
	exposures.push_back(.1);exposures.push_back(.2);
	gains.push_back(4);gains.push_back(12);gains.push_back(16);
	focusChannel=FITC;
	focusExp=.2;
	focusGain=12;
	float BFexp=.2;
	int BFgain=6;
	focus->SetHalogenVoltage(1.8);
	focus->SetObj(Scope::OBJ_20);
	setWorkingDir("D:\\Eric\\10nMSingleStepLigation32DegC\\");
	te.setFixedTemp(26,true);
	pmp.washChamberFullStroke(0,2,62);
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65,true);
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,true);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,5,true);
	te.setFixedTemp(32,true);
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(0,2,62);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp,BFgain);
	takePicture(FITC,"FITCControl.tif",focusExp,focusGain);
	getMultiplePics("Control","",CY5,exposures,gains);
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	system("pause");
	pmp.washWithChamberVol(1,5,62);//quick low salt wash;
	pmp.fillWithReagent(2);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 30 minute Ligation"<<endl;
	t.startTimer();
	int numMinutes=30;
	int i=0;
	for(i=0;i<=numMinutes;i++){
		t.waitAfterLastStart(i*60000);
		cout<<"Acquiring images for "<<i<<" minute timepoint"<<endl;
		takePicture(FITC,"FITCMin"+toString(i,2)+"c3.tif",focusExp,focusGain,true);//focus 5um
		takePicture(BF,"BFMin"+toString(i,2)+"c0.tif",BFexp,BFgain);
		getMultiplePics("Min"+toString(i,2),"t"+toString(numMinutes-i,3)+"c5",CY5,exposures,gains);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(0,4,62);//wash with high salt
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(FITC,"FITCAfterHSWash.tif",focusExp,focusGain,true);
	takePicture(BF,"BFAfterHSWash.tif",BFexp,BFgain);
	getMultiplePics("AfterHSWash","",CY5,exposures,gains);
	te.record=false;
	te.powerOff();
	cout<<"SBL Single Step 10nM 9mer Ligation DONE!!\n\n"<<endl;
}

void Controller::SBLSingleStepLigation(){
	//R0=beads R1=Anchor R2=Ligase  W0=low salt W1=high salt
	
	Timer t;
	focusChannel=5;//Cy5
	cam->setExposure(.1);
	focusExp=.1;
	float Cy5exp=.1;
	float BFexp=.1;
	cam->setGain(4);
	focusGain=4;
	focus->SetHalogenVoltage(2.1);
	focus->SetObj(Scope::OBJ_20);
	setWorkingDir("D:\\Eric\\10nMSingleStepLigation001\\");
	te.setFixedTemp(26);
	te.powerOn();
	liveView();
	
	////pmp.washChamberFullStroke(0,2,80);
	////cam->waitTemp();
	//cout<<"Please select position for experiment and then exit"<<endl;
	//liveView();//get position for acquisition
	//	cout<<"Raising Temperature to 65 deg C"<<endl;
	//te.recordTemps2(1000,dir+"Temps.txt");
	//te.setFixedTemp(65);
	//te.powerOn();
	//te.wait();
	//cout<<"Bringing in Anchor reagent"<<endl;
	//pmp.fillWithReagent(1,true);
	//cout<<"Performing temperature controlled hybridization"<<endl;
	//te.linearTempRamp(26,15,true);
	//cout<<"High Salt Washing"<<endl;
	//pmp.washChamberFullStroke(1,2,62);//high salt wash not too fast
	//te.recordTemps2(1000,dir+"Temps.txt");
	
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp);
	takePicture(CY5,"Cy5Control.tif",Cy5exp);
	pmp.washWithChamberVol(0,5,62);//quick low salt wash;
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	//system("pause");
	pmp.fillWithReagent(2);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 30 minute Ligation"<<endl;
	t.startTimer();
	for(int i=0;i<61;i++){
		t.waitAfterLastStart(i*60000);
		cout<<"Acquiring images for "<<i<<" minute timepoint"<<endl;
		takePicture(CY5,"Cy5endminust"+toString(60-i,3)+"c5.tif",Cy5exp,-1,true);//focus 10um
		takePicture(BF,"BFendminust"+toString(60-i,3)+"c0.tif",BFexp);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(1,4,62);//wash with high salt
	pmp.wait();
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(CY5,"Cy5AfterHSWash.tif",Cy5exp,-1,true);//focus 10um
	takePicture(BF,"BFAfterHSWash.tif",BFexp);
	te.record=false;
	cout<<"SBL Single Step 100nM 9mer Ligation DONE!!\n\n"<<endl;
}

void Controller::heatAndWash(){
	focusChannel=5;//Cy5
	focusExp=.035;
	int gain=6;
	cam->setGain(6);
	acquireBFexp=.1;
	focus->SetHalogenVoltage(1.2);
	float Cy5exp=.035;
	float Cy3exp=.035;
	dir="D:\\Eric\\HeatAndWashAfterSBLSingleStepLigation003\\";
	long int err=SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS) cout<<"create directory failed"<<endl;
//	pmp.reagentPorts[0]="B2";//Anchor
	//pmp.reagentPorts[1]="B4";//Ligation mixture containing 9mer comp and non comp
	//pmp.reagentPorts[2]="";
	//pmp.washPorts[0]="B1";//low salt
	//pmp.washPorts[1]="B3";//high salt
	//pmp.washPorts[2]="B5";//milliQ water for dehybridization
	//pmp.washPorts[3]="";
	//pmp.stepsAirGap=60/pmp.uLperStep;
	//pmp.stepsChamb=20/pmp.uLperStep;
	//pmp.stepsOver=30/pmp.uLperStep;
	//cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	pmp.initialize();
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(25);
	te.powerOn();
	pmp.washChamber(1,1);//high salt wash not too fast
	pmp.wait();
	double center=focus->getPos();
	double bestZ=autoFocus(20,10);
	focus->move(bestZ);
	cout<<"Autofocus adjustment by "<<abs((double)(bestZ-center))<<"um from "<<center<<"um to "<<bestZ<<"um"<<endl;
	pmp.washChamber(2,1);//milliQ wash
	char buffer[16];
	char buf[16];
	CImg<unsigned short>* washingCy5;
	CImg<unsigned short>* washingBF;
	for(int j=0;j<12;j++){//temp ramp from 25 to 70 by 5 degrees
		te.setFixedTemp(25+j*5);
		cout<<"Setting temperature to "<<(25+j*5)<<"deg C"<<endl;
		te.wait(.5);	
		Timer::wait(60000);
		pmp.wait();
		center=focus->getPos();
		bestZ=autoFocus(20,10);
		focus->move(bestZ);
		cout<<"Autofocus adjustment by "<<abs((double)(bestZ-center))<<"um from "<<center<<"um to "<<bestZ<<"um"<<endl;
		focus->SetTurret(Scope::TURRET_BF);
	focus.halogenOn();
	focus->wait();
	cam->takePicture(acquireBFexp,gain,1);
	cam->waitIdle();
	focus.halogenOff();
	focus->SetTurret(Scope::TURRET_PINKEL);
	
	washingBF=cam->getNewPic();
	focus->wait();
		filt->switchFilter(5);//Cy5
		cam->takePicture(Cy5exp,gain,1);
		cam->waitIdle();
		filt->closeShutter();
		pmp.washChamber(2,1);//milliQ wash
		washingCy5=cam->getNewPic();
		sprintf(buffer,"%03d",j);
		cam->saveTiff(dir+"BF_"+string(itoa(25+j*5,buf,10))+"degCwashing_t"+string(buffer)+"c6.tif",washingBF);
		cam->saveTiff(dir+"Cy5_"+string(itoa(25+j*5,buf,10))+"degCwashing_t"+string(buffer)+"c5.tif",washingCy5);
		delete washingCy5;
		delete washingBF;
	}
	te.record=false;
	te.powerOff();
	cout<<"Heat and Wash Complete!!"<<endl<<endl;
}

void Controller::SBLSingleStepLigationWithNonComp(){	
		Timer swatch;
	focusChannel=5;//Cy5
	focusExp=.1;
	int gain=6;
	cam->setGain(gain);
	acquireBFexp=.1;
	focus->SetHalogenVoltage(1.2);
	float Cy5exp=.1;
	float Cy3exp=.1;
	dir="D:\\Eric\\SBLSingleStepLigationWithNonComp001\\";
	long int err=SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS) cout<<"create directory failed"<<endl;
//	pmp.reagentPorts[0]="B2";//Anchor
	//pmp.reagentPorts[1]="B4";//Ligation mixture with 9mer comp and non-comp
	//pmp.reagentPorts[2]="";
	//pmp.washPorts[0]="B1";//low salt
	//pmp.washPorts[1]="B3";//high salt
	//pmp.washPorts[2]="";
	//pmp.stepsAirGap=60/pmp.uLperStep;
	//pmp.stepsChamb=20/pmp.uLperStep;
	//pmp.stepsOver=30/pmp.uLperStep;
	cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	pmp.initialize();
	pmp.washChamber(0, 2);//wash with low salt
	pmp.washChamber(0);
	pmp.washChamber(1);
	cout<<"Raising Temperature to 70 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(70);
	te.powerOn();
	te.wait();
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(0);
	pmp.oscillateChamber(50);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,15,true);
	te.setFixedTemp(26);
	pmp.oscillating=false;
	cout<<"Recycling Anchor reagent"<<endl;
	cout<<"High Salt Washing"<<endl;
	pmp.washChamber(1);//high salt wash not too fast
	pmp.wait();
	te.wait();
	double center=focus->getPos();
	double bestZ=autoFocus();
	focus->move(bestZ);
	cout<<"Autofocus adjustment by "<<abs((double)(bestZ-center))<<"um from "<<center<<"um to "<<bestZ<<"um"<<endl;
	cout<<"Acquiring control images"<<endl;
	focus->SetTurret(Scope::TURRET_BF);
	focus.halogenOn();
	focus->wait();
	cam->takePicture(acquireBFexp,gain,1);
	cam->waitIdle();
	focus.halogenOff();
	focus->SetTurret(Scope::TURRET_PINKEL);
	CImg<unsigned short>* controlBF=cam->getNewPic();
	focus->wait();
	filt->switchFilter(4);//Cy3
	cam->takePicture(Cy3exp,gain,1);
	cam->waitIdle();
	CImg<unsigned short>* controlCy3=cam->getNewPic();
	filt->switchFilter(5);//Cy5
	cam->takePicture(Cy5exp,gain,1);
	cam->waitIdle();
	CImg<unsigned short>* controlCy5=cam->getNewPic();
	filt->closeShutter();
	pmp.washChamber(0);
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	pmp.fillWithReagent(1);//bring in 9mer Cy5 ligation mixture
	Camera::saveTiff(dir+"controlBF.tif",controlBF);
	Camera::saveTiff(dir+"controlCy3.tif",controlCy3);
	Camera::saveTiff(dir+"controlCy5.tif",controlCy5);
	char buf[16];
	pmp.wait();
	cout<<"Performing 30 minute Ligation"<<endl;
	swatch.startTimer();
	for(int i=0;i<31;i++){
		delete controlBF;
		delete controlCy3;
		delete controlCy5;
		swatch.waitAfterLastStart(i*60000);
		cout<<"Acquiring images for "<<i<<" minute timepoint"<<endl;
		pmp.fill4Image();
		pmp.wait();
		if (!(i%5)) {
			cout<<"re-autofocusing"<<endl;
			center=focus->getPos();
			bestZ=autoFocus(5,2.5);
			focus->move(bestZ);
			cout<<"Autofocus adjustment by "<<abs((double)(bestZ-center))<<"um from "<<center<<"um to "<<bestZ<<"um"<<endl;
		}
		focus->SetTurret(Scope::TURRET_BF);
		focus.halogenOn();
		focus->wait();
		cam->takePicture(acquireBFexp,gain,1);
		cam->waitIdle();
		focus.halogenOff();
		focus->SetTurret(Scope::TURRET_PINKEL);	
		controlBF=cam->getNewPic();
		focus->wait();
		filt->switchFilter(3);//Cy3
		cam->takePicture(Cy3exp,gain,1);
		cam->waitIdle();
		controlCy3=cam->getNewPic();
		filt->switchFilter(5);//Cy5
		cam->takePicture(Cy5exp,gain,1);
		cam->waitIdle();
		controlCy5=cam->getNewPic();
		filt->closeShutter();
		pmp.resumeAfterImage();
		sprintf(buf,"%03d",30-i);
		Camera::saveTiff(dir+"BFminut"+string(buf)+"c6.tif",controlBF);
		Camera::saveTiff(dir+"Cy3minut"+string(buf)+"c4.tif",controlCy3);
		Camera::saveTiff(dir+"Cy5minut"+string(buf)+"c5.tif",controlCy5);
	}
	cout<<"recycling ligation reagent"<<endl;
	pmp.wait();
	cout<<"washing with high salt"<<endl;
	pmp.washChamber(1);//wash with high salt
	delete controlBF;
	delete controlCy3;
	delete controlCy5;
	pmp.wait();
	focus->move(autoFocus());
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	focus->SetTurret(Scope::TURRET_BF);
	focus.halogenOn();
	cam->takePicture(acquireBFexp,gain,1);
	cam->waitIdle();
	focus.halogenOff();
	focus->SetTurret(Scope::TURRET_PINKEL);	
	controlBF=cam->getNewPic();
	cam->showImage(controlBF,"DEBUGBF");
	focus->wait();
	filt->switchFilter(4);//Cy3
	cam->takePicture(Cy5exp,gain,1);
	cam->waitIdle();
	controlCy3=cam->getNewPic();
	filt->switchFilter(5);//Cy5
	cam->takePicture(Cy5exp,gain,1);
	cam->waitIdle();
	controlCy5=cam->getNewPic();
	filt->closeShutter();
	cam->showImage(controlCy5,"DEBUGCy5");
	Camera::saveTiff(dir+string(buf)+"AfterHighSaltWashBF.tif",controlBF);
	Camera::saveTiff(dir+string(buf)+"AfterHighSaltWashCy3.tif",controlCy3);
	Camera::saveTiff(dir+string(buf)+"AfterHighSaltWashCy5.tif",controlCy5);
	delete controlBF;
	delete controlCy3;
	delete controlCy5;
	te.record=false;
	cout<<"SBL Single Step 9mer comp and non-comp Ligation DONE!!\n\n"<<endl;
}


void Controller::SBLPrepareArray(){
	Timer swatch;
	focus.halogenOff();
	dir="D:\\Eric\\SBLPrepareSlideArray001\\";
	long int err=SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS) cout<<"create directory failed"<<endl;
//	pmp.reagentPorts[0]="B2";//Dynal Beads with template 120uL
	//pmp.reagentPorts[1]="B4";//Neutravidin 120uL
	//pmp.reagentPorts[2]="B6";//Free Biotin 120uL
	//pmp.reagentPorts[3]="";
	//pmp.washPorts[0]="B1";//SSCT
//	pmp.washPorts[1]="B3";//ethanol
//	pmp.washPorts[2]="";
	//pmp.stepsAirGap=60/pmp.uLperStep;
	//pmp.stepsChamb=100;
	//pmp.stepsOver=pmp.stepsChamb/3;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(37);
	te.powerOn();
	te.wait();
	pmp.washChamber(0);
	cout<<"Load Dynal Beads"<<endl;
	system("pause");
	pmp.primeReagent(0);
	pmp.washChamber(0);
	pmp.fillWithReagent(0);//bring in dynal beads
	pmp.oscillateChamber();
	cout<<"Apply Magnet to fill wells"<<endl;
	system("pause");
	//pmp.oscillating=false;
	pmp.washChamber(0);
	pmp.washChamber(1);
	pmp.washChamber(0);
	pmp.primeReagent(1);
	pmp.washChamber(0);
	pmp.fillWithReagent(1);//fill with neutravidin
	//pmp.oscillateChamber(pmp.chamberVol,"S30");
	Timer::wait(1000*60*120);//2hour wait
	pmp.oscillating=false;
	pmp.washChamber(0);
	pmp.primeReagent(2);
	pmp.washChamber(0);
	pmp.fillWithReagent(2);//fill with biotin
	//pmp.oscillateChamber(pmp.chamberVol,"S30");
	Timer::wait(1000*60*120);//2hour wait
	pmp.oscillating=false;
	pmp.washChamber(0);
	te.record=false;
	te.powerOff();
	pmp.wait();
	cout<<"Slide ready for ligation!!"<<endl;
}

void Controller::SBL200nmFluoLigation(){
	Timer swatch;
	focus.halogenOff();
	dir="D:\\Eric\\SBL200nmFluoLigation001\\";
	long int err=SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS) cout<<"create directory failed"<<endl;
//	pmp.reagentPorts[0]="B2";//Anchor
	//pmp.reagentPorts[1]="B4";//Ligation
	//pmp.reagentPorts[2]="B6";//Fluospheres
	//pmp.reagentPorts[3]="";
	//pmp.washPorts[0]="B1";//high salt
	//pmp.washPorts[1]="B3";//low salt
	//pmp.washPorts[2]="B5";//HPHWB
	//pmp.washPorts[3]="";//SSCT
	//pmp.reagentSpeed=62;
	//pmp.stepsAirGap=60/pmp.uLperStep;
//	pmp.stepsChamb=100;//20/pmp.uLperStep;
	//pmp.stepsOver=pmp.stepsChamb/3;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(65);
	te.powerOn();
	te.wait();
	pmp.washChamber(0);
	cout<<"Prepare tubing for Anchor (reagent 0)"<<endl;
	system("pause");
	cout<<"Bringing in Anchor for 30 minute hybridization"<<endl;
	pmp.primeReagent(0);
	pmp.washChamber(0);
	pmp.fillWithReagent(0);
	//pmp.oscillateChamber(pmp.chamberVol,"S30");
	te.linearTempRamp(25,30,true);
	//pmp.oscillating=false;
	//pmp.wait();
	pmp.washChamber(1);
	cout<<"Prepare tubing for Ligation mixture (reagent 1)"<<endl;
	system("pause");
	cout<<"Bringing in Ligation mixture for 30 minute ligation"<<endl;
	pmp.primeReagent(1);
	pmp.washChamber(1);
	pmp.fillWithReagent(1);
	Timer::wait(1000*60*30);
	pmp.washChamber(0);
	cout<<"Prepare tubing for fluospheres (reagent 2)"<<endl;
	system("pause");
	cout<<"Bringing in 200nm Fluospheres for 10 minute incubation"<<endl;
	pmp.primeReagent(2);
	pmp.washChamber(0);
	pmp.fillWithReagent(2);
	Timer::wait(1000*60*10);
	cout<<"Performing extensive wash with High pH SSC"<<endl;
	pmp.washChamber(2);
	te.powerOff();
	te.record=false;
	cout<<"200nm Fluosphere Ligation Complete"<<endl;
}

void Controller::testProtocol(){
		//R0=beads R1=Anchor R2=Ligase  W0=low salt W1=high salt
	Timer t;
	focusChannel=5;//Cy5
	focusExp=.1;
	double ws=500;
	pmp.reagentSpeed=500;
	focusGain=6;
	float Cy5exp=.1;
	float BFexp=.1;
	cam->setGain(10);
	focus->SetObj(Scope::OBJ_20);
	focus->SetHalogenVoltage(2.1);
	setWorkingDir("D:\\Eric\\100nMSingleStepLigation001\\");
	pmp.washChamberFullStroke(0,1,ws);
	//cam->waitTemp();
	cout<<"Please select position for experiment and then exit"<<endl;
	liveView();//get position for acquisition
	pmp.washChamberFullStroke(0,1,ws);//wash with low salt
	cout<<"Raising Temperature to 65 deg C"<<endl;
	te.recordTemps2(1000,dir+"Temps.txt");
	te.setFixedTemp(35);
	te.powerOn();
	te.wait();
	cout<<"Bringing in Anchor reagent"<<endl;
	pmp.fillWithReagent(1,true);
	pmp.oscillateChamber(30,1000);
	cout<<"Performing temperature controlled hybridization"<<endl;
	te.linearTempRamp(26,.1,true);
	pmp.stopOsc();
	cout<<"High Salt Washing"<<endl;
	pmp.washChamberFullStroke(1,1,ws);//high salt wash not too fast
	focus->move(fineAutofocus(20.0));//focus 20um
	cout<<"Acquiring control images"<<endl;
	takePicture(BF,"BFControl.tif",BFexp);
	takePicture(CY5,"Cy5Control.tif",Cy5exp);
	pmp.washWithChamberVol(0,1,ws);//quick low salt wash;
	cout<<"Bringing in 9mer Ligation mixture"<<endl;
	pmp.fillWithReagent(2);//bring in 9mer Cy5 ligation mixture
	cout<<"Performing 30 minute Ligation"<<endl;
	t.startTimer();
	for(int i=0;i<3;i++){
		t.waitAfterLastStart(i*60000);
		cout<<"Acquiring images for "<<i<<" minute timepoint"<<endl;
		takePicture(CY5,"Cy5endminust"+toString(30-i,3)+"c5.tif",Cy5exp,-1,true);//focus 5um
		takePicture(BF,"BFendminust"+toString(30-i,3)+"c0.tif",BFexp);
	}
	cout<<"washing with high salt"<<endl;
	pmp.washChamberFullStroke(1,1,ws);//wash with high salt
	pmp.wait();
	cout<<"acquiring final ligation images after high salt wash"<<endl;
	takePicture(CY5,"Cy5AfterHSWash.tif",Cy5exp,-1,true);//focus 10um
	takePicture(BF,"BFAfterHSWash.tif",BFexp);
	te.record=false;
	cout<<"SBL Single Step 100nM 9mer Ligation DONE!!\n\n"<<endl;

}

*/
