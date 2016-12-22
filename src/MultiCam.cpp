// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, February 24, 2011</lastedit>
// ===================================
#include "MultiCam.h"
#include <process.h>
#include "Controller.h"
extern Controller cont;

using namespace std;

struct StartSeriesArg{
	vector<Camera*>* cameras;
	int numImages;
	int seriesNum;
	Scan* scan;
	AcquisitionParameters ap;
	vector<HANDLE> *saveThreads;
	int num;
};

struct MasterThreadArg{
	vector<Camera*> *cameras;
	vector<HANDLE> *saveThreads;
};


vector<int> getTriggerLines(vector<Camera*> cameras){
	vector<int> ret;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		ret.push_back((*c)->triggerLines.front());		
	}
	return ret;
}

MultiCam::MultiCam(vector<Camera*> cameras):Camera(cameras.front()->t,getTriggerLines(cameras)),cameras(cameras),startSeriesThreads(NULL),saveThreads(NULL){
}

MultiCam::~MultiCam(){
}

void MultiCam::abort(){
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->abort();
	}
}

HANDLE MultiCam::startSeries(int numImages,int seriesNum, Scan* scan,int camNum){
	//call startSeries for every camera and then spawn single master thread which waits for file saving for every camera.  The single master thread then waits for all saveThreads to finish
	
	unsigned threadID;
	HANDLE masterCopy;
	
	int i=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){
		StartSeriesArg *arg=new StartSeriesArg();
		arg->cameras=&cameras;
		arg->numImages=numImages;
		arg->seriesNum=seriesNum;
		arg->scan=scan;
		arg->num=i;
		arg->saveThreads=&saveThreads;
		startSeriesThreads.push_back((HANDLE) _beginthreadex(NULL,0, &startSeriesThread,arg,0,&threadID));
	}
	MasterThreadArg *m=new MasterThreadArg();
	m->cameras=&cameras;
	m->saveThreads=&saveThreads;
	for(vector<HANDLE>::iterator i=startSeriesThreads.begin();i!=startSeriesThreads.end();i++){
		WaitForSingleObject(*i,INFINITE);
	}
	startSeriesThreads.clear();
	HANDLE master=(HANDLE) _beginthreadex(NULL,0, &masterThread,m,0,&threadID);
	bool b=DuplicateHandle(GetCurrentProcess(),
					master,
					GetCurrentProcess(),
					&masterCopy,
					0,
					false,
					DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(masterCopy);
	return master;
}

unsigned __stdcall MultiCam::startSeriesThread(void* param){
	StartSeriesArg *arg=(StartSeriesArg*) param;
	arg->saveThreads->push_back(arg->cameras->at(arg->num)->startSeries(arg->numImages,arg->seriesNum,arg->scan,arg->num));
	delete arg;
	return 0;
}

unsigned __stdcall MultiCam::masterThread(void* param){
	MasterThreadArg *m=(MasterThreadArg *) param;
	for(vector<HANDLE>::iterator i=m->saveThreads->begin();i!=m->saveThreads->end();i++){
		WaitForSingleObject(*i,INFINITE);
	}
	m->saveThreads->clear();
	delete m;
	return 0;
}

