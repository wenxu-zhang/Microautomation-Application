// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, April 04, 2012</lastedit>
// ===================================
#pragma once
#include "Controller.h"
#include "Scan.h"
#include "Definitions.h"
#include "Chamber.h"
#include "DefiniteFocus.h"
//using namespace control;
 extern Controller cont;
 extern Record logFile;
 
class ProtocolEric{
public:
	static void runProtocol();

	//global functions not specific to a microscope
	static void expose(int numFOVs, double numFOVSpacingY,double time, AcquisitionChannel* channel);
	static void DefiniteFocus();
	//chamber 0 is left 1 is right and 2 is both
	//channel 0 is left 1 is middle 2 is right
	//section 0 is bottom 1 is center 2 is top
	static void imageMultipleFOVs(customProtocol& scan);
	static void imageSingleFOV(int FOVNum, customProtocol& scan);
	static bool loadProtocol(string filename,customProtocol& scan);
	static bool selectProtocol(customProtocol& scan);
	static bool reloadProtocolList();
	static unsigned __stdcall dfThread(void* param);
	static unsigned __stdcall kineticsScanThread(void* param);
	static unsigned __stdcall omnicureThread(void* param);
	static unsigned __stdcall electrophoresisThread(void* param);
#ifdef AXIOVERT

	static void AmplifiedProtocols();

	static void nSBSHomopolymer();
	static void photocleave(double numFOVSpacingY,double numFOVSpacingX,Focus *f1,Focus*f2,double seconds, vector<vector<AcquisitionChannel>> &FOVChannels);
	
	//static void protocol1();
#endif
#ifdef OBSERVER

	//READS Protocols
	static void kineticsScan(customProtocol& scan);

	//Erin's Protocols
	static void electrophoresis(customProtocol& scan);
	static void electrophoresis2(customProtocol& scan);
	static void electrophoresis3(customProtocol& scan);

	//SBS Protocols
	static void SBSFirstExperiment();

	static void customScan();
	static void timeSeries(customProtocol& scan);
	static void timeSeries2(customProtocol& scan);
	static void timeSeries3(customProtocol& scan);
	static void reagentKinetics(customProtocol& scan);


	//SBL Protocols

	static void testEvent();
	static void testFocus();
	static void testPosition();
	static void TIRFadjust();
	static void intensityAdjust();
	static void getFocus(double range, double step);
	static void cyclicAmplified();
	static void pumpTest();
	static void testProtocol();
	static void channelPics();
	static void ligationKinetics();
	static void ligationKineticsAccum();
	static void photocleavageKinetics();
	

	//SBD protocols
	static void focusEvaluation();
	static void testExperiment();
	

	//pump protocols
	static void pumpProtocols();

	static void testAirLoad(int start,int end);
	static double mmTubing2uL(double mm);
	static double mmChannel2uL(double mm);

	static void loadChannelsAirGap(int start, int end);
	static void washChannels(int start, int end);
	static void airGap(int chan);
	static void loadChannel(int chan);
	static void washChannel(int chan);

	//static void takeTIRFImages(double numFOVSpacing,Focus* f1,Focus*f2,vector<vector<AcquisitionChannel>> &FOVChannels);
	//static void takeTIRFBleachImages(double sampleSec,double totalSec,double numFOVSpacing,Focus*f1,Focus*f2,std::vector<vector<AcquisitionChannel>> &FOVChannels);
	//static void takeBleachImages(double sampleSec,double totalSec,double numFOVSpacing,Focus*f1,Focus*f2,std::string root,std::vector<vector<AcquisitionChannel>> &FOVChannels);
	//static void takeBleachImagesFOV(double sampleSec,double totalSec,double numFOVSpacing,Focus* f1,Focus* f2,std::string root,std::vector<AcquisitionChannel> &channels);
	
	
	static void washAndImage(customProtocol& scan);
	static void photocleaveAndImage(double seconds,customProtocol& scan);
#endif
};

//it may have been more elegant to implement this as a scan with multiple channels depending on the total time series.
//was a hack really necessary?
class DefiniteFocusScan:public Scan{
public:
	string namePrefix;
	DefiniteFocusScan(std::vector<AcquisitionChannel>& acquisitionChannels,int numSpots,double FOVspacing,int direction,string namePrefix="");
	int num;
	int direction;
	double FOVspacing;
	DefiniteFocus dfs;
	void move2NextSpotGlobal(ScanFocus* sf=NULL,int spotNum=-1);
	void move2NextSpotLocal(ScanFocus* sf=NULL,int spotNum=-1);
	int numSpotsGlobal();
	void getImageProperties(int scanNum,int spotNum,int chanNum,string& fileName,string& comment, int camNum=-1){
		if (numSpotsGlobal()>1)
			fileName=string("FOV")+toString(spotNum,3);
		else
			fileName=namePrefix;
		AcquisitionChannel a=this->getChan(chanNum);
		AcquisitionChannel* i=&a;
		if (i->chan->tirf){
				fileName=fileName+"Image"+::toString(chanNum,2)+i->chan->toString(i->out,camNum)+"m"+i->m->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain(camNum))+"i"+i->chan->lite().ls->intensityToString(i->intensity)+"a"+::toString(i->TIRFangle);
		}else
			fileName=fileName+"Image"+::toString(chanNum,2)+i->chan->toString(i->out,camNum)+"m"+i->m->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain(camNum))+"i"+i->chan->lite().ls->intensityToString(i->intensity);
	
	}
	void selectScanRegionGUI(){}
	void getScanRegion(int &minX,int &maxX,int &minY,int &maxY){}
	~DefiniteFocusScan();
};




