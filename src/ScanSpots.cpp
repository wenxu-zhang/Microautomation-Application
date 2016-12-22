// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, June 16, 2011</lastedit>
// ===================================
#include "ScanSpots.h"
#include "Controller.h"
#include "XYStage.h"
#include<limits>
using namespace std;
extern Controller cont;
extern Record logFile;

ScanSpots::ScanSpots(std::vector<AcquisitionChannel>& acquisitionChannels,string dir,ScanFocus *sf,int showOnScreenPeriod)
:Scan(acquisitionChannels,dir,sf,showOnScreenPeriod)
{
	cout<<"For selecting scan region, ";
	cont.currentChannel().modify();
	selectScanRegionGUI(cont.currentChannel());
	currentSpot=0;
}
ScanSpots::~ScanSpots(void){
}
void ScanSpots::move2NextSpotGlobal(ScanFocus *sf,int spotNum){
	cont.stg->move(x[currentSpot],y[currentSpot]);
	//scanLogFile->write(toString(cont.stg->getX()+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ()),true,"move in ScanSpots");
	if(sf) {
		//double focuspos=sf->getFocus(x[currentSpot],y[currentSpot]);
		cont.focus->move(sf->getFocus(x[currentSpot],y[currentSpot]));
		//logFile.write("move focus for pos"+toString(currentSpot)+" focuspos="+toString(focuspos),true,"ScanSpots::move2NextSpot");
		cont.focus->wait();
		//logFile.write("after focus wait: "+toString(cont.focus->getZ()),true,"ScanSpots::move2NextSpot");
	}else{
		cont.focus->move(z[currentSpot]);
		cont.focus->wait();
	}
	//scanLogFile->write(toString(cont.stg->getX()+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ()),true,"move in ScanSpots");
	if(currentSpot<numSpotsGlobal()-1 && numSpotsGlobal()!=1)
		currentSpot++;
	else //Reset currentSpot when last position is reached.
		currentSpot=0;
}//update x,y and focus 
int ScanSpots::numSpotsGlobal(){
	return x.size();
}//return num spots per scan
void ScanSpots::getImageProperties(int scanNum,int spotNum,int chanNum,string& fileName,string& comment,int camNum){
	fileName=string("t")+toString(scanNum,3)+string("p")+toString(spotNum,3)+string("c")+toString(chanNum);
	comment=string("scanSpots");
}//return false if it is not to be saved (i.e. dummy acquisition)
void ScanSpots::selectScanRegionGUI(AcquisitionChannel viewChan){
	viewChan.on();
	viewChan.out->cam->startLiveView();
	int c=1;
	while (c!=0){
		cout<<"Move stage to next position to scan.\n";
		cont.stg->stageControl();
		cout<<"Enter 1 to accept, 2 to reject, 0 when finished.\n";
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case 1:
				x.push_back(cont.stg->getX());
				y.push_back(cont.stg->getY());
				z.push_back(cont.focus->getZ());
				scanLogFile->write("Position accepted: "+toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ()),true);
			case 2:
			case 0: break;
		}
	}
	viewChan.out->cam->stopLiveView();
}
void ScanSpots::getScanRegion(int &minX,int &maxX,int &minY,int &maxY){
	minX=x[0];
	maxX=x[0];
	minY=y[0];
	maxY=y[0];
	for(int i=1;i<numSpotsGlobal();i++){
		if(x[i]<minX) minX=x[i];
		if(x[i]>maxX) maxX=x[i];
		if(y[i]<minY) minY=y[i];
		if(y[i]>maxY) maxY=y[i];
	}
}
void ScanSpots::recordFocus(int numScan, int scanNum){
	string fileName,comment;
	Focus focus=cont.currentFocus;
	double tempz;
	if(scanNum==1) scanLogFile->write("x\ty\tz\tTemp\ttime",true);
	for(int n=0;n<numScan;n++){
		for(int p=0;p<numSpotsGlobal();p++){
			tempz=cont.focus->getZ();
			move2NextSpotGlobal();
			cont.focus->move(tempz);
			cont.stg->wait();
			int cnum=0;
			for(int i=0;i<numChannels();i++){
				AcquisitionChannel a=getChan(i);
				a.on();
				focus.a=a;
				a.wait();
				focus.getFocus(true);
				cont.focus->wait();
				scanLogFile->write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp()),true,"ScanSpots::recordFocus");
				getImageProperties(n+scanNum,p,0,fileName,comment);
				a.takePicture(fileName);
				cont.te.comment="scan: "+toString(n+scanNum)+" pos: "+toString(p)+" "+fileName+"\t"+toString(cont.focus->getZ());
				cont.te.triggerRecord=true;
				scanLogFile->write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp()),true,"ScanSpots::recordFocus");
			}
		}
	}
	if (numChannels() >= 1){
		getChan(numChannels()-1).off();
		cout<<"Scan completed.\n";
		cont.setCurrentChannel(getChan(numChannels()-1));
	}
}