HANDLE MultiCam::startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan *s){
		//spawn single master thread which calls startseries for each camera.  The single master thread then waits for all saveThreads to finish
	unsigned threadID;
	HANDLE masterCopy;
	StartSeriesArg *arg=new StartSeriesArg();
	arg->cameras=&cameras;
	arg->numImages=numImages;
	arg->seriesNum=seriesNum;
	arg->scan=s;
	arg->ap=ap;
	HANDLE master=(HANDLE) _beginthreadex(NULL,0, &masterThread2,arg,0,&threadID);
	bool b=DuplicateHandle(GetCurrentProcess(),
					master,
					GetCurrentProcess(),
					&masterCopy,
					0,
					false,
					DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(master);
	return master;
}

unsigned __stdcall MultiCam::masterThread2(void* param){
	StartSeriesArg *arg=(StartSeriesArg*) param;
	vector<HANDLE> saveThreads;
	for(vector<Camera*>::iterator c=arg->cameras->begin();c!=arg->cameras->end();c++){
		saveThreads.push_back((*c)->startSeries2(arg->numImages,arg->ap,arg->seriesNum,arg->scan));
	}
	for(vector<HANDLE>::iterator i=saveThreads.begin();i!=saveThreads.end();i++){
		WaitForSingleObject(*i,INFINITE);
	}
	delete arg;
	return 0;
}

float MultiCam::getPixelSize(){
	float ret=cameras.front()->getPixelSize();
	float sz;
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		sz=(*c)->getPixelSize();
		if (sz!=ret){
			logFile.write("Warning: Multicam pixel sizes are different",true);
			ret=max(ret,sz);
		}
	}
	return ret;
}

int MultiCam::getBitDepth(){
	int ret=cameras.front()->getBitDepth();
	int sz;
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		sz=(*c)->getBitDepth();
		if (sz!=ret){
			logFile.write("Warning: Multicam bit depths are different",true);
			ret=min(ret,sz);
		}
	}
	return ret;	
}

double MultiCam::getTemp(){
	double ret=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		ret+=(*c)->getTemp();
	}
	ret=ret/cameras.size();
	return ret;
}


void MultiCam::waitTemp(){
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->waitTemp();
	}
}


//void saveSeries(int num,string fileName,const AcquisitionChannel &ac){this->dat2Tiff(num,fileName,ac);}
double MultiCam::startFocusSeries(int num,AcquisitionParameters ap){
	double ret=cameras.front()->startFocusSeries(num,ap);
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		if (ret!=(*c)->startFocusSeries(num,ap))
			logFile.write("startFocusSeries error: real exposure times are not the same, we need external triggering?");
	}
	return ret;
}


double MultiCam::startStepwiseFocusSeries(int num, AcquisitionParameters ap){
	double minTrig=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		minTrig=max(minTrig,(*c)->startStepwiseFocusSeries(num,ap));
	}
	return minTrig;
}


int MultiCam::getBestFocus(){
	int val=cameras.front()->getBestFocus();
	int newval;
	double avg=val;
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		newval=(*c)->getBestFocus();
		if (val!=newval)
			logFile.write("Warning: getBestFocus returned different images for different cameras, taking the average");
		avg+=newval;
	}
	avg=avg/cameras.size();
	return round(avg);
}


LONGLONG MultiCam::takePicture(AcquisitionChannel* ac, std::string fileName){
	LONGLONG ret=0;
	if (fileName=="temp"){
		for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
			ret=max(ret,(*c)->takePicture(ac,"temp"));
		}
	}else{
		int i=0;
		for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){
			ret=max(ret,(*c)->takePicture(ac,fileName+"cam"+::toString(i,1)));
		}
	}
	return ret;
}


void MultiCam::takeAccumulation(AcquisitionChannel* ac, int num, std::string fileName){
	logFile.write("take Accumulation not supported for multiple cameras",true);
}


void MultiCam::startLiveView(LiveViewData lvd){
	bool sameFOV=true;
	double w,h;
	double iW=cameras.front()->getImageWidth(cont.currentChannel().ap);
	double iH=cameras.front()->getImageHeight(cont.currentChannel().ap);
	//double ratio=iWidth/iHeight;
	int nW=1;//sqrt((double)cameras.size());
	int nH=cameras.size();//ceil(double(cameras.size())/nW);
	double currDiff=abs(nW*iW/(nH*iH)-double(lvd.width)/lvd.height);
	while(nH>1){//we can add more to the width and decrease height
		if (currDiff>abs((nW+ceil(double(nW)/(nH-1)))*iW/((nH-1)*iH)-double(lvd.width)/lvd.height)){
			nW=nW+ceil(double(nW)/(nH-1));
			nH=nH-1;
		}else
			break;
//		if (nW*iW/(nH*iH)>width/height)//we can fill the width, try less width
//			nW
	}

	int pixelWidth=floor(double(lvd.width)/nW);
	int pixelHeight=floor(double(lvd.height)/nH);
	int tempX;
	int tempY;
	int i=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){

		w=(*c)->getImageWidth(cont.currentChannel().ap);
		h=(*c)->getImageHeight(cont.currentChannel().ap);
		sameFOV=sameFOV&&iW==w&&iH==h;
		tempX=lvd.x+pixelWidth*(int(i-nW*floor(double(i)/nW))%nW);
		tempY=lvd.y+pixelHeight*ceil(double(i)/nW);
		(*c)->startLiveView(LiveViewData(tempX,tempY,pixelWidth,pixelHeight,&scaleFactor,&centerX,&centerY,&zoom,&centerXEff,&centerYEff,i));
	}
	if (!sameFOV)
		logFile.write("Warning: Multiple cameras FOV size mismatch.  All images will not have the same FOV",true);
}

