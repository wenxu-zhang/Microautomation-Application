// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include "Scan.h"
class SpotSeriesScan:public Scan{
private:
	double period;
	double totalTime;
	vector<double> times;
	vector<double> actualTimes;
	Timer t;
	HANDLE h;
	int direction;
	double FOVspacing;
	bool newFOV;
	void (*globalFunc)(int);//
	int numGroups;//how many times do we need to send trigger event to trigger all groups
public:
	vector<double> getActualTimes(){return actualTimes;}
	static void nothing(int){}


	SpotSeriesScan(AcquisitionChannel ac, string workingDir,double timeSec, double period=0, ScanFocus* sf=NULL);
	

	SpotSeriesScan(AcquisitionChannel ac, string workingDir,vector<double> timeSec,ScanFocus* sf=NULL, bool newFOV=false, int direction=1,double FOVspacing=2,void (*f)(int)=nothing);
	
	//constructor to take multiple channels at arbitrary time points with focus update between time points only. Need to trigger all AcquisitionGroups following each focus update
	SpotSeriesScan(std::vector<AcquisitionChannel> ac, string workingDir,vector<double> timeSec,ScanFocus* sf=NULL, bool newFOV=false, int direction=1,double FOVspacing=2,void (*f)(int)=nothing);
	

	//first set of images will be taken without focus update   //second set will be as normal
	SpotSeriesScan(AcquisitionChannel ac, string workingDir,vector<double> timeSecsFirst,vector<double> timeSecsSecond,ScanFocus* sf);
	~SpotSeriesScan();
	static vector<AcquisitionChannel> createACvector(AcquisitionChannel ac, double timeSec, double period=0,ScanFocus* sf=NULL);
	static vector<AcquisitionChannel> createACvector(AcquisitionChannel ac, vector<double> timeSec,ScanFocus* sf=NULL);
	void move2NextSpotGlobal(ScanFocus *sf=NULL,int spotNum=-1);
	void move2NextSpotLocal(ScanFocus *sf=NULL,int spotNum=-1);
	int numSpotsGlobal();
	void getImageProperties(int scanNum,int spotNum,int chanNum, string& fileName,string& comment,int camNum=-1);
	void selectScanRegionGUI(){}
	void getScanRegion(int& x1,int& y1,int &x2,int &y2);
	void setDuration(double timeSecs);

	void sendStartEvent();

	static unsigned __stdcall spotSeriesThread(void* param);

	void runScan(int numScan,bool wait);
	void abort();
};