// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Scan.h"
#include <string>
class GridScan: public Scan{
public:
	GridScan::GridScan(std::vector<AcquisitionChannel>& acquisitionChannels,int showOnScreenPeriod=1);
	~GridScan(void);
	void move2NextSpot();//update x,y and focus 
	int numSpots();//return num spots per scan
	void getImageProperties(int scanNum,int spotNum,int chanNum,string& fileName,string& comment);//return false if it is not to be saved (i.e. dummy acquisition)
	void selectScanRegionGUI(AcquisitionChannel viewChan);
private:
	std::vector<int> x,y;	//what were the 3 dimensions?
	std::vector<double> z;
};

/*
double Controller::getFocus(int x,int y){
	if (!focusAdjust) return focus->getPos();
	double ret;
	NR::splin2(*(this->y),*(this->x),*(this->z),*secondDer,y,x,ret);
	return ret;
}*/


/*
void Controller::getScanRegion(){
	char c;
	unsigned threadID;
	cout<<"Now defining scan region.\n";
	prepareChannel(selectLiveViewChannel());
getregion:	HANDLE thread=(HANDLE) _beginthreadex(NULL,0,Camera::LiveThread,cam,0,&threadID);
	CloseHandle(thread);
	cout<<"Move stage to start of scan region.\nExit Stage Control when done.\n";
	stageControl();
	scanMinX=stg->getX();
	scanMinY=stg->getY();
	cout<<"Coordinate ("<<scanMinX<<","<<scanMinY<<") accepted\n";
	cout<<"Move stage to end of scan region.\nExit Stage Control when done.\n";
	stageControl();
	scanMaxX=stg->getX();
	scanMaxY=stg->getY();
	cout<<"Coordinate ("<<scanMaxX<<","<<scanMaxY<<") accepted\n";
	//we could take the floor of this if we want
	cam->liveDisp->close();
	lightOff();
	double imageWidth=(cam->maxXPixel-cam->minXPixel+1)*cam->getPixelSize()/(focus->getTotalMag()*stg->getStepSize());//X steps per image
	double imageHeight=(cam->maxYPixel-cam->minYPixel+1)*cam->getPixelSize()/(focus->getTotalMag()*stg->getStepSize());//Y steps per image
	cout<<"Image Width is "<<imageWidth*stg->getStepSize()<<" microns.  Image Height is "<<imageHeight*stg->getStepSize()<<" microns"<<endl;
	xStep=(int) floor(imageWidth*(1-stg->getOverlap()/100));
	yStep=(int) floor(imageHeight*(1-stg->getOverlap()/100));
	cout<<"Each move in x="<<xStep<<" steps.  Each move in y="<<yStep<<" steps."<<endl;
	stepsX=(int) ceil((scanMaxX-scanMinX)/(imageWidth*(1-stg->getOverlap()/100))+1);//(imageWidth*(1-overlap/100)));
	stepsY=(int) ceil((scanMaxY-scanMinY)/(imageHeight*(1-stg->getOverlap()/100))+1);//(imageHeight*(1-overlap/100)));
	cout<<"Number of steps in x direction: "<<dec<<stepsX<<endl;
	cout<<"Number of steps in y direction: "<<dec<<stepsY<<endl;
	cout<<"Total number of fields to acquire: "<<dec<<stepsX*stepsY<<endl;
	if (stepsX<=0 ||stepsY<=0) {
		cout<<endl<<endl<<"End of scan region must have coordinates larger than start of scan region, please try again"<<endl<<endl<<endl;
		goto getregion;
		return;
	}
	diffX=stepsX*xStep;
	diffY=stepsY*yStep;
	cout<<"Total move in X: "<<diffX<<" steps.  Total move in Y: "<<diffY<<" steps."<<endl;
	RecordFile <<"Start scan region from coordinate ("<<scanMinX<<","<<scanMinY<<").\n"
				<<"End scan region from coordinate ("<<scanMaxX<<","<<scanMaxY<<").\n"
				<<"Number of steps in x direction: "<<dec<<stepsX<<endl
				<<"Number of steps in y direction: "<<dec<<stepsY<<endl
				<<"Total move in X: "<<diffX<<" steps.\n"
				<<"Total move in Y: "<<diffY<<" steps."<<endl;
}


void GridScan::move2NextSpot(){
for (int k=0;k<numChannels;k++){
		if (del[k]>2)
		{
			filt->triggerBoth(0,0);
			if (ch[k]=-1) focus.halogenOn(bf[k],false);
			else focus.halogenOff(false);
			focus->SetTurret(turret[k]);
			fWheel->SwitchEmFilter(fw[k]);
			stg->wait();
			focus->wait(true);
		}
		filt->triggerBoth(del[k], exps[k]);//d->start();	
	}
	while(j<stepsY){//move to each row
		for(;;){//move to each column
			//if (acquireBF){
			//	focus.halogenOn(-1,false);
			//	BFexp.resetTimer();
			//}
			////take picture
			////wait for stuff to stop moving
			//stg->wait();
			//focus->wait();
			//logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString(stg->getX())+" Y:"+toString(stg->getY())+ " ZPos:"+toString( focus->getPos()),DEBUGCONTROLLER);
			//if (acquireBF){
			//	//d->triggerCam();
			//	filt->triggerCam();
			//	BFexp.startTimer();
			//	//if (exps[0]!=-1) d->prepareTrigger();
			//	BFexp.waitAfterStart(acquireBFexp);
			//	if (exps[0]!=-1) {
			//		focus.halogenOff(true);
			//		focus->wait();
			//	}
			//}
			cout<<"i="<<i<<" j="<<j<<endl;
			//if (exps[0]!=0) {
			//	filt->triggerBoth(numChannels+1, del, exps, turret, fw, focus, fWheel);//d->start();
			//}
			for (int k=0;k<numChannels;k++){
				if (del[k]>2)
				{
					filt->triggerBoth(0,0);
					if (ch[k]=-1) focus.halogenOn(bf[k],false);
					else focus.halogenOff(false);
					if (k==0){
						if ((i==stepsX-1 && j%2==0) || (i==0&&j%2==1)){
							j++;
							if (j==stepsY) break;
							stg->incY(yStep);//stepDown();
						}else if (j%2==0) {
							stg->incX(xStep);//stepRight();
							i++;
						}else {
							stg->incX(-xStep);//stepLeft();
							i--;
						}
						if (j%2==0){//moving right
							focu=getFocus(scanMinX+xStep*i,scanMinY+yStep*j);
							focus->move(focu);
						}else{//moving left
							focu=getFocus(scanMinX+xStep*(stepsX-1-i),scanMinY+yStep*j);
							focus->move(focu);
						}
					}
					focus->SetTurret(turret[k]);
					fWheel->SwitchEmFilter(fw[k]);
					stg->wait();
					focus->wait(true);
				}
				filt->triggerBoth(del[k], exps[k]);//d->start();	
			}
			//Sleep(200);
			//if (!acquireBF) d->prepareTrigger();
		}
	}
}
*/