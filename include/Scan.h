// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Saturday, March 12, 2011</lastedit>
// ===================================
#pragma once
#include <vector>
#include "AcquisitionSeries.h"
#include "ImageProperties.h"
#include "Scanfocus.h"
class Camera;

class Scan{
	friend Camera;
protected:
	Scan(std::vector<AcquisitionChannel>& acquisitionChannels,string workingDir,ScanFocus *sf=NULL,int showOnScreenPeriod=1);
	Record *scanLogFile;
	std::vector<std::vector<std::vector<int>>> x,y;
	std::vector<std::vector<std::vector<double>>> z,temp,temp2;
	std::vector<std::vector<std::vector<std::string>>> timeStamp; //"YYYY:MM:DD HH:MM:SS.mmm"
	HANDLE abortEvent;
	HANDLE generationCompleteEvent;
	HANDLE readyForStartEvent;
public:
	//member variables
	ScanFocus* sf;//if this is null we don't have pre focusing capabilities
	std::vector<AcquisitionSeries*> acquisitionSeries;
	//std::vector<AcquisitionChannel> acquisitionChannels;we dont need this...everything is in acquisitionSeries
//	std::vector<int> imageNumToChanNum;//vector that converts image number to channel number based on dummy acquisitions (due to slow mechanical movements of the microscope or pauses between acquisitions during a kinetics)
	string workingDirectory;
	std::vector<HANDLE> saveSeriesThreads;
	int showOnScreenPeriod;//5 is every fifth spot is shown (i.e. 0,5,10,15 BUT only if the acquisition channel says so)   show at every spot would be 1
	HANDLE hStartScan;//to coordinate dat2tiff and camera 
	int scanTotal;
	//real methods
	//Eric's runScan with internal tracking of total number of scans
	void runScan(int numScan=1,bool wait=true);
	static unsigned __stdcall runScanThread(void* param);
	void _runScan(int numScan);
	HANDLE triggerStartEvent;
	void enableStartEvent();
	void disableStartEvent();

	//after sending startevent you should wait for generation complete before sending another startevent
	void sendStartEvent();

	bool waitGenerationComplete();
	bool waitReadyForStartEvent();
	//Ying-Ja's runScan with external tracking of total number of scans
	void runScan(int numScan, int scanNum);
	//imagenum in current series
	bool getImageProperties(int imageNum,int seriesNum, ImageProperties &ip,int camNum=-1);//return false if it is not to be saved (i.e. dummy acquisition)
	AcquisitionChannel getChan(int chanNum);
	//if these channels cannot be acquired in one acquisitionGroup we will return false and nothing will be added
	bool addAcquisitionGroup(std::vector<AcquisitionChannel> acquisitionChannels, int numLocalSpotChanges=0);
	
	void groupChannels2Series(std::vector<AcquisitionChannel> acquisitionChannels);
	void wait();//this must be called by the derived class' destructor to ensure the scan object is no longer needed 
	HANDLE hRunScanThread;
	//stop all save threads and stop runScan thread
	void abort();
	//virtual methods
	virtual ~Scan()=0;
	virtual void move2NextSpotGlobal(ScanFocus *sf=NULL, int spotNum=-1)=0;//update x,y and focus 
	virtual void move2NextSpotLocal(ScanFocus *sf=NULL, int spotNum=-1)=0;//update x,y and focus 
	virtual int numSpotsGlobal()=0;//return num spots for global movements.  All acquisition series will be performed beginning at each global spot. Within each global spot, the scan may move to local spots, which is determined by a Scan subclass.
	int numSpotsLocal(){return _numSpotsLocal;}//this is not virtual because it can be determined from the vector of acquisition series. A Scan subclass should check this value to see if it supports that number of local movements.
	virtual void getImageProperties(int scanNum,int spotNum,int chanNum,string& fileName,string& comment,int camNum=-1)=0;//return false if it is not to be saved (i.e. dummy acquisition)
	virtual void selectScanRegionGUI()=0;
	virtual void getScanRegion(int &minX,int &maxX,int &minY,int &maxY)=0;
	int numChannels();
	vector<int> chanToSeries;
private:
	int _numSpotsLocal;

	
	//3D arrays to keep track of image properties during scan 
	//scanNum, spotNum, chanNum
	
};

