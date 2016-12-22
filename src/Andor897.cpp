#include "Definitions.h"
#include "Andor897.h"
#include "tiffio.h" 
#include <iostream>
//#include <ilut.h>
#include <process.h>
#include <sstream>
#include <math.h>
#include "atmcd32d.h"    //andor function definitions
#include "Controller.h"
#include "Timer.h"
#include "ImageProperties.h"
#include "Utils.h"
#include "Magnification.h"
#include <Winerror.h>
extern Record logFile,focusLogFile;
#include "DisplayList.h"
extern Record logFile;
extern Controller cont;
extern HANDLE AndorSelectCamMutex;
using namespace cimg_library;
typedef unsigned char *CStr;


#define Andor897SelCam(functionCall,camPtr) WaitForSingleObject(AndorSelectCamMutex, INFINITE);\
											camPtr->getError(SetCurrentCamera(camPtr->camHandle));\
											camPtr->getError(functionCall);\
											ReleaseMutex(AndorSelectCamMutex);

struct Score{
	cimg_library::CImg<unsigned short>** images;
	int ind;
	LONGLONG* result;
	double* times;
};


struct ThreadGroup{
	int num;
	HANDLE threads[MAXIMUM_WAIT_OBJECTS];
};

struct Accum897{
	int num;
	string fileName;
	Andor897* c;
	AcquisitionChannel* ac;
};

void Andor897::getError(unsigned int err_val){
	//system("pause");
	if (err_val!=DRV_SUCCESS){
		logFile.write(string("Andor897 Driver Error Code: ")+::toString((int)err_val), true);
		int stat;
		GetStatus(&stat);
		logFile.write(string("Andor897 Camera ")+toString(this->camHandle)+" status code:"+toString(stat),true);
		ReleaseMutex(AndorSelectCamMutex);
		clickAbort();//this calls endthread so execution stops
		return;		
	}
}

struct Arg{
	CImg<unsigned short>* img;
	CImgDisplay* disp;
	//string title;
};

struct Button{
	CImgDisplay* d;
	HANDLE h;
};

//doubly linked list
//replaced by class Disp
struct Image{
	CImgDisplay* disp;
	Image* next;
	Image* prev;
};

struct Series{
	Andor897* a;
	int numImages;
	int seriesNum;
	string tempDir;
	Scan* scan;
	int camNum;
	Series(Andor897* a,int numImages,int seriesNum,string tempDir,Scan* scan, int camNum):a(a),numImages(numImages),seriesNum(seriesNum),tempDir(tempDir),scan(scan),camNum(camNum){}
};

