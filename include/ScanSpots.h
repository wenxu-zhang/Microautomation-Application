// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Scan.h"

class ScanSpots :
	public Scan
{
public:
	ScanSpots::ScanSpots(std::vector<AcquisitionChannel>& acquisitionChannels,string dir,ScanFocus *sf=0,int showOnScreenPeriod=1);
	~ScanSpots(void);
	void move2NextSpotGlobal(ScanFocus *sf=NULL,int spotNum=-1);//update x,y and focus 
	void move2NextSpotLocal(ScanFocus *sf=NULL,int spotNum=-1){};
	int numSpotsGlobal();//return num spots per scan
	void getImageProperties(int scanNum,int spotNum,int chanNum,string& fileName,string& comment,int camNum=-1);//return false if it is not to be saved (i.e. dummy acquisition)
	void selectScanRegionGUI(AcquisitionChannel viewChan);
	void getScanRegion(int &minX,int &maxX,int &minY,int &maxY);
	void selectScanRegionGUI(){}
	void recordFocus(int numScan=1, int scanNum=1); //test function to autofocus at every spot and record the z positions.
private:
	std::vector<int> x,y;	//what were the 3 dimensions?
	std::vector<double> z;
	int currentSpot;
};