void MultiCam::waitLiveViewStart(){
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->waitLiveViewStart();
	}
}


void MultiCam::saveLiveImage(std::string fileName){
	int i=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){
		(*c)->saveLiveImage(fileName+"cam"+::toString(i,1));
	}
}


void MultiCam::stopLiveView(){
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->stopLiveView();
	}
}


bool MultiCam::isLiveView(){
	bool b=false;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		b=b||(*c)->isLiveView();
	}
	return b;
}


double MultiCam::getImageWidth(AcquisitionParameters& ap){
	double ret=cameras.front()->getImageWidth(ap);
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		if (ret!=(*c)->getImageWidth(ap))
			logFile.write("Warning: Image widths are not the same across cameras, this needs to be handled post acquisition",true);
	}
	return ret;
}


double MultiCam::getImageHeight(AcquisitionParameters& ap){
	double ret=cameras.front()->getImageHeight(ap);
	for(vector<Camera*>::iterator c=cameras.begin()+1;c!=cameras.end();c++){
		if (ret!=(*c)->getImageHeight(ap))
			logFile.write("Warning: Image heights are not the same across cameras, this needs to be handled post acquisition",true);
	}
	return ret;
}


void MultiCam::wait(){
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->wait();
	}
}

AcquisitionParameters MultiCam::getDefaultAcquisitionParameters(){
	AcquisitionParameters ap;
	ap.gains=vector<int>(cameras.size(),0);
	int i=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){
		AcquisitionParameters temp=(*c)->getDefaultAcquisitionParameters();
		ap.gains.at(i)=temp.getGain();
	}
	return ap;
}
bool MultiCam::validate(AcquisitionParameters& ap){
	bool b=true;
	if (ap.gains.size()!=cameras.size()){
		logFile.write("Acqusition Channel does not have the correct number of gains, setting gains to defaults",true);
		ap=getDefaultAcquisitionParameters();
	}
	AcquisitionParameters apTemp(ap);
	vector<int> temp;
	int i=0;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++,i++){
		apTemp.setGain(ap.gains.at(i));
		b=b&&(*c)->validate(apTemp);
		temp.push_back(apTemp.getGain());		
	}
	ap.setGain(temp);
	return b;
}

//adjust intensity on each camera separately, need to take more pictures but this is easier for now
void MultiCam::adjustIntensity(AcquisitionChannel& a,double percentSaturation){
	double min=numeric_limits<double>::max( );
	double minTemp;
	AcquisitionChannel best;
	AcquisitionChannel aTemp=a;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		(*c)->adjustIntensity(aTemp,percentSaturation);
		minTemp=aTemp.	ap.exp*aTemp.ap.getGain();
		if (minTemp<min){
			best=aTemp;
			min=minTemp;
		}
		aTemp=a;
	}
}


bool MultiCam::isTriggerable(AcquisitionParameters& ap){
	bool b=true;
	for(vector<Camera*>::iterator c=cameras.begin();c!=cameras.end();c++){
		b=b&&(*c)->isTriggerable(ap);
	}
	return b;
}