//abstract methods implementation
HANDLE Andor897::startSeries(int numImages,int seriesNum, Scan* scan, int camNum){
	unsigned int err1;
	waitIdle();
	static int dirNum=-1;
	static int maxInt=std::numeric_limits<int>::max();
	static int length=toString(maxInt).length();
	dirNum++;
	string tempDir=string(ANDOR897TEMPDIR)+::toString(dirNum,length)+"\\";
	long int err=SHCreateDirectoryEx(NULL,tempDir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS){
		logFile.write(string("Error: startSeries could not create the temporary directory: ")+tempDir,true);
		clickAbort();
	}
	unsigned threadID;
	HANDLE saveThreadCopy;
	HANDLE saveThread=(HANDLE) _beginthreadex(NULL, 0, &dat2Tiff, new Series(this,numImages,seriesNum,tempDir,scan,camNum), 0, &threadID);//don't forget to delete the new Series in the dat2tiff thread when done
	bool b=DuplicateHandle(GetCurrentProcess(),
					saveThread,
					GetCurrentProcess(),
					&saveThreadCopy,
					0,
					false,
					DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(saveThread);
	AcquisitionParameters ap=scan->acquisitionSeries[seriesNum%scan->acquisitionSeries.size()]->acquisitionGroups[0]->acquisitionChannels[0].ap;
	this->bin=ap.bin;
	this->getImageRegion(ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	this->startKineticsFTExtTrig(tempDir,numImages+1,ap.bin,ap.getGain(camNum));//need to take one more picture because the first image saved (upon triggering) is a dummy image
	return saveThreadCopy;
}

HANDLE Andor897::startSeries2(int numImages,AcquisitionParameters ap,int seriesNum,Scan *s){
	waitIdle();
	static int dirNum=std::numeric_limits<int>::max();
	static int length=toString(dirNum).length();
	dirNum--;
	string tempDir=string(ANDOR897TEMPDIR)+::toString(dirNum,length)+"\\";
	long int err=SHCreateDirectoryEx(NULL,tempDir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS){
		logFile.write(string("Error: startSeries could not create the temporary directory: ")+tempDir,true);
		clickAbort();
	}
	unsigned threadID;
	HANDLE saveThreadCopy;
	HANDLE saveThread=(HANDLE) _beginthreadex(NULL, 0, &dat2Tiff, new Series(this,numImages,seriesNum,tempDir,s,-1), 0, &threadID);
	bool b=DuplicateHandle(GetCurrentProcess(),
					saveThread,
					GetCurrentProcess(),
					&saveThreadCopy,
					0,
					false,
					DUPLICATE_SAME_ACCESS);
	cont.threads.addThread(saveThread);
	this->bin=ap.bin;
	this->getImageRegion(ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	this->startInternalFTKinetics(tempDir,numImages,ap.exp,ap.bin,ap.gain);
	return saveThreadCopy;
}

double Andor897::startFocusSeries(int num,AcquisitionParameters ap){
	unsigned threadID;
	double ret;
		this->getImageRegion(ap,minXPixel,minYPixel,maxXPixel,maxYPixel);

		ret=this->startFTKinetics(num,ap.exp,ap.bin,ap.gain);
		focus=(HANDLE) _beginthreadex(NULL,0,FocusThread,this,0,&threadID);
	return ret;
}

double Andor897::startStepwiseFocusSeries(int num, AcquisitionParameters ap){
	unsigned threadID;
	waitIdle();
	ResetEvent(this->newImage);
	this->getImageRegion(ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	double ret=this->startNonFTKineticsExtTrig(num,ap.exp,ap.bin,ap.gain);
	focus=(HANDLE) _beginthreadex(NULL,0,FocusThread,this,0,&threadID);
	return ret;
}

void Andor897::terminateSeries(){
	CheckExists()
	AbortAcquisition();
}

void Andor897::abort(){
	CheckExists()
	isPresent=false;
	DWORD result1=WaitForSingleObject(AndorSelectCamMutex, 2000);
	if (result1==WAIT_TIMEOUT) ReleaseMutex(AndorSelectCamMutex);
	SetCurrentCamera(this->camHandle);							
	TerminateThread(focus,0);
	if (result1!=WAIT_TIMEOUT) AbortAcquisition();
	if (live){
		_isLiveView=false;
		DWORD result2=WaitForSingleObject(live,2000);
		if (result2==WAIT_TIMEOUT){
			TerminateThread(live,0);
			TerminateThread(buttonWait,0);
		}
		CloseHandle(live);
		CloseHandle(buttonWait);
		live=NULL;
		buttonWait=NULL;
	}
	if (focus) CloseHandle(focus);
	this->heatUp();
	if (result1!=WAIT_TIMEOUT) SetDriverEvent(NULL);
	CloseHandle(newImage);	
	if (result1!=WAIT_TIMEOUT) SetShutter(0,2,0,0);
	if (result1!=WAIT_TIMEOUT) ShutDown();
	ReleaseMutex(AndorSelectCamMutex);
}

int Andor897::getBestFocus(){
	WaitForSingleObject(focus,INFINITE);
	DWORD ret;
	GetExitCodeThread(focus,&ret); 
	CloseHandle(focus);
	focus=0;
	return (int) ret;
}

void Andor897::takeAccumulation(AcquisitionChannel* ac, int num, string fileName){
	Accum897 a;
	a.fileName=fileName;
	a.num=num;
	a.c=this;
	a.ac=ac;
	this->getImageRegion(ac->ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	this->startFTKinetics(num,ac->ap.exp,ac->ap.bin,ac->ap.gain);
	ac->on(true);
	this->trigger();
	unsigned threadID;
	HANDLE save=(HANDLE) _beginthreadex(NULL,0,SaveAccum,&a,0,&threadID);
	waitIdle();
	ac->off();
	WaitForSingleObject(save,INFINITE);
}

unsigned __stdcall Andor897::SaveAccum(void* param){
	Accum897* a=(Accum897*) param;
	long acc,series;
	for(int i=0;i<a->num;i++){
		Andor897SelCam(GetAcquisitionProgress(&acc,&series),a->c)
		if (!a->c->isIdle())
			WaitForSingleObject(a->c->newImage,INFINITE);
		saveTiff(a->fileName+"a"+::toString(i,2),a->c->getOldest(),string("AccumImage:")+::toString(i)+" "+a->ac->toString());
	}
	return 0;
}

LONGLONG Andor897::takePicture(AcquisitionChannel* ac, string fileName){
	LONGLONG ret=0;
	static int temp=0;
	if (fileName=="temp"){
		temp++;
		fileName=string(ANDOR897TEMPDIR)+fileName+::toString(temp,5);
	}
	static AcquisitionChannel c(*ac);
	static double max=0;
	if (c.chan==ac->chan && c.m==ac->m){//make sure we will not over-expose
		//same channel, same mag
		double guessMax=max*(ac->intensity/c.intensity)*(ac->ap.exp/c.ap.exp)*(ac->ap.gain/c.ap.gain)*(ac->ap.bin/c.ap.bin)*(ac->ap.bin/c.ap.bin);
		if (guessMax>ANDOR897MAXPIXELVAL*0.95) {
			logFile.write("ANDOR: Image will likely over-expose. Exp:"+toString(ac->ap.exp)+" Gain"+toString(ac->ap.exp),DEBUGANDOR897);
		}
	}
	c=*ac;
	this->getImageRegion(ac->ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	int x=cont.stg->getX();
	int y=cont.stg->getY();
	double z=cont.focus->getZ();
	double temperature=cont.te.getTemp();
	double temperature2=cont.te.getTemp2();
	string time=Timer::getSysTime();
	this->takePicture(ac->ap.exp,ac->ap.gain,ac->ap.bin);

	ac->on(true);
	this->trigger();
	Timer::wait(ac->ap.exp*1000);
	ac->off(true);
	this->waitIdle();
	CImg<unsigned short>* pic=getNewPic();
	if (pic->max()>ANDOR897MAXPIXELVAL*0.95) logFile.write("Camera: Warning. Picture taken was overexposed or close to saturation. Please adjust settings",DEBUGANDOR897);
	max=pic->max();
	if (fileName.at(0)!='!') {
		this->saveTiff(pic,ImageProperties(ac,fileName,"called through camera takePicture method",x,y,z,temperature,temperature2,time,pic->max()));
		
	}else {
		getScore(pic,&ret);
		if (ac->showOnScreen) Andor897::showImage(pic,fileName+string(" Score=")+::toString(ret));
	}	
	delete pic;
	return ret;
}


void Andor897::adjustIntensity(AcquisitionChannel& ac,double percentSaturation){
	CheckExists()
	bool redo=false;
	unsigned short max;
	this->takePicture(ac.ap.exp,ac.ap.gain,ac.ap.bin);
	ac.on(true);
	this->trigger();
	this->waitIdle();
	ac.off();
	CImg<unsigned short>* pic=getNewPic();
	max=pic->max();
	if (max>ANDOR897MAXPIXELVAL) {
		logFile.write("Camera: Adjustment error: Picture taken was overexposed. adjusting settings and retrying",DEBUGANDOR897);
		max=((double)ANDOR897MAXPIXELVAL)*percentSaturation/0.5;//artificially adjust max so we get half saturation
		redo=true;
	}
	delete pic;

	double newExp;
	//first try to adjust gain within reason
	int newGain=ac.ap.gain*ANDOR897MAXPIXELVAL*percentSaturation/max;
	if (newGain>32){
		newGain=32;
	}if (newGain<3){
		newGain=3;
	}
	newExp=((double)ANDOR897MAXPIXELVAL)*percentSaturation*ac.ap.exp/(((double)max)*(((double)newGain)/ac.ap.gain));
	if (newGain!=ac.ap.gain){
		logFile.write(string("Camera (AdjustIntensity) Max pixel value was:")+toString(max)+" out of "+toString(ANDOR897MAXPIXELVAL)+" Desired Percent Saturation is "+toString(percentSaturation)+" Changed gain from: " +toString(ac.ap.gain)+" to "+toString(newGain));
	}
	if (newExp!=ac.ap.exp){//most likely it was changed
		logFile.write(string("Camera (AdjustIntensity) Max pixel value was:")+toString(max)+ " out of "+toString(ANDOR897MAXPIXELVAL)+" Desired Percent Saturation is "+toString(percentSaturation)+" Changed exposure from: "+toString(ac.ap.exp)+" to "+toString(newExp));
	}
	ac.ap.exp=newExp;
	ac.ap.setGain(newGain);
	if (redo) adjustIntensity(ac,percentSaturation);
}
void Andor897::startLiveView(LiveViewData lvd){
	CheckExists()
	if (live)
		stopLiveView();
	unsigned threadID;
	_isLiveView=true;
	live=(HANDLE) _beginthreadex(NULL,0,LiveThread,new LiveViewArg(this,lvd),0,&threadID);
}

void Andor897::waitLiveViewStart(){
	CheckExists()
	if (WAIT_TIMEOUT==WaitForSingleObject(liveViewStart,INFINITE)){
		logFile.write("Error: wait for live view to start timed out",true);
	}
}

void Andor897::saveLiveImage(string fileName){
	this->fileName=fileName;
	this->takepic=true;
}

void Andor897::stopLiveView(){
	CheckExists()
	if (!live)
		return;
	_isLiveView=false;
	//liveDisp.close();
	WaitForSingleObject(live,INFINITE);
	CloseHandle(live);
	live=NULL;
}

bool Andor897::isLiveView(){
	return this->_isLiveView;
//	if (this->liveDisp.is_closed())
//		return false;
//	return true;
}

double Andor897::getImageWidth(AcquisitionParameters& ap){
	int minX,minY,maxX,maxY;
	getImageRegion(ap,minX,minY,maxX,maxY);
	return (maxX-minX+1)*getPixelSize();
}

double Andor897::getImageHeight(AcquisitionParameters& ap){
	int minX,minY,maxX,maxY; 
	getImageRegion(ap,minX,minY,maxX,maxY);
	return (maxY-minY+1)*getPixelSize();
}

void Andor897::wait(){
	this->waitIdle();
}

bool Andor897::isValid(AcquisitionParameters &ap){
	return true;
}

double Andor897::getTemp(){
	return this->getCoolTemp();
}

unsigned __stdcall Andor897::dat2Tiff(void* param){
	Series* s=(Series *) param;//this was allocated with new so we need to delete it when done
	int camNum=s->camNum;
	
	ostringstream oldFile;
	string fileName, descr;
	int numXpixels,numYpixels;
	ImageProperties ip;
	DWORD bytesRead;
	char pos[256];
	HANDLE spool;
	s->a->getNumPixels(s->scan->acquisitionSeries[s->seriesNum%s->scan->acquisitionSeries.size()]->acquisitionGroups[0]->acquisitionChannels[0].ap, numXpixels,numYpixels);
	CImg<unsigned short> * image=new CImg<unsigned short>(numXpixels,numYpixels);
	Timer swatch;
	//NOTE: FIRST IMAGE FROM ANDOR KINETICS SERIES WITH EXTERNAL TRIGGERING IS A DUMMY IMAGE
	for(int i=1;i<s->numImages+1;i++){
		oldFile.str("");
		//nextFile.str("");
		sprintf(pos,"%0.10d",i);
		oldFile <<s->tempDir<<pos<<".dat"<<flush;		
		swatch.resetTimer();
		swatch.startTimer();
		while(true){//wait for file to exist
			if (WAIT_OBJECT_0==WaitForSingleObject(s->a->getAbortEvent(s->scan),0)){
				Andor897SelCam(AbortAcquisition(),s->a);
				return 0;//abortEvent signaled
			}
			spool=CreateFile(oldFile.str().c_str(), GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
			if (spool!=INVALID_HANDLE_VALUE) break;
			if (swatch.getTime()>SAVEFILETIMEOUT) {
				logFile.write(string("Error: Could not open file: ")+oldFile.str()+ " Camera did not spool to disk. Check trigger cables/settings.  Aborting Acquisition",true);
				Andor897SelCam(AbortAcquisition(),s->a)
				clickAbort();
				return 0;
			}
		}
		if (!(s->scan->getImageProperties(i-1,s->seriesNum,ip))){//dummy image
			CloseHandle(spool);
			//remove(oldFile.str().c_str());//do this at the end   
			continue;
		}
		ReadFile(spool,image->data(), numXpixels*numYpixels*sizeof(unsigned short), &bytesRead,NULL);
		if (camNum!=-1)//multi-cam mode, lets append the camera number and channel name
			ip.name=ip.name+"Cam"+::toString(camNum)+ip.ac->chan->toString(ip.ac->out,camNum);
		saveTiff(image,ip);
		//saveTiff(oldFile.str(),image,""); //saves all pictures and dummies in order without naming them.
		CloseHandle(spool);
		//remove(oldFile.str().c_str());//do this at the end   
	}
	//all files have been saved. now we must clean up this temp directory
	string tempdir=s->tempDir.substr(0,s->tempDir.size()-1);
	if (!s->a->isIdle())
		logFile.write("Error: Andor is not done taking pictures",true);
	DeleteDirectory(tempdir.c_str(),true);
	delete s;//this should be correct, doesnt need to be used anymore
	return 0;
}

Andor897::Andor897(Trigger* t,int triggerLine,int deviceNum,int orientation):
Camera(t,vector<int>(1,triggerLine),orientation),
deviceNum(deviceNum),
isPresent(true),
focusing(false),
temp(-80),
freq(0),
takepic(false),
first(true),
//liveDisp(),
focus(NULL),
live(NULL),
liveViewStart(NULL),
_isLiveView(false)
{
	unsigned threadID;
	setup=(HANDLE) _beginthreadex(NULL, 0, &setupThread, this, 0, &threadID);
}

void Andor897::waitSetup(){
	WaitForSingleObject(setup,INFINITE);
}

unsigned __stdcall Andor897::setupThread(void* param){
	Andor897* a=(Andor897*) param;
	a->liveViewStart=CreateEvent(NULL,TRUE,FALSE,NULL);
	/*cout<<"size of unsigned long long is: "<<sizeof(ULONGLONG)<<" bytes"<<endl;
	cout<<"size of unsigned long is: "<<sizeof(ULONG)<<" bytes"<<endl;
	cout<<"size of unsigned int is: "<<sizeof(UINT)<<" bytes"<<endl;
	cout<<"size of float is: "<<sizeof(float)<<" bytes"<<endl;
	cout<<"size of double is: "<<sizeof(double)<<" bytes"<<endl;
	*/
	if (!a->t || !a->t->isValidLine(a->triggerLines.front())){
		logFile.write("Andor897 NOT PRESENT....needs valid trigger line",true);
		a->isPresent=false;
		return 0;
	}
	
	unsigned int err;
	long totalCameras;
	err=GetAvailableCameras(&totalCameras);
	if (DRV_SUCCESS!=err){//look in current directory for .ini file
		//cout<<"could not initialize Andor897 driver: "<<err_val<<endl;
		a->isPresent=false;
		logFile.write("Error: Andor Driver could not query number of cameras",true);
		logFile.write("Andor897 NOT PRESENT",true);
		return 0; 
	}

	if (a->deviceNum<0 || a->deviceNum>totalCameras){
		logFile.write("Error: Andor device number is not valid",true);
		a->isPresent=false;
		logFile.write("Andor897 NOT PRESENT",true);
		return 0;
	}


	err=::GetCameraHandle(a->deviceNum,&a->camHandle);
	if (DRV_SUCCESS!=err){//look in current directory for .ini file
		//cout<<"could not initialize Andor897 driver: "<<err_val<<endl;
		a->isPresent=false;
		cout<<"Andor897 NOT PRESENT"<<endl;
		return 0;
	}

	unsigned int err_val;
	err_val=SetCurrentCamera(a->camHandle);
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		//cout<<"could not initialize Andor897 driver: "<<err_val<<endl;
		logFile.write(string("Error: Could not set current camera. Handle is")+toString(a->camHandle),true);
		a->isPresent=false;
		logFile.write("Andor897 NOT PRESENT",true);
		return 0;
	}
	Andor897SelCam(err_val=Initialize(NULL),a);
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		//cout<<"could not initialize Andor897 driver: "<<err_val<<endl;
		a->isPresent=false;
		cout<<"Andor897 NOT PRESENT"<<endl;
		return 0;
	}
	Andor897SelCam(SetCoolerMode(1),a)//temperature is maintained on shutdown Remember to set temperature to 20C
	Andor897SelCam(err_val=GetDetector(&a->XPixels,&a->YPixels),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get detector: "<<err_val<<endl;
		a->isPresent=false;
		cout<<"Andor897 NOT PRESENT"<<endl;
		return 0;
	}
	cout<<"Andor897 ready"<<endl;
	//cout<<"XPixels: "<<XPixels<<" YPixels: "<<YPixels<<endl;
	a->bin=1;;
	a->getImageRegion(AcquisitionParameters(),a->minXPixel,a->minYPixel,a->maxXPixel,a->maxYPixel);//FULL image
	Andor897SelCam(err_val=SetShutter(0,1,0,0),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set shutter timings: "<<err_val<<endl;
		system("pause");
		exit(0);
	}
	Andor897SelCam(err_val=SetReadMode(4),a)//image acquisition mode
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set read mode to image acquisition: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(err_val=SetVSSpeed(3),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set VSSpeed: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	float speed;
	Andor897SelCam(err_val=GetVSSpeed(3,&speed),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get VSSpeed: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	//logFile.write(string("Andor Vertical Shift Speed is ")+toString(speed)+" microseconds per pixel",true);
	Andor897SelCam(err_val=SetVSAmplitude(0),a)//can be 0-4 with 4 being the highest voltage for high readout speeds
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set VS Amplitude: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(err_val=SetHSSpeed(0,0),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set HSSpeed: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(err_val=GetHSSpeed(0,0,0,&speed),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get HSSpeed: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	//cout<<"Horizontal Shift Speed is "<<speed<<" MHz"<<endl;
	int numADChan;
	Andor897SelCam(err_val=GetNumberADChannels(&numADChan),a)
	int chan=0;
	//cout<<"Number of AD Channels is: "<<numADChan<<" We selected channel "<<chan<<endl;
	
	Andor897SelCam(err_val=SetADChannel(chan),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set ADChannel: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	int numAmp;
	Andor897SelCam(err_val=GetNumberAmp(&numAmp),a)
	//cout<<"Number of Output Amp Channels is: "<<numAmp<<endl;
	int filterMode;
	Andor897SelCam(GetFilterMode(&filterMode),a)
	if (filterMode==0){
		//cout<<"cosmic ray filter is OFF"<<endl;
	}else{
		//cout<<"cosmic ray filter is ON"<<endl;
	}
	Andor897SelCam(err_val=SetAcquisitionMode(1),a)//single scan mode...5 would be run til abort
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set acquisition mode to single scan: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(err_val=SetFrameTransferMode(0),a)//frame transfer mode OFF
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set Andor897 to FT Mode: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}

	Andor897SelCam(err_val=SetTriggerMode(0),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set internal triggering mode: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(SetOutputAmplifier(0),a)//standard emmccd gain
	int noGains;
	Andor897SelCam(err_val=GetNumberPreAmpGains(&noGains),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not find number of preamp gains: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	float pgain1, pgain2,pgain0;
	Andor897SelCam(GetPreAmpGain(0,&pgain0),a)
	//cout<<"Number of PreAmp Gains: "<<noGains;
	//cout<<" Gain 0="<<pgain0<<"x";
	Andor897SelCam(GetPreAmpGain(1,&pgain1),a)
	//cout<<" Gain 1="<<pgain1<<"x";
	Andor897SelCam(GetPreAmpGain(2,&pgain2),a)
	//cout<<" Gain 2="<<pgain2<<"x"<<endl;
	//cout<<" Setting PreAmp Gain to "<<pgain0<<"x"<<endl;
	Andor897SelCam(err_val=SetPreAmpGain(2),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get set pre amp gain: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}

	a->setExposure(.1);
	//SetAccumulationCycleTime();
	//SetNumberAccumulations();
	//SetNumberKinetics();
	//SetKineticCycleTime();
	long numPics;
	Andor897SelCam(SetDMAParameters(1,.01),a)
	Andor897SelCam(err_val=GetSizeOfCircularBuffer(&numPics),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get size of circular buffer: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	//cout<<"Capacity of circular Buffer: "<<numPics<<" pictures"<<endl;

	a->newImage=CreateEvent(NULL,FALSE,FALSE,NULL);
	a->triggerReady=CreateEvent(NULL,FALSE,FALSE,NULL);
	Andor897SelCam(err_val=SetDriverEvent(a->newImage),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set driver event: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(err_val=SetBaselineClamp(1),a)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set baseline clamp: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}

	a->coolDown();
//	Andor897SelCam(SetEMAdvanced(1),this)//this allows for higher gain values but do we need it?
//	Andor897SelCam(SetEMGainMode(3),this)
//	Andor897SelCam(GetEMGainRange(&minGain,&maxGain),this)
	Andor897SelCam(SetEMAdvanced(0),a)//this limits em gain to 300 which should be fine
	Andor897SelCam(SetEMGainMode(2),a)
	Andor897SelCam(GetEMGainRange(&a->minGain,&a->maxGain),a)

	Andor897SelCam(GetBitDepth(chan, &a->bitDepth),a)
	Andor897SelCam(SetEMGainMode(2),a)//Real EM Gain is 3...note to turn EM off we need to be in mode 0 or 1 and send a DAC value of 0
	//Linear gain is 2
	//12bit gain is 1
	//8bit gain is 0
	//setExposure(.1);
	a->gain=a->minGain;//we can turn gain off by changing the em gain mode to 0 or 1 and sending a DAC value of 1;
	a->setGain(a->gain);
	//setBin(1);
	/*if (camHandle==201){
		this->takePicture(.1,1,5);
		this->trigger();
		CImg<unsigned short>* test=getNewPic();
		delete test;
		((Camera*)this)->startLiveView();
		system("pause");
		this->stopLiveView();
	}*/
	//Cleanout AndorTempFOlder
	//System::Directory::Delete(
	if (!DeleteDirectory(string(ANDOR897TEMPDIR).c_str()))
		logFile.write("Could not delete Andor temp directory",true);	
}

Andor897::~Andor897(){
	CheckExists()
	WaitForSingleObject(AndorSelectCamMutex, INFINITE);
	SetCurrentCamera(this->camHandle);
	_isLiveView=false;
	//liveDisp.close();
	closeAll();
	focusing=false;
	AbortAcquisition();
	SetDriverEvent(NULL);
	SetEvent(newImage);
	SetTemperature(20);
	CoolerOFF();
	CloseHandle(newImage);	
	SetShutter(0,2,0,0);
	ShutDown();
	ReleaseMutex(AndorSelectCamMutex);
}

void Andor897::closeAll(){
	CheckExists()
//	Disp::closeAll();
}

long Andor897::spoolProgress(){
	CheckExists(0)
	long ret;
	unsigned int err_val;
	Andor897SelCam(err_val=GetSpoolProgress(&ret),this)
		if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get spool progress: "<<err_val<<endl;
		system("pause");
		exit(0);;
	}
	return ret;
}

void Andor897::waitIdle(){
	CheckExists()
	while(!isIdle());
}

void Andor897::coolDown(){
	CheckExists()
	int minT,maxT;
	Andor897SelCam(GetTemperatureRange(&minT,&maxT),this)
	//cout<<"Min Temp for cooling is: "<<minT<<" Max Temp for cooling is: "<<maxT<<endl;
	if (temp<minT || temp>maxT){
		cout<<"error Andor897 cooling temperature is out of range"<<endl;
		system("pause");
		exit(0);;
	}
	Andor897SelCam(SetTemperature((int)temp),this)
	Andor897SelCam(CoolerON(),this)
	//while(GetTemperature(&temp)!=DRV_TEMP_STABILIZED){}
	//cout<< "Andor897 temperature has reached "<<temp<<" degrees Celsius"<<endl;
}

void Andor897::heatUp(){
	CheckExists()
	Andor897SelCam(SetTemperature(20),this)//Actively return to ambient
	Andor897SelCam(CoolerOFF(),this)//not sure if this call is necessary
	unsigned int err_val;
	Andor897SelCam(err_val=GetTemperatureF(&temp),this)
	if (err_val!=DRV_TEMP_OFF)
	logFile.write(string("Error: Andor897 could not get camera temperature: ")+toString(int(err_val)),true);
	logFile.write(string("Current temp is ")+toString(temp)+" degree C. Let camera heat back up to ambient.",DEBUGANDOR897);
}

float Andor897::getCoolTemp(){
	CheckExists(0)
	waitIdle();
	float target, ambient, coolvolt;
	unsigned int err_val;
	Andor897SelCam(err_val=GetTemperatureStatus(&temp, &target, &ambient, &coolvolt),this)
	if (err_val!=DRV_SUCCESS)
		cout<<"Error getting temperature: "<<err_val<<endl;
	cout << "Sensor Temperature= " << temp << ", target temperature= " << target 
		 << ", ambient temperature= " << ambient << ", cooler voltage= " << coolvolt <<endl;
	return temp;
}

void Andor897::waitTemp(){
	CheckExists()
	cout<<"Waiting for Andor897 to cool down to "<<this->temp<<" deg C..."<<endl;
	int temp;
	Timer t;
	t.startTimer();
	int i=0;
	unsigned int b;
	while(true){
		Andor897SelCam(b=GetTemperature(&temp),this)
		if (b==DRV_TEMP_STABILIZED)
			break;
		if (t.getTime()>i*1*60*1000){
			cout<<"After "<<i*1<<"min";
			getCoolTemp();
			i++;
		}
	}
	cout<<"...Andor897 Temperature Reached!"<<endl;
}

long Andor897::totalNum(){
	CheckExists(0)
	long ret;
	Andor897SelCam(GetTotalNumberImagesAcquired(&ret),this)
	return ret;
}

int Andor897::getBitDepth(){
	return bitDepth;
}

float Andor897::getPixelSize(){
	CheckExists(0)
	static bool once=false;
	float y;
	static float x;
	if (once) return x;
	Andor897SelCam(GetPixelSize(&x,&y),this)
	once=true;
	assert(x==y);
	return x;
}
void Andor897::saveTiff(string fileName, CImg<unsigned short>* c,string descr, bool asEightBit){
	fileName+=".tif";
	if (asEightBit){
		c->cut(0,ANDOR897MAXPIXELVAL);
		c->operator>>=(ANDOR897BITS-8*sizeof(unsigned char));//multiply by 255/16383
		CImg<unsigned char> temp(*c);
		temp.save(fileName.c_str());

	}else c->save(fileName.c_str());
	
	//NOTE: if the image has no pixel value over 255 then it will save it as 8 bit tif//unless we are using libtiff which we are
	TIFF* tif=TIFFOpen(fileName.c_str(), "a");
	TIFFSetDirectory(tif,0);
	//TIFFSetField(tif,TIFFTAG_IMAGEDESCRIPTION,ip.comment);
	TIFFSetField(tif,TIFFTAG_SOFTWARE,descr.c_str());//so we can see it in windows properties dialog under CREATION SOFTWARE
	TIFFRewriteDirectory(tif);
	TIFFClose(tif);

}

void Andor897::saveTiff(CImg<unsigned short>* c,ImageProperties& ip){
	string fileName=ip.name+".tif";
	if (ip.ac->ap.asEightBit){
		c->cut(0,ANDOR897MAXPIXELVAL);
		c->operator>>=(ANDOR897BITS-8*sizeof(unsigned char));//multiply by 255/16383
		CImg<unsigned char> temp(*c);
		temp.save(fileName.c_str());

	}else c->save(fileName.c_str());
	
	if (ip.ac->showOnScreen) Andor897::showImage(c,ip.name+string("MaxVal=")+::toString((int)ip.max)+"/"+::toString(ANDOR897MAXPIXELVAL)+" "+ip.ac->toString());

	TIFF* tif=TIFFOpen(fileName.c_str(), "a");
	TIFFSetDirectory(tif,0);
	//TIFFSetField(tif,TIFFTAG_IMAGEDESCRIPTION,ip.comment);
	TIFFSetField(tif,TIFFTAG_SOFTWARE,"Eric Roller (UCSD) and CImg Library");//so we can see it in windows properties dialog under CREATION SOFTWARE
	TIFFRewriteDirectory(tif);
	TIFFClose(tif);
	logFile.write(fileName+" saved",DEBUGANDOR897);
}

void Andor897::showFile(string fileName){
	CImg<unsigned short>* Cimage=new CImg<unsigned short>(fileName.c_str());
	Arg* a= new Arg();
	ostringstream myStream;
	myStream.str("File Image");
	//myStream<<"Exposure:"<<exp<<" Gain:"<<gain<<" Binning:"<<bin<<flush;
	string final=fileName;
	CImg<unsigned short>* copyCimage=Cimage;
	CImgDisplay *disp=new CImgDisplay((const CImg<unsigned short>)(*copyCimage),final.c_str(),1,false,false);
//	Disp::addDisp(disp);
	a->disp=disp;
	a->img=copyCimage;
	//a->title=final;
	unsigned threadID;
	HANDLE thread=(HANDLE) _beginthreadex(NULL,0,
				   Andor897::ImageThread,
				   a,
				   0,
				   &threadID);
	CloseHandle(thread);
	
}

void Andor897::showImage(CImg<unsigned short>* Cimage, string title){
//	CheckExists()
	Arg* a= new Arg();
//	ostringstream myStream;
//	//myStream.str("");
//	myStream<<"Exposure:"<<exp<<" Gain:"<<gain<<" Binning:"<<bin<<flush;
	string final=title;//+"......"+myStream.str();
	CImg<unsigned short>* copyCimage=new CImg<unsigned short>(*(Cimage));
	CImgDisplay *disp=new CImgDisplay((const CImg<unsigned short>)(*copyCimage),final.c_str(),1,false,false);
//	Disp::addDisp(disp);
	a->disp=disp;
	a->img=copyCimage;
	//a->title=final;
	unsigned threadID;
	HANDLE thread=(HANDLE) _beginthreadex(NULL,0,
				   Andor897::ImageThread,
				   a,
				   0,
				   &threadID);
	CloseHandle(thread);
}

unsigned __stdcall Andor897::ButtonWaitThread(void *param){
	Button* b=(Button*) param;
	while(!b->d->is_closed()){
		b->d->wait();
		//WaitForSingleObject(cimg::Win32attr().wait_event,INFINITE);
		if (b->d->button())
			SetEvent(b->h); 
	}
	return 0;
}

void Andor897::testMultipleCameras(){
	SetCurrentCamera(camHandle);
	ResetEvent(newImage);
	SetTriggerMode(0);//internal trigger
	SetAcquisitionMode(5);//run til abort
	SetReadMode(4);
	SetKineticCycleTime(0);
	SetFrameTransferMode(1);
	SetImage(bin,bin,minXPixel+1,maxXPixel+1,minYPixel+1,maxYPixel+1);
	SetExposureTime(.1);
	SetEMCCDGain(3);
	StartAcquisition();
}

void Andor897::testStop(){
	SetCurrentCamera(camHandle);
	AbortAcquisition();
}

CImg<unsigned short>* Andor897::testGetPic(){
	CImg<unsigned short> *copyImage=new CImg<unsigned short>((maxXPixel-minXPixel+1)/bin,(maxYPixel-minYPixel+1)/bin);
	CheckExists(copyImage)
	
	WaitForSingleObject(newImage,INFINITE);
	unsigned int err_val;
	err_val=SetCurrentCamera(camHandle);
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get 16bit image: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	err_val=GetMostRecentImage16(copyImage->data(),(maxXPixel-minXPixel+1)*(maxYPixel-minYPixel+1)/bin/bin);
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get 16bit image: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	return copyImage;
}

unsigned __stdcall Andor897::LiveThread(void *param){
	LiveViewArg* lva=(LiveViewArg*) param;//this was allocated with new so we need to delete it when done
	Andor897* c=(Andor897*) lva->a;
	if (!c->isPresent) return 0;
	unsigned threadID;
	unsigned int err_val;
	c->waitIdle();

	ResetEvent(c->newImage);
	Andor897SelCam(SetTriggerMode(0),c)//internal trigger
	Andor897SelCam(err_val=SetAcquisitionMode(5),c)//run til abort
	Andor897SelCam(SetReadMode(4),c)
	Andor897SelCam(SetKineticCycleTime(0),c)
	Andor897SelCam(SetFrameTransferMode(1),c)
	float desiredExp=cont.currentChannel().ap.exp;
	c->getImageRegion(cont.currentChannel().ap,c->minXPixel,c->minYPixel,c->maxXPixel,c->maxYPixel);
	c->setBin(cont.currentChannel().ap.bin);
	c->setExposure(cont.currentChannel().ap.exp);
	int gain=cont.currentChannel().ap.getGain(lva->lvd.camNum);
	c->setGain(gain);
	Andor897SelCam(err_val=SetSpool(0,2,"",10),c)
	float kin,acc;
	//Timer::wait(100); BUG in ANDOR DRIVER THIS MAY BE REQUIRED
	Andor897SelCam(GetAcquisitionTimings(&(c->exp),&acc,&kin),c)
	if (c->exp>desiredExp) {
		Andor897SelCam(SetFrameTransferMode(0),c)
		c->setExposure(desiredExp);
	}

	//Andor897SelCam(FreeInternalMemory(),c) not really necessary?
	//Andor897SelCam(PrepareAcquisition(),c) not really necessary?
	
	
	int numXpixels=(c->maxXPixel-c->minXPixel+1)/c->bin;
	int numYpixels=(c->maxYPixel-c->minYPixel+1)/c->bin;
	CImg<unsigned short> copyImage(numXpixels,numYpixels);
	CImg<unsigned short> copyImageDisp;//=new CImg<unsigned short>(numXpixels,numYpixels);
	CImg<unsigned short> displayImage(lva->lvd.width,lva->lvd.height);//=new CImg<unsigned short>(numXpixels,numYpixels);
	
	CImgDisplay testDisp=CImgDisplay(displayImage,"",0,false,true);

	HWND hwndFound=testDisp._window;//FindWindow(NULL,liveDisp.title());	
	RECT area;
	GetWindowRect(hwndFound,&area);
	displayImage=CImg<unsigned short>(2*lva->lvd.width-(area.right-area.left),2*lva->lvd.height-(area.bottom-area.top));
	CImgDisplay liveDisp=CImgDisplay(displayImage,string(string("Live View ")+cont.currentChannel().chan->toString(cont.currentChannel().out,lva->lvd.camNum)).c_str(),0,false,true);	
	hwndFound=liveDisp._window;//FindWindow(NULL,liveDisp.title());
	GetWindowRect(hwndFound,&area);
	MoveWindow(hwndFound,lva->lvd.x,lva->lvd.y,area.right-area.left,area.bottom-area.top,true);
	SetEvent(c->liveViewStart);
	//liveDisp.show();
		
	*lva->lvd.zoom=getZoom(*lva->lvd.scaleFactor);;//total zoom for current view;
	//*lva->lvd->centerX;//centerX of image about which to zoom (from 0 to 1)
	//c->centerY;//centerY of image about which to zoom (from 0 to 1)
	getCenter(lva->lvd);
	DWORD num=2;
	HANDLE lpHandles[2];
	lpHandles[1]=c->newImage;
	int stat;
	Andor897SelCam(err_val=StartAcquisition(),c)
	//Timer::wait(5000);
	Andor897SelCam(GetStatus(&stat),c)
	if (stat!=DRV_ACQUIRING){
		logFile.write(string("Error: live acquisition not running, status is ")+toString(stat),true);
		system("pause");
		return 0;
	}
	WaitForSingleObject(lpHandles[1],INFINITE);
	Andor897SelCam(err_val=GetMostRecentImage16(copyImage.data(),numXpixels*numYpixels),c)
	int beforeX,afterX,beforeY,afterY;
	copyImageDisp=getAdjustedImage(copyImage,*lva->lvd.zoom,*lva->lvd.centerXEff,*lva->lvd.centerYEff,liveDisp.width(),liveDisp.height(),beforeX,afterX,beforeY,afterY);
	displayImage=getFinalImage(*lva,copyImageDisp,*lva->lvd.zoom);
	liveDisp.display(displayImage);
	liveDisp.show();
	
	
	
	lpHandles[0]=CreateEvent(NULL,false,false,NULL);//cimg::Win32attr().wait_event;//liveDisp.wait_disp;
	Button* b=new Button();b->d=&liveDisp;b->h=lpHandles[0];
	c->buttonWait=(HANDLE) _beginthreadex(NULL,0,ButtonWaitThread,b,0,&threadID);

	CImg<int> selection;
	


	while(c->_isLiveView){
		DWORD ret=WaitForMultipleObjects(num,lpHandles,false,INFINITE);
		if (ret==WAIT_OBJECT_0+1){//new image available
			Andor897SelCam(err_val=GetMostRecentImage16(copyImage.data(),numXpixels*numYpixels),c)
			if (c->orientation==1){//mirror image in X direction
				copyImage.transpose();
				copyImage.rotate(90);
			}
			copyImageDisp=getAdjustedImage(copyImage,*lva->lvd.zoom,*lva->lvd.centerXEff,*lva->lvd.centerYEff,liveDisp.width(),liveDisp.height(),beforeX,afterX,beforeY,afterY);
			displayImage=getFinalImage(*lva,copyImageDisp,*lva->lvd.zoom);
			liveDisp.display(displayImage);

			
			if (c->takepic){
				Andor897::saveTiff(c->fileName,&copyImage);
				Andor897::saveTiff(c->fileName+"scaleBar",&displayImage);
				c->takepic=false;
			}
			//ResetEvent(c->newImage); automatically reset
		}
		//ret=WaitForSingleObject(lpHandles[0],30);

		else if (ret==WAIT_OBJECT_0){//button clicked	
			if (liveDisp.button()&2){
				if (lva->lvd.scaleFactor->size()>0) {
					lva->lvd.scaleFactor->pop_back();
					lva->lvd.centerX->pop_back();
					lva->lvd.centerY->pop_back();
					*lva->lvd.zoom=getZoom(*(lva->lvd.scaleFactor));
					getCenter(lva->lvd);
				}
				while(liveDisp.button()&2) ;
				::showConsole();
			}else if (liveDisp.button()&1){
				selection=displayImage.select(liveDisp,2,0);
				::showConsole();
				if (lva->lvd.scaleFactor->size()<5 && selection[0]!=selection[3] && selection[1]!=selection[4]) {
					//make sure selection includes part of the image
					if (//x0 or x1 is in image
						(((selection[0]<liveDisp.width()/2.0+copyImageDisp.width()/2.0) && (selection[0]>liveDisp.width()/2.0-copyImageDisp.width()/2.0)) ||   //x0 is in image
						((selection[3]<liveDisp.width()/2.0+copyImageDisp.width()/2.0) && (selection[3]>liveDisp.width()/2.0-copyImageDisp.width()/2.0))) 	//x1 is in image
						&& //y0 or y1 is in image
						(((selection[1]<liveDisp.height()/2.0+copyImageDisp.height()/2.0) && (selection[1]>liveDisp.height()/2.0-copyImageDisp.height()/2.0)) ||   //x0 is in image
						((selection[4]<liveDisp.height()/2.0+copyImageDisp.height()/2.0) && (selection[4]>liveDisp.height()/2.0-copyImageDisp.height()/2.0)))) {
					
						//level++;
						int x1=selection[0];
						int x2=selection[3];
						int y1=selection[1];
						int y2=selection[4];
						double centerXtemp,centerYtemp;
						if (double(liveDisp.width())/copyImage.width()>double(liveDisp.height())/copyImage.height()){
							//zooming will fit to height
							centerXtemp=(selection[0]+selection[3])/2.0/liveDisp.width()-.5;
							centerXtemp=centerXtemp*liveDisp.width()*copyImage.height()/liveDisp.height()/copyImageDisp.width();
							centerYtemp=(selection[1]+selection[4])/2.0/liveDisp.height()-.5;
						}else{
							//zooming will fit to width
							centerYtemp=(selection[1]+selection[4])/2.0/liveDisp.height()-.5;
							centerYtemp=centerYtemp*liveDisp.height()*copyImage.width()/liveDisp.width()/copyImage.height();
							centerXtemp=(selection[0]+selection[3])/2.0/liveDisp.width()-.5;
						}
						double scaleX=abs(double(liveDisp.width())/(selection[0]-selection[3]));
						double scaleY=abs(double(liveDisp.height())/(selection[1]-selection[4]));
						lva->lvd.scaleFactor->push_back(min(scaleX,scaleY));
						lva->lvd.centerX->push_back(centerXtemp);
						lva->lvd.centerY->push_back(centerYtemp);
						*lva->lvd.zoom=getZoom(*(lva->lvd.scaleFactor));
						getCenter(lva->lvd);
					}
				}
			}
		}
		
	}
	Andor897SelCam(err_val=AbortAcquisition(),c)
	Andor897SelCam(err_val=SetAcquisitionMode(1),c)//single scan
	ResetEvent(c->liveViewStart);
	TerminateThread(c->buttonWait,0);
	
	WaitForSingleObject(c->buttonWait,INFINITE);
	CloseHandle(lpHandles[0]);
	CloseHandle(c->buttonWait);
	c->buttonWait=NULL;
	ResetEvent(c->newImage);
	delete lva;
	return 0;
}




//obsolete replaced by ImageThread
unsigned __stdcall Andor897::FileThread(void *param){
	string* fileName=(string*) param;
	CImg<> newImage(fileName->c_str());
	newImage.display(fileName->c_str());
	//newImage.~CImg();
	delete fileName;
	return 0;
}

unsigned __stdcall Andor897::ImageThread(void *param){

	Arg* a=(Arg*) param;
	//c->display();
	//a->img->display("testing",128,1344);
	//check the display to see if it is closed every 10sec
	//HANDLE thread;
	//DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&thread,0,true,DUPLICATE_SAME_ACCESS);
	DisplayList::Display* d=cont.displays.addDisplay(a->disp);
	while(!a->disp->is_closed()){
		a->disp->wait();
	}
	cont.displays.removeDisp(d);
	delete a->disp;
	delete a->img;
	delete a;
	return 0;
}


void Andor897::setExposure(float newExp){
	CheckExists()
	waitIdle();
	this->exp=newExp;
	unsigned int err_val;
	Andor897SelCam(err_val=SetExposureTime(this->exp),this)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set exposure time of" <<newExp<<" : "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);
	}
}

float Andor897::getExposure(){
	CheckExists(0)
	return exp;
}

int Andor897::getGain(){
	CheckExists(0)
	return gain;
}


void Andor897::setGain(int& newGain){
	CheckExists()
	waitIdle();
	//cout << "Gain must be between " << low << " and " << high << endl;
	if (newGain<minGain){
		if (newGain!=0)
			logFile.write(string("Error: gain value of ")+::toString(newGain)+" is too small. em gain will be turned off (i.e. gain=0)",true);// set to "+::toString(minGain)+" until we learn how to turn EM gain off");
		Andor897SelCam(SetEMGainMode(0),this)
		Andor897SelCam(SetEMCCDGain(0),this)//gain=minGain;
		newGain=0;//this should update the current channel if we have access to it
	}else {
		if (gain==0)
			Andor897SelCam(SetEMGainMode(2),this)
		if (newGain>maxGain){	
			logFile.write(string("Error: gain value of ")+::toString(gain)+" is too large. setting to "+::toString(maxGain));
			newGain=maxGain;//this should update the current channel if we have access to it
		}
		Andor897SelCam(SetEMCCDGain(newGain),this)
	}
	gain=newGain;
}

int Andor897::getBin(){
	CheckExists(0)
	return bin;
}

void Andor897::setBin(int newBin){
	CheckExists()
	waitIdle();
	/*setImageRegion must be called prior to this function (implementation of the abstract functions will enforce this)
	if (newBin<bin){
		maxXPixel=1004;
		maxYPixel=1002;
		minXPixel=1;
		minYPixel=1;
	}
	*/
	this->bin=newBin;
	int rX=(maxXPixel-minXPixel+1) % bin;
	int rY=(maxYPixel-minYPixel+1) % bin;
	while(rX>0){
		if ((rX % 2)==0) {
			minXPixel++;
		}else{
			maxXPixel--;
		}
		rX--;
	}
	while(rY>0){
		if ((rY % 2)==0) {
			minYPixel++;
		}else{
			maxYPixel--;
		}
		rY--;
	}
	unsigned int err_val;
	Andor897SelCam(err_val=SetImage(bin,bin,minXPixel+1,maxXPixel+1,minYPixel+1,maxYPixel+1),this)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not set image region/size: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
}


void Andor897::getNumPixels(const AcquisitionParameters& ap,int& numXpixels,int& numYpixels){
	CheckExists()
	int minX,minY,maxX,maxY;
	getImageRegion(ap,minX,minY,maxX,maxY);
	numXpixels=maxX-minX+1;
	numYpixels=maxY-minY+1;
	return;
}

void Andor897::getImageRegion(const AcquisitionParameters& ap,int& minX,int& minY,int& maxX,int& maxY){
	CheckExists()
	//actually xPixel=round(ap.imageRegion.maxX*XPixels-.5) but needs to round towards zero to avoid -1 or XPixels which are out of bounds!
	//without assuming a rounding convention can implement it like this without danger of exceeding the bounds
	maxX=round(XPixels*ap.imageRegion.maxX-.5);
	maxY=round(YPixels*ap.imageRegion.maxY-.5);
	minX=round(XPixels*ap.imageRegion.minX-.5);
	minY=round(YPixels*ap.imageRegion.minY-.5);
	if (ap.imageRegion.maxX==1)
		maxX=XPixels-1;
	if (ap.imageRegion.maxY==1)
		maxY=YPixels-1;
	if (ap.imageRegion.minX==0)
		minX=0;
	if (ap.imageRegion.minY==0)
		minY=0;


	int rX=(maxX-minX+1) % ap.bin;
	int rY=(maxY-minY+1) % ap.bin;

	if (minX-(rX/2.0+0.5)<1 || maxX+(rX/2)>XPixels){//we must expand in
		minX+=(int)(rX/2.0+0.5);
		maxX-=rX/2;
		if (minX>maxX) {
			logFile.write("Acquisition Parameters are not compatible. Reduce binning or increase image region.",true," Reverting to full image acquisition with no Binning.");
			minX=1;maxX=XPixels;minY=1;maxY=YPixels;
			return;
		}
	}else{
		minX-=(int)(rX/2.0+0.5);
		maxX+=rX/2;
	}

	if (minY-(rY/2.0+0.5)<1 || maxY+(rY/2)>YPixels){//we must expand in
		minY+=(int)(rY/2.0+0.5);
		maxY-=rY/2;
		if (minX>maxX){
			logFile.write("Acquisition Parameters are not compatible. Reduce binning or increase image region.",true," Reverting to full image acquisition with no Binning.");
			minX=1;maxX=XPixels;minY=1;maxY=YPixels;
			return;
		}
		}else{
		minY-=(int)(rY/2.0+0.5);
		maxY+=rY/2;
	}
}

/*//0 will be full image,1=quadrant1, 2=quadrant2, 3=quadrant3,4=quadrant4, 5=middle quadrant
void Andor897::setImageRegion(int opt, int percentX, int percentY){
	CheckExists()
	int tminXPixel,tminYPixel,tmaxXPixel,tmaxYPixel;
	int ANDORHACK=1;//this min X pixel to avoid nasty stuff in image  could just be the loaner Andor897
	switch(opt){
		case 0:
			tminXPixel=ANDORHACK;
			tminYPixel=1;
			tmaxXPixel=XPixels;
			tmaxYPixel=YPixels;
			break;
		case 1:
			tminXPixel=XPixels/2;
			tminYPixel=1;
			tmaxXPixel=XPixels;
			tmaxYPixel=YPixels/2;
			break;
		case 2:
			tminXPixel=ANDORHACK;
			tminYPixel=1;
			tmaxXPixel=XPixels/2;
			tmaxYPixel=YPixels/2;
			break;
		case 3:
			tminXPixel=ANDORHACK;
			tminYPixel=YPixels/2;
			tmaxXPixel=XPixels/2;
			tmaxYPixel=YPixels;
			break;
		case 4:
			tminXPixel=XPixels/2;
			tminYPixel=YPixels/2;
			tmaxXPixel=XPixels;
			tmaxYPixel=YPixels;
			break;
		case 5:
			tminXPixel=XPixels/4;
			tminYPixel=YPixels/4;
			tmaxXPixel=XPixels*3/4;
			tmaxYPixel=YPixels*3/4;
			break;
		default:
			cout<<"setImageRegion Error: "<<opt<<" is not an option. Defaulting to full image"<<endl;
			setImageRegion(0);
			return;
	}
	int trangeX=tmaxXPixel-tminXPixel;
	int trangeY=tmaxYPixel-tminYPixel;
	maxXPixel=0.5*(trangeX*(percentX/100.0)+tmaxXPixel+tminXPixel);
	maxYPixel=0.5*(trangeY*(percentY/100.0)+tmaxYPixel+tminYPixel);
	minXPixel=0.5*(tmaxXPixel+tminXPixel-trangeX*(percentX/100.0));
	minYPixel=0.5*(tmaxYPixel+tminYPixel-trangeY*(percentY/100.0));
	
	cout<<"MinX: "<<minXPixel;
		cout<<" MaxX: "<<maxXPixel<<endl;
		cout<<"MinY: "<<minYPixel;
		cout<<" MaxY: "<<maxYPixel<<endl;
	
	setBin(bin);
}
*/
//TODO: check that the image size is correct given the binning value
void Andor897::takePicture(float newExp, int newGain, int newBin){
	CheckExists()
	waitIdle();
	if (newExp!=-1) setExposure(newExp);
	if (newBin!=-1) setBin(newBin);
	if (newGain!=-1) setGain(newGain);
	ResetEvent(this->newImage);
	if (newExp<0.35){
	Andor897SelCam(SetFrameTransferMode(0),this)//non-frame transfer mode
	}else {
		Andor897SelCam(SetFrameTransferMode(1),this)
	}
	Andor897SelCam(SetAcquisitionMode(3),this)//kinetics
	Andor897SelCam(SetTriggerMode(6),this)//external start trigger
	Andor897SelCam(SetNumberKinetics(1),this)//single snap
	Andor897SelCam(SetKineticCycleTime(0),this)
	float acc,kin;
	Andor897SelCam(GetAcquisitionTimings(&exp,&acc,&kin),this)
	logFile.write(string("Snap. Actual exposure: ")+::toString(exp));
	Andor897SelCam(PrepareAcquisition(),this)
	Andor897SelCam(StartAcquisition(),this)
	Timer::wait(200);
	//waitIdle();//we should not need to do anything during this period
	
}

bool Andor897::isIdle(){
	CheckExists(true)
	int stat;
	Andor897SelCam(GetStatus(&stat),this)
	if (stat==DRV_IDLE) return true;
	return false;
}

//get newest picture from circular frame buffer
//binning or subpixels must be the same from when the image was taken
LONGLONG Andor897::Kinetics(){
	CheckExists(0)
	LARGE_INTEGER realstart,start,end,freq;
	QueryPerformanceFrequency(&freq);
	Andor897SelCam(SetTriggerMode(6),this)
	Andor897SelCam(SetAcquisitionMode(3),this)//kinetics series
	Andor897SelCam(SetNumberKinetics(100),this)//4 pictures
	Andor897SelCam(SetExposureTime(.1),this)//exposure time =20us
	Andor897SelCam(SetKineticCycleTime(0),this)//40ms cycle time
	Andor897SelCam(SetFrameTransferMode(1),this)//non-frame-transfer mode
	float Camexp,acc,kin;
	Andor897SelCam(GetAcquisitionTimings(&Camexp,&acc,&kin),this)
	cout<<"Actual exposure is "<<Camexp<<" Kinetic Cycle is "<<kin<<endl;
	QueryPerformanceCounter(&start);
	Andor897SelCam(PrepareAcquisition(),this)
	QueryPerformanceCounter(&end);
	cout<<"Prepare Acquisition took "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds to return"<<endl;
	realstart=start;
	QueryPerformanceCounter(&start);
	unsigned int err_val;
	Andor897SelCam(err_val=StartAcquisition(),this)
	QueryPerformanceCounter(&end);
	cout<<"Start Acquisition took "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds to return"<<endl;
	start=end;
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not start acquisition: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	/*int stat;
	while(true){
		GetStatus(&stat);
		if (stat==DRV_IDLE) break;
	}*/

	WaitForSingleObject(this->newImage,INFINITE);
	QueryPerformanceCounter(&end);
	cout<<"time to first image: "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds"<<endl;
	start=end;
	WaitForSingleObject(this->newImage,INFINITE);
	QueryPerformanceCounter(&end);
	cout<<"time between first and second image: "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds"<<endl;
	start=end;
	WaitForSingleObject(this->newImage,INFINITE);
	QueryPerformanceCounter(&end);
	cout<<"time between second and third image: "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds"<<endl;
	start=end;
	WaitForSingleObject(this->newImage,INFINITE);
	QueryPerformanceCounter(&end);
	cout<<"time between third and fourth image: "<<((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<<" seconds"<<endl;
	int stat;
	Andor897SelCam(GetStatus(&stat),this)
	if (stat!=DRV_IDLE) cout<<"driver not finished...why not?"<<endl;
	Andor897SelCam(SetAcquisitionMode(1),this)
	return end.QuadPart-realstart.QuadPart;
}

bool Andor897::isTriggerable(AcquisitionParameters &ap){
	CheckExists(true)
	unsigned int err_val;
	if (!isValid(ap)) return false;
	this->bin=ap.bin;
	this->getImageRegion(ap,minXPixel,minYPixel,maxXPixel,maxYPixel);
	Andor897SelCam(err_val=SetTriggerMode(1),this)//external trigger
	Andor897SelCam(err_val=SetAcquisitionMode(3),this)//kinetics
	Andor897SelCam(err_val=SetReadMode(4),this)//image acquisition mode
	Andor897SelCam(err_val=SetNumberKinetics(10),this)//number of pictures to acquire
	Andor897SelCam(err_val=SetExposureTime(0),this)//delay before exposure
	setBin(ap.bin);
	setGain(ap.gain);
	Andor897SelCam(err_val=SetKineticCycleTime(0),this)//we want minimum repition rate
	Andor897SelCam(err_val=SetFrameTransferMode(1),this)//frame transfer mode
	char* dir=toCharStar(string(DEFAULTWORKINGDIR)+"Error-this should not exist");
	Andor897SelCam(err_val=SetSpool(1,2,dir,10),this)
	float acc,exp2,minTrigTime;;
	Andor897SelCam(GetAcquisitionTimings(&exp2,&acc,&minTrigTime),this)
	delete[] dir;
	if (minTrigTime>ap.exp) return false;//change "<" to ">"
	return true;
}

//scanning
float Andor897::startKineticsFTExtTrig(string directory,int num,int newBin, int newGain){
	CheckExists(0)
	first=true;
	waitIdle();
	//setImageRegion(0,100,100);do this in autofocus
	unsigned int err_val;
	//ResetEvent(this->newImage);
	Andor897SelCam(err_val=SetTriggerMode(1),this)//external trigger
	Andor897SelCam(err_val=SetAcquisitionMode(3),this)//kinetics
	Andor897SelCam(err_val=SetReadMode(4),this)//image acquisition mode
	
	//num=15000;
	Andor897SelCam(err_val=SetNumberKinetics(num),this)//number of pictures to acquire
	Andor897SelCam(err_val=SetExposureTime(0),this)//delay before exposure
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		logFile.write(string("Error: could not set exposure time- ")+::toString((int)err_val),DEBUGANDOR897);
		heatUp();
		clickAbort();
	}
	float oldGain=gain;
	float oldBin=bin;
	if (newBin!=-1) setBin(newBin);
	if (newGain!=-1) setGain(newGain);
	gain=oldGain;
	bin=oldBin;
	Andor897SelCam(SetKineticCycleTime(0),this)//we want minimum repition rate
	Andor897SelCam(SetFrameTransferMode(1),this)//frame transfer mode
	//string directory=".\\output\\"+scanName+"\\";
	//directory=".\\output\\";
	//cout<<"directory is "<<directory<<endl;
	//string s=".\\output\\"+scanName+"\\";
	char* dir=toCharStar(directory);
	logFile.write(string("Spooling directory is ")+string(dir),DEBUGANDOR897);
	Andor897SelCam(err_val=SetSpool(1,2,dir,10),this)
	delete[] dir;
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set spooling options: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	float acc,exp2;
	Andor897SelCam(GetAcquisitionTimings(&exp2,&acc,&minTrigTime),this)
		logFile.write("FT Mode Scanning. Number Pictures: "+::toString(num)+" delay: "+::toString(exp2)+" Min Trigger Time (not updated properly by Andor SDK): "+::toString(minTrigTime),DEBUGANDOR897);
	long ret=0;
	Andor897SelCam(err_val=GetSizeOfCircularBuffer(&ret),this)
	if (err_val!=DRV_SUCCESS){
		cout<<"could not get size of circular buffer: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	Andor897SelCam(err_val=PrepareAcquisition(),this)

	Andor897SelCam(err_val=StartAcquisition(),this)
	Timer::wait(200);
	
	return minTrigTime;
}

float Andor897::startInternalFTKinetics(string directory,int num,float exp,int newBin, int newGain){
	CheckExists(0)
	first=true;
	waitIdle();
	//setImageRegion(0,100,100);do this in autofocus
	unsigned int err_val;
	//ResetEvent(this->newImage);
	Andor897SelCam(err_val=SetTriggerMode(6),this)//external start trigger
	Andor897SelCam(err_val=SetAcquisitionMode(3),this)//kinetics
	Andor897SelCam(err_val=SetReadMode(4),this)//image acquisition mode
	
	//num=15000;
	Andor897SelCam(err_val=SetNumberKinetics(num),this)//number of pictures to acquire
	Andor897SelCam(err_val=SetExposureTime(exp),this)//delay before exposure
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		logFile.write(string("Error: could not set exposure time- ")+::toString((int)err_val),DEBUGANDOR897);
		heatUp();
		clickAbort();
	}
	float oldGain=gain;
	float oldBin=bin;
	if (newBin!=-1) setBin(newBin);
	if (newGain!=-1) setGain(newGain);
	gain=oldGain;
	bin=oldBin;
	Andor897SelCam(SetKineticCycleTime(0),this)//we want minimum repition rate
	Andor897SelCam(SetFrameTransferMode(1),this)//frame transfer mode
	//string directory=".\\output\\"+scanName+"\\";
	//directory=".\\output\\";
	//cout<<"directory is "<<directory<<endl;
	//string s=".\\output\\"+scanName+"\\";
	char* dir=toCharStar(directory);
	logFile.write(string("Spooling directory is ")+string(dir),DEBUGANDOR897);
	Andor897SelCam(err_val=SetSpool(1,2,dir,10),this)
	delete[] dir;
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set spooling options: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	float acc,exp2;
	Andor897SelCam(GetAcquisitionTimings(&exp2,&acc,&minTrigTime),this)
	cout<<"FT Mode Internal Triggering. Number Pictures: "<<num<<" delay: "<<exp2<<" Cycle Time (not updated properly by Andor SDK): "<<minTrigTime<<endl;
	long ret=0;
	Andor897SelCam(err_val=GetSizeOfCircularBuffer(&ret),this)
	if (err_val!=DRV_SUCCESS){
		cout<<"could not get size of circular buffer: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	Andor897SelCam(err_val=PrepareAcquisition(),this)

	Andor897SelCam(err_val=StartAcquisition(),this)
	Timer::wait(200);
	
	return minTrigTime;
}

//stepwise autofocus
float Andor897::startNonFTKineticsExtTrig(int num,float newExp,int newBin, int newGain){
	CheckExists(0)
	logFile.write("NonFTKinetics started for Stepwise focusing");
	focusNum=num;
	unsigned int err_val;
	

	Andor897SelCam(SetSpool(0,2,"",10),this)
	Andor897SelCam(SetReadMode(4),this)//image acquisition mode	
	Andor897SelCam(SetAcquisitionMode(3),this)//kinetics
	Andor897SelCam(SetFrameTransferMode(0),this)//frame transfer mode
	Andor897SelCam(SetTriggerMode(1),this)//external trigger
	Andor897SelCam(SetExposureTime(newExp),this)
	Andor897SelCam(SetNumberKinetics(num),this)//number of pictures to acquire
	//SetKineticCycleTime(0);
	Andor897SelCam(SetFrameTransferMode(0),this)//frame transfer mode
	Andor897SelCam(SetExposureTime(newExp),this)
	float exp2,acc,kin;
	Andor897SelCam(err_val=GetAcquisitionTimings(&exp2,&acc,&kin),this)
	logFile.write(string("Kinetic Series Autofocusing. Number of Images: ")+toString(num)+" Desired exposure: "+toString(newExp)+" Actual exposure: "+toString(exp2)+" sec Min Trig Time: "+toString(kin));
	long index;
	Andor897SelCam(GetSizeOfCircularBuffer(&index),this)
	logFile.write(string("Circular Buffer has ")+toString(index)+" images available");
	//cout<<"actual gain: "<<gain<<endl;
	
	Andor897SelCam(PrepareAcquisition(),this)
	Andor897SelCam(StartAcquisition(),this)
	Timer::wait(200);//not sure if this is long enough. the delay seemed to be around 100ms
	return kin;
}

//autofocus
float Andor897::startFTKinetics(int num,float newExp,int newBin, int newGain){
	CheckExists(0)
	unsigned int err_val;
	waitIdle();
	ResetEvent(this->newImage);
	//CLEANONE=CreateEvent(NULL,FALSE,FALSE,NULL);
	//setImageRegion(0,25,25);
	
	float oldExp=exp;
	float oldGain=gain;
	float oldBin=bin;
	
	if (newBin!=-1) setBin(newBin);
	if (newGain!=-1) setGain(newGain);
	setExposure(.2);
	gain=oldGain;
	bin=oldBin;
			
	Andor897SelCam(err_val=SetTriggerMode(6),this)//external start trigger
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set external start trigger mode: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	Andor897SelCam(err_val=SetAcquisitionMode(3),this)//kinetics
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set kinetics acquisition mode for autofocus: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}

	//put this right before setframetransfermode because the ANDOR SDK is retarded
	if (newExp!=-1) setExposure(newExp); else newExp=oldExp;
	exp=oldExp;
	//setExposure(.2);

	if (newExp<0.035){
		logFile.write("Warning: kinetic series not in frame transfer mode. Exposure is too short");
		Andor897SelCam(err_val=SetFrameTransferMode(0),this)//frame transfer mode
	}else {
		Andor897SelCam(err_val=SetFrameTransferMode(1),this)
	}
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set frame transfer mode for autofocus: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	
	Andor897SelCam(err_val=SetNumberKinetics(num),this)//number of pictures to acquire
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set number of scans in kinetic series: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	focusNum=num;
	Andor897SelCam(err_val=SetKineticCycleTime(0),this)
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set kinetic cycle time for autofocus: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}

	Andor897SelCam(err_val=SetSpool(0,2,"",10),this)
	if (err_val!=DRV_SUCCESS){
		cout<<"could not set spooling options: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	
	//ResetEvent(this->newImage);
	float exp2,acc,kin;
	Andor897SelCam(err_val=GetAcquisitionTimings(&exp2,&acc,&kin),this)
	logFile.write(string("Kinetic Series Autofocusing. Number of Images: ")+toString(num)+" Desired exposure: "+toString(newExp)+" Actual exposure: "+toString(exp2)+" sec Kinetic Cycle Time: "+toString(kin));
	long index;
	Andor897SelCam(GetSizeOfCircularBuffer(&index),this)
	logFile.write(string("Circular Buffer has ")+toString(index)+" images available");
	//cout<<"actual gain: "<<gain<<endl;
	
	Andor897SelCam(PrepareAcquisition(),this)
	Andor897SelCam(StartAcquisition(),this)
	Timer::wait(200);//not sure if this is long enough. the delay seemed to be around 100ms
	return kin;
}



CImg<unsigned short>* Andor897::getNewPic(){
	CImg<unsigned short> *copyImage=new CImg<unsigned short>((maxXPixel-minXPixel+1)/bin,(maxYPixel-minYPixel+1)/bin);
	CheckExists(copyImage)
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
	LARGE_INTEGER current, freq;
	QueryPerformanceFrequency(&freq);
	WaitForSingleObject(newImage,INFINITE);
	QueryPerformanceCounter(&current);
	//cout<<"Waited "<<(current.QuadPart-start.QuadPart)*1000.0/freq.QuadPart<<" ms for event to be signaled. Acquisition complete: Transfering new data to memory"<<endl;
	unsigned int err_val;
	Andor897SelCam(err_val=GetMostRecentImage16(copyImage->data(),(maxXPixel-minXPixel+1)*(maxYPixel-minYPixel+1)/bin/bin),this)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get 16bit image: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}/*
	int status;
	Andor897SelCam(GetStatus(&status),this)
	if (status!=DRV_IDLE){
		cout<<"driver is not idle even though event was signaled...status="<<dec<<status<<endl;
	
		LARGE_INTEGER start;
		QueryPerformanceCounter(&start);
		LARGE_INTEGER current, freq;
		QueryPerformanceFrequency(&freq);
		while(true){
			Andor897SelCam(GetStatus(&status),this)
			if (status==DRV_IDLE) break;
		}
		QueryPerformanceCounter(&current);
		cout<<"took "<<(current.QuadPart-start.QuadPart)*1000.0/freq.QuadPart<<" ms to finish acqusition"<<endl; 
	}
	*/
	return copyImage;
}

CImg<unsigned short>* Andor897::getOldest(){
	CImg<unsigned short> *copyImage=new CImg<unsigned short>((maxXPixel-minXPixel+1)/bin,(maxYPixel-minYPixel+1)/bin);
	CheckExists(copyImage)
	unsigned int err_val;
	Andor897SelCam(err_val=GetOldestImage16(copyImage->data(),(maxXPixel-minXPixel+1)*(maxYPixel-minYPixel+1)/bin/bin),this)
	if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
		cout<<"could not get 16bit image: "<<err_val<<endl;
		system("pause");
		heatUp();
		exit(0);;
	}
	return copyImage;
}

unsigned __stdcall Andor897::FocusThread(void* param){
	Andor897* cam=(Andor897*) param;
	if (!cam->isPresent) return 0;
	//ResetEvent(cam->newImage);
	cam->focusing=true;
	char c[256];
	unsigned ind=0;
	unsigned bestInd=0;
	ostringstream myStream;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LONGLONG maxScore=0;//UINTMAXINT;MAXLONGLONG;//0xFFFFFFFFFFFFFFFF;
	LONGLONG *score=new LONGLONG[cam->focusNum];
	CImg<unsigned short>** images=new CImg<unsigned short>*[cam->focusNum];
	double *times=new double[cam->focusNum];
	int numGroups;
	if (cam->focusNum % MAXIMUM_WAIT_OBJECTS ==0)
		numGroups=cam->focusNum/MAXIMUM_WAIT_OBJECTS;
	else numGroups=1+cam->focusNum/MAXIMUM_WAIT_OBJECTS;
	if (numGroups>MAXIMUM_WAIT_OBJECTS){
		logFile.write("Cannot handle that number of images: "+::toString(numGroups),true,::toString(cam->focusNum));
		return 0;
	}
	HANDLE* threadGroups=new HANDLE[numGroups];
	ThreadGroup* tg=new ThreadGroup();
	int currentGroup=0;
	LARGE_INTEGER start,end;
	int status;
	unsigned int threadID;
//	CImg<unsigned short>* temp;
//	CImg<unsigned short>* best=new CImg<unsigned short>((cam->maxXPixel-cam->minXPixel+1)/cam->bin,(cam->maxYPixel-cam->minYPixel+1)/cam->bin);
	CImg<unsigned short>* image;	unsigned int err_val;
	while(cam->isIdle());//wait for Andor897 to start
	while(cam->focusing && ind<cam->focusNum){
		bool b=false;
		if (!cam->isIdle()){//only wait if Andor897 is not done acquiring
WAITFORIMAGE:			WaitForSingleObject(cam->newImage,INFINITE);
			b=true;
		}
		image=new CImg<unsigned short>((cam->maxXPixel-cam->minXPixel+1)/cam->bin,(cam->maxYPixel-cam->minYPixel+1)/cam->bin);

		Andor897SelCam(err_val=GetOldestImage16(image->data(),((cam->maxXPixel-cam->minXPixel+1)/cam->bin)*((cam->maxYPixel-cam->minYPixel+1)/cam->bin)),cam)
		if (DRV_SUCCESS!=err_val){//look in current directory for .ini file
			if (err_val=20024) goto WAITFORIMAGE;
			cout<<"could not get 16bit image: "<<err_val<<endl;
			int stat;
			Andor897SelCam(GetStatus(&stat),cam)
			cout<<"Driver status: "<<stat<<endl;
			system("pause");
			cam->heatUp();
			exit(0);;
		}
		//cout<<"got image"<<endl;
		//*image.ptr(0,0)=0;
		//*image.ptr(0,1)=0;
		//*image.ptr(0,2)=0;
		//*image.ptr(0,3)=0;
		//cout<<"center of image is"<<*image.ptr(100,100)<<endl;
		//QueryPerformanceCounter(&start);
		
		if (ind % MAXIMUM_WAIT_OBJECTS==0 && ind>0){
			tg->num=MAXIMUM_WAIT_OBJECTS;
			HANDLE group=(HANDLE) _beginthreadex(NULL,0,
				   Andor897::threadGroup,
				   tg,
				   0,
				   &threadID);
		   threadGroups[currentGroup]=group;
			currentGroup+=1;
			tg=new ThreadGroup();
		}
		
		Score* s=new Score();
		images[ind]=image;
		s->images=images;
		s->ind=ind;
		s->result=score;
		s->times=times;
		HANDLE h=(HANDLE) _beginthreadex(NULL,0,
				   Andor897::getScore,
				   s,
				   0,
				   &threadID);
		tg->threads[ind%MAXIMUM_WAIT_OBJECTS]=h;
		//cout<<"score is "<<score<<endl;
		//QueryPerformanceCounter(&end);
		//myStream.str("");
		//	myStream<<"Image number "<<(ind+1)<<" in series. Score is "<<score<<flush;
		//	cam->showImage(image,myStream.str());
	/*	if (score>maxScore){
			maxScore=score;
			bestInd=ind;
			temp=best;
			best=image;
			image=temp;
			//if (cam->debugFocus){
			myStream.str("");
			myStream<<"New Best Image Found. Image number "<<(ind+1)<<" in series. Score is "<<score<<" mean pixel value: "<<best->sum()/best->size()<<flush;
			cam->showImage(best,myStream.str());
			//}
		}
		*/
		//if (debugFocus){
			
		//}
		
		ind++;
		//if (cam->isIdle()) {
		//	cout<<"Focus thread saw: "<<ind<<" images"<<endl;
		//	break;
		//}
		
		//cout<<"Analysis of image took "<< ((double)(end.QuadPart-start.QuadPart))/freq.QuadPart<< " sec"<<endl;
		
	}
	tg->num=((ind-1) % MAXIMUM_WAIT_OBJECTS)+1;
	HANDLE Tgroup=(HANDLE) _beginthreadex(NULL,0,
				   Andor897::threadGroup,
				   tg,
				   0,
				   &threadID);
   threadGroups[currentGroup]=Tgroup;
   Timer threadWait(true);
	WaitForMultipleObjects(numGroups,threadGroups,true,INFINITE);
	threadWait.stopTimer();
	logFile.write("Focus wait for score threads took "+::toString(threadWait.getTime())+"ms",DEBUGANDOR897);
	for(int i=0;i<numGroups;i++){
		CloseHandle(threadGroups[i]);
	}
	delete[] threadGroups;
	//Timer::wait(3000);
	double totalTime=0;
	for (int i=0;i<cam->focusNum;i++){

		totalTime+=times[i];	
		if (score[i]>maxScore){
			bestInd=i;
			maxScore=score[i];
		}
	}
#ifdef DEBUGANDORFOCUS
	if (DEBUGANDORFOCUS){
	logFile.write(string("Best index is ")+::toString((int)bestInd)+" with score "+::toString(maxScore)+" Total images= "+::toString(cam->focusNum),true);
	for (int i=0;i<cam->focusNum;i++){
		showImage(images[i],"Image "+::toString(i)+" Score is "+::toString(score[i]));
	}
	system("pause");
	cont.displays.closeAll();
	}
#endif
	for (int i=0;i<cam->focusNum;i++){
		delete images[i];
	}
	delete [] images;
	logFile.write("Total calculation time is "+::toString(totalTime)+"ms Average is "+::toString(totalTime/cam->focusNum),DEBUGANDOR897);
	//cam->showImage(best,toString((int)bestInd));
	//cout<<"Focus thread saw: "<<ind<<" images"<<endl;
	cam->focusing=false;
	//if (cam->debug) 
	//cam->showImage(best, "Best image in series");
	//delete best;
	//delete image;
	//cout<<"Max Score is: "<<maxScore<<" Image #"<<bestInd+1<<endl;
	//cam->setImageRegion(0,100,100);
	focusLogFile.write(Timer::getSysTime()+"\t"+::toString((int)bestInd)+"\t"+::toString(maxScore),DEBUGANDOR897);
	return bestInd;
}
/*

LONGLONG Andor897::getScore(CImg<unsigned short>& image){
	CImg<unsigned short>* Cimage=&image;
	//return Cimage->norm();
	//autocorrelation method of autofocus follows:  F=sum(i=1:M-1,j=1:N)g(i,j)*g(i+1,j)-sum(i=1:M-2,j=1:N)g(i,j)*g(i+2,j)
	//this is autocorrelation in the y direction
	LONGLONG sum1=0;
	int ind;
	//LONGLONG sum2=0;
	for(int j=0;j<=Cimage->width-1;j++){
		for(int i=0;i<=Cimage->height-3;i++){
			sum1+=Cimage->ptr()[i*Cimage->width+j]*Cimage->ptr()[(i+1)*Cimage->width+j]-Cimage->ptr()[i*Cimage->width+j]*Cimage->ptr()[(i+2)*Cimage->width+j];
			//sum2+=Cimage->ptr()[i*Cimage->width+j]*Cimage->ptr()[(i+2)*Cimage->width+j];
		}
	}
	for(int j=0;j<Cimage->width-1;j++){
		sum1+=Cimage->ptr()[(Cimage->height-2)*Cimage->width+j]*Cimage->ptr()[(Cimage->height-1)*Cimage->width+j];
	}
	return sum1;
}

*/
unsigned __stdcall Andor897::threadGroup(void* param){
	ThreadGroup* tg=(ThreadGroup*) param;
	WaitForMultipleObjects(tg->num,tg->threads,true,INFINITE);
	for(int i=0;i<tg->num;i++){
		CloseHandle(tg->threads[i]);
	}
	delete param;
	return 0;
}
unsigned __stdcall Andor897::getScore(void* param){//CImg<unsigned short>* Cimage,LONGLONG* sum1){
	Score* s=(Score*) param;
	cimg_library::CImg<unsigned short>* Cimage=s->images[s->ind];
	LONGLONG res;
	LONGLONG* sum1=&res;
	(*sum1)=0;
	//CImg<unsigned short>* Cimage=&image;
	//return Cimage->norm();
	//autocorrelation method of autofocus follows:  F=sum(i=1:M-1,j=1:N)g(i,j)*g(i+1,j)-sum(i=1:M-2,j=1:N)g(i,j)*g(i+2,j)
	//this is autocorrelation in the y direction (along a column)
	//int ind;
	//double bad=Cimage(0,0);
	//double bad2=Cimage(1,0);
	//double bad3=Cimage(3,0);
	//Timer::wait(1000);
	//if (true) return 0;
	//LONGLONG sum2=0;
	Timer calcTime(true);
	for(int j=0;j<=Cimage->width()-1;j++){
		for(int i=0;i<=Cimage->height()-3;i++){
		/*
			if (image(j,i)<0 || image(j,i)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i)=0;
			}
			if (image(j,i+1)<0 || image(j,i+1)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i+1)=0;
			}
			if (image(j,i+2)<0 || image(j,i+2)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i+2)=0;
			}
			*/
			//sum1+=((ULONGLONG)Cimage(j,i)*Cimage(j,i+1)-Cimage(j,i)*Cimage(j,i+2));
			(*sum1)+=Cimage->data()[i*Cimage->width()+j]*Cimage->data()[(i+1)*Cimage->width()+j]-Cimage->data()[i*Cimage->width()+j]*Cimage->data()[(i+2)*Cimage->width()+j];
			//sum2+=Cimage->ptr()[i*Cimage->width+j]*Cimage->ptr()[(i+2)*Cimage->width+j];
		}
	}
	for(int j=0;j<Cimage->width()-1;j++){
		/*
		if (image(j,image.height-1)<0 || image(j,image.height-1)>16383){
			//cout<<"error pixel"<<endl; 
			image(j,image.height-1)=0;
		}
		if (image(j,image.height-2)<0 || image(j,image.height-2)>16383){
			//cout<<"error pixel"<<endl; 
			image(j,image.height-2)=0;
		}
		*/
		//sum1+=Cimage(j,Cimage.height-2)*Cimage(j,Cimage.height-1);
		(*sum1)+=Cimage->data()[(Cimage->height()-2)*Cimage->width()+j]*Cimage->data()[(Cimage->height()-1)*Cimage->width()+j];
	}
	calcTime.stopTimer();
	s->result[s->ind]=*sum1;
	s->times[s->ind]=calcTime.getTime();
	delete param;
	return 0;
}



void Andor897::getScore(CImg<unsigned short>* Cimage,LONGLONG* sum1){
	(*sum1)=0;
	//CImg<unsigned short>* Cimage=&image;
	//return Cimage->norm();
	//autocorrelation method of autofocus follows:  F=sum(i=1:M-1,j=1:N)g(i,j)*g(i+1,j)-sum(i=1:M-2,j=1:N)g(i,j)*g(i+2,j)
	//this is autocorrelation in the y direction (along a column)
	//int ind;
	//double bad=Cimage(0,0);
	//double bad2=Cimage(1,0);
	//double bad3=Cimage(3,0);
	//Timer::wait(1000);
	//if (true) return 0;
	//LONGLONG sum2=0;
	for(int j=0;j<=Cimage->width()-1;j++){
		for(int i=0;i<=Cimage->height()-3;i++){
		/*
			if (image(j,i)<0 || image(j,i)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i)=0;
			}
			if (image(j,i+1)<0 || image(j,i+1)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i+1)=0;
			}
			if (image(j,i+2)<0 || image(j,i+2)>16383) {
				//cout<<"error pixel"<<endl;
				image(j,i+2)=0;
			}
			*/
			//sum1+=((ULONGLONG)Cimage(j,i)*Cimage(j,i+1)-Cimage(j,i)*Cimage(j,i+2));
			(*sum1)+=Cimage->data()[i*Cimage->width()+j]*Cimage->data()[(i+1)*Cimage->width()+j]-Cimage->data()[i*Cimage->width()+j]*Cimage->data()[(i+2)*Cimage->width()+j];
			
			//sum2+=Cimage->ptr()[i*Cimage->width+j]*Cimage->ptr()[(i+2)*Cimage->width+j];
		}
	}
	for(int j=0;j<Cimage->width()-1;j++){
		/*
		if (image(j,image.height-1)<0 || image(j,image.height-1)>16383){
			//cout<<"error pixel"<<endl; 
			image(j,image.height-1)=0;
		}
		if (image(j,image.height-2)<0 || image(j,image.height-2)>16383){
			//cout<<"error pixel"<<endl; 
			image(j,image.height-2)=0;
		}
		*/
		//sum1+=Cimage(j,Cimage.height-2)*Cimage(j,Cimage.height-1);
		(*sum1)+=Cimage->data()[(Cimage->height()-2)*Cimage->width()+j]*Cimage->data()[(Cimage->height()-1)*Cimage->width()+j];
		
	}
	//cout<<"Double is:"<<d<<" ULONGLONG is:"<<*sum1<<endl;
}

/*
int main(int argc, char *argv[]){
	cout<<"did this work"<<endl;
	Andor897::showFile("trees.tif");
	cout<<"can I print this"<<endl;
	system("pause");
}*/


