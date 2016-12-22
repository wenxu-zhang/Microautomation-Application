// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, April 04, 2012</lastedit>
// ===================================
#include "Scan.h"
#include "Timer.h"
#include "Utils.h"
#include "XYStage.h"
#include <string>
extern Controller cont;
extern Record logFile;


Scan::Scan(std::vector<AcquisitionChannel>& acquisitionChannels,string workingDir,ScanFocus *sf,int showOnScreenPeriod)
:_numSpotsLocal(0),showOnScreenPeriod(showOnScreenPeriod),triggerStartEvent(NULL),hRunScanThread(NULL)
{
	abortEvent=CreateEvent(NULL,true,false,NULL);//set this event in case of abort to signal threads to stop execution
	generationCompleteEvent=CreateEvent(NULL,false, false,NULL);
	readyForStartEvent=CreateEvent(NULL,false,false,NULL);
	this->sf=sf;
	workingDirectory=cont.workingDir+workingDir+"\\";
	scanLogFile=new Record(workingDirectory+"scanLog.txt",0,logFile.swatch);
	groupChannels2Series(acquisitionChannels);
	hStartScan=CreateEvent(NULL,true,true,NULL);
	scanTotal=0;
	for(vector<AcquisitionSeries*>::iterator si=acquisitionSeries.begin();si!=acquisitionSeries.end();si++){
		for(vector<AcquisitionGroup*>::iterator g=(*si)->acquisitionGroups.begin();g!=(*si)->acquisitionGroups.end();g++){
			(*g)->acquisitionChannels.front().out->cam->t->getWaveform(**g);
		}
	}
}

Scan::~Scan(){
		//wait();
		//delete sf;
		disableStartEvent();
		scanLogFile->write("Closing file.\n");
		delete scanLogFile;
		for(vector<AcquisitionSeries*>::iterator si=acquisitionSeries.begin();si!=acquisitionSeries.end();si++){
			delete (*si);
		}
}

void Scan::abort(){
		SetEvent(abortEvent);
		for (vector<HANDLE>::iterator i=saveSeriesThreads.begin();i!=saveSeriesThreads.end();i++){
			WaitForSingleObject(*i,INFINITE);
			CloseHandle((*i));
		}	
		saveSeriesThreads.clear();
		if (hRunScanThread)
			WaitForSingleObject(hRunScanThread,INFINITE);
		hRunScanThread=NULL;
		ResetEvent(abortEvent);
		
}
void Scan::wait(){//this must be called by the derived class' destructor to ensure the scan object is no longer needed 
	if (hRunScanThread){
			WaitForSingleObject(hRunScanThread,INFINITE);
			CloseHandle(hRunScanThread);
	}
	for (vector<HANDLE>::iterator i=saveSeriesThreads.begin();i!=saveSeriesThreads.end();i++){
			WaitForSingleObject((*i),INFINITE);
			CloseHandle((*i));
	}		
	saveSeriesThreads.clear();
		
	hRunScanThread=NULL;		
}

bool Scan::addAcquisitionGroup(std::vector<AcquisitionChannel> acquisitionChannels, int numLocalSpotChanges){
	AcquisitionGroup* ag= new AcquisitionGroup(numChannels(),numLocalSpotChanges);
	for(vector<AcquisitionChannel>::iterator i=acquisitionChannels.begin();i!=acquisitionChannels.end();i++){
		if (!ag->addAcquisitionChannel(*i)){
			delete ag;
			return false;
		}
	}
	if(!acquisitionSeries.back()->addAcquisitionGroup(ag)){
		//need new series
		AcquisitionSeries *currentSeries=new AcquisitionSeries(acquisitionSeries.back()->numChannels()+acquisitionSeries.back()->cumNumChans);
		currentSeries->addAcquisitionGroup(ag);
		scanLogFile->write("Acquisition Series #"+toString((int)acquisitionSeries.size()+1),DEBUGSCAN);
		acquisitionSeries.push_back(currentSeries);
	}
	vector<int> v(ag->numChans(),acquisitionSeries.size()-1);
	chanToSeries.insert(chanToSeries.end(),v.begin(),v.end());
	_numSpotsLocal+=ag->numLocalSpotChanges;
	ag->getChan(0).out->cam->t->getWaveform(*ag);
	return true;
}

void Scan::groupChannels2Series(std::vector<AcquisitionChannel> acquisitionChannels){
	AcquisitionSeries *currentSeries=new AcquisitionSeries(0);
	AcquisitionGroup *currentGroup=new AcquisitionGroup(0);
	vector<int> v;
	scanLogFile->write("Acquisition Channels:\n\tAcquisition Series #1 \n\tAcquisition Group #1:",DEBUGSCAN);
	int i=0;
	for(vector<AcquisitionChannel>::iterator c=acquisitionChannels.begin();c!=acquisitionChannels.end();c++,i++){
		if (!currentGroup->addAcquisitionChannel(*c)){
			//need new group
			if(!currentSeries->addAcquisitionGroup(currentGroup)){
				//need new series
				v=vector<int>(currentSeries->numChannels(),acquisitionSeries.size());
				chanToSeries.insert(chanToSeries.end(),v.begin(),v.end());
				acquisitionSeries.push_back(currentSeries);
				_numSpotsLocal+=currentSeries->numLocalSpotChanges;
				currentSeries=new AcquisitionSeries(currentSeries->numChannels()+currentSeries->cumNumChans);
				currentSeries->addAcquisitionGroup(currentGroup);
				scanLogFile->write("Acquisition Series #"+toString((int)acquisitionSeries.size()+1),DEBUGSCAN);
			}
			//acquisition channel added to new group 
			currentGroup=new AcquisitionGroup(currentGroup->cumNumChans+currentGroup->numChans());
			currentGroup->addAcquisitionChannel(*c);
			scanLogFile->write("Acquisition Group #"+toString((int)currentSeries->acquisitionGroups.size()+1),DEBUGSCAN);
			scanLogFile->write((*c).toString(),DEBUGSCAN);
		}
	}
	//what to do with last acquisition group
	if(!currentSeries->addAcquisitionGroup(currentGroup)){
		//need new series
		v=vector<int>(currentSeries->numChannels(),acquisitionSeries.size());
		chanToSeries.insert(chanToSeries.end(),v.begin(),v.end());
		acquisitionSeries.push_back(currentSeries);
		_numSpotsLocal+=currentSeries->numLocalSpotChanges;
		currentSeries=new AcquisitionSeries(currentSeries->numChannels()+currentSeries->cumNumChans);
		currentSeries->addAcquisitionGroup(currentGroup);
		scanLogFile->write("Acquisition Series #"+toString((int)acquisitionSeries.size()+1),DEBUGSCAN);
	}
	v=vector<int>(currentSeries->numChannels(),acquisitionSeries.size());
	chanToSeries.insert(chanToSeries.end(),v.begin(),v.end());
	acquisitionSeries.push_back(currentSeries);
	_numSpotsLocal+=currentSeries->numLocalSpotChanges;
}

void Scan::enableStartEvent(){
	if (triggerStartEvent) {
		HANDLE temp=triggerStartEvent;
		triggerStartEvent=NULL;
		SetEvent(temp);
		CloseHandle(temp);
	}
	triggerStartEvent=CreateEvent(NULL,false, false,NULL);
	for(vector<AcquisitionSeries*>::iterator s=acquisitionSeries.begin();s!=acquisitionSeries.end();s++){
		for(vector<AcquisitionGroup*>::iterator g=(*s)->acquisitionGroups.begin();g!=(*s)->acquisitionGroups.end();g++){
			(*g)->acquisitionChannels.front().out->cam->t->setTriggerStartEvent(triggerStartEvent);
		}
	}
}

void Scan::disableStartEvent(){
	if (triggerStartEvent) {
		HANDLE temp=triggerStartEvent;
		triggerStartEvent=NULL;
		SetEvent(temp);
		CloseHandle(temp);
	}
	for(vector<AcquisitionSeries*>::iterator s=acquisitionSeries.begin();s!=acquisitionSeries.end();s++){
		for(vector<AcquisitionGroup*>::iterator g=(*s)->acquisitionGroups.begin();g!=(*s)->acquisitionGroups.end();g++){
			(*g)->acquisitionChannels.front().out->cam->t->disableTriggerStartEvent();
		}
	}
}

bool Scan::waitGenerationComplete(){
	HANDLE h[]={abortEvent,generationCompleteEvent};//give preference to abortEvent
	DWORD eventNum=WaitForMultipleObjects(2,h,false,INFINITE);
	if (eventNum==WAIT_OBJECT_0)
			return false;
	return true;
}

bool Scan::waitReadyForStartEvent(){
	HANDLE h[]={abortEvent,readyForStartEvent};//give preference to abortEvent
	DWORD eventNum=WaitForMultipleObjects(2,h,false,INFINITE);
	if (eventNum==WAIT_OBJECT_0)
			return false;
	return true;
}

void Scan::sendStartEvent(){
	if (!triggerStartEvent)
		logFile.write("Scan Error: trigger Start event is not enabled",true);
	SetEvent(triggerStartEvent);
}
struct RunScanArg{
	Scan* s;
	int num;
};


void Scan::runScan(int numScan,bool wait){
	if (this->acquisitionSeries.size()==0){
		logFile.write("No Acquisition Channels For this Scan. No imaging done",true);
		return;
	}
	if (wait)
		_runScan(numScan);
	else{
		RunScanArg* r=new RunScanArg();
		r->s=this;
		r->num=numScan;
		unsigned int threadID;
		hRunScanThread=(HANDLE) _beginthreadex(NULL,0, &runScanThread,r,0,&threadID);
		cont.threads.addThread(hRunScanThread);
		while(r->s->saveSeriesThreads.size()==0);
	}
}

unsigned __stdcall Scan::runScanThread(void* param){
	RunScanArg* r=(RunScanArg*) param;
	r->s->_runScan(r->num);
	delete r;
	return 0;
}

//Eric's run scan with internal tracking of total scans performed
void Scan::_runScan(int numScan){
	Timer swatch(true);
	string fileName,comment;
	int seriesNum;
	Timer t(true);
	if (scanTotal==0) scanLogFile->write("x\ty\tz\tTemp\tTemp2\ttime",DEBUGSCAN);
	/*bool b1=acquisitionChannels.size()==1;
	bool b2=numSpots()==1;
	bool b3=acquisitionChannels.front().out->cam->isTriggerable(acquisitionChannels.front().ap);
	bool singleImageTimeSeries=b1&&b2&&b3;	
	AcquisitionGroup ag;
	if (singleImageTimeSeries){
	for(int i=0;i<numScan;i++){
	ag.addAcquisitionChannel(acquisitionChannels.front());
	}
	}*/
	for(int n=0;n<numScan;n++,scanTotal++){
		//preallocate space for 2D vectors
		//if (singleImageTimeSeries){//only one channel and only one spot so treat each scan as a different channel
		//	x.push_back(vector<vector<int>>(numSpots(),vector<int>(numScan,0)));
		//	y.push_back(vector<vector<int>>(numSpots(),vector<int>(numScan,0)));
		//	z.push_back(vector<vector<double>>(numSpots(),vector<double>(numScan,0)));
		//	temp.push_back(vector<vector<double>>(numSpots(),vector<double>(numScan,0)));
		//	temp2.push_back(vector<vector<double>>(numSpots(),vector<double>(numScan,0)));
		//	timeStamp.push_back(vector<vector<string>>(numSpots(),vector<string>(numScan,"")));
		//	n=numScan-1;//so we don't go through this loop again  scanTotal will only increment by 1
		//}else{
		x.push_back(vector<vector<int>>(numSpotsGlobal(),vector<int>(numChannels(),0)));
		y.push_back(vector<vector<int>>(numSpotsGlobal(),vector<int>(numChannels(),0)));
		z.push_back(vector<vector<double>>(numSpotsGlobal(),vector<double>(numChannels(),0)));
		temp.push_back(vector<vector<double>>(numSpotsGlobal(),vector<double>(numChannels(),0)));
		temp2.push_back(vector<vector<double>>(numSpotsGlobal(),vector<double>(numChannels(),0)));
		timeStamp.push_back(vector<vector<string>>(numSpotsGlobal(),vector<string>(numChannels(),"")));
		//}
		for(int p=0;p<numSpotsGlobal();p++){
			move2NextSpotGlobal(sf,p);
			int snum=0;
			for(vector<AcquisitionSeries*>::iterator si=acquisitionSeries.begin();si!=acquisitionSeries.end();si++,snum++){

				//start kinetic series if necessary
				//logFile.write(string("Run Scan: time until just before start series was ")+swatch.toString());
				if (acquisitionSeries.size()==1){
					if (snum==0 && p==0)//since there is only one series, group all runs of this series into one large series
						saveSeriesThreads.push_back(acquisitionSeries.front()->acquisitionGroups.front()->acquisitionChannels.front().out->cam->startSeries(numScan*numSpotsGlobal()*((*si)->numImages()+1)-1,scanTotal,this));//ER 2/13/11 +1 because dummy between each series and -1 because not dummy after last series......//ER 5/13/09 -1 because the last acquisition group is not following by a dummy
				}else //just run this series
					saveSeriesThreads.push_back((*si)->acquisitionGroups.front()->acquisitionChannels.front().out->cam->startSeries((*si)->numImages(),snum+(p+scanTotal*numSpotsGlobal())*acquisitionSeries.size(),this));	
				//logFile.write(string("Run Scan: time until just after start series was ")+swatch.toString());
				//int gNum=0; not needed, just compare to acquisitionSeries.front().acquisitionGroups.begin()
				for(vector<AcquisitionGroup*>::iterator g=(*si)->acquisitionGroups.begin();g!=(*si)->acquisitionGroups.end();g++){
					int numLocalSpots=(*g)->numLocalSpotChanges;
					bool localMove=true;
					if (numLocalSpots==0){
						numLocalSpots=1;
						localMove=false;
					}
					//only enter ring buffer and set waveform if it is the first time and we have a single series with a single group
					//OR
					//there are multiple groups and we need to enter ring buffer for each group
					if ((acquisitionSeries.size()!=1 || acquisitionSeries.front()->acquisitionGroups.size()!=1) || (acquisitionSeries.size()==1 && acquisitionSeries.front()->acquisitionGroups.size()==1 && snum==0 && p==0)){
						//we don't want to turn the light on...just prepare the acquisitionChannel
						//passing an intensity value of 0 will do this
						AcquisitionChannel tempChan=(*g)->getChan(0+(*g)->cumNumChans);
						tempChan.intensity=tempChan.chan->lite().ls->cp->zeroIntensity;
						tempChan.on(false);
						//must enter ring buffer for all light sources
						for(set<LightSource*>::iterator light=(*g)->lights.begin();light!=(*g)->lights.end();light++){
							(*light)->enterRingBuffer(**g);
						}
						(*g)->acquisitionChannels.front().out->cam->t->setWaveform((**g));//set timings and configuration for this generation
					}
					for(int i=0;i<numLocalSpots;i++){
						if (localMove){
							cont.stg->wait();
							cont.focus->wait();
							move2NextSpotLocal(sf,i);
						}
						
						////prepare ring buffer and waveforms if we are triggering and this is the first time or we have different acquisition groups
						//if ((*g).acquisitionChannels.size()!=1 && (g==acquisitionSeries.front().acquisitionGroups.begin() || acquisitionSeries.front().acquisitionGroups.size()!=1)){
						//	for(set<LightSource*>::iterator light=(*g).lights.begin();light!=(*g).lights.end();light++){
						//		(*light)->enterRingBuffer(*g);
						//	}
						//	(*g).acquisitionChannels.front().out->cam->t->setWaveform((*g));
						//}

						//Before we figure out how to get out the data for each channel, assign the final value for all AcquisitionChannels in this AcquisitionGroup
						int xCurrent=cont.stg->getX(),yCurrent=cont.stg->getY();
						double zCurrent=cont.focus->getZ(),tempCurrent=cont.te.getTemp(),temp2Current=cont.te.getTemp2();
						string timeStampCurrent=Timer::getSysTime();
						int totalImages;
						//if (singleImageTimeSeries)//if only one channel and one spot make sure we treat each scan as a separate channel
						//	totalImages=numScan;
						//else
						totalImages=(*g)->numChans();
						for(int c=0;c<totalImages;c++){
							x[scanTotal][p][c]=xCurrent;
							y[scanTotal][p][c]=yCurrent;
							z[scanTotal][p][c]=zCurrent;
							temp[scanTotal][p][c]=tempCurrent;
							temp2[scanTotal][p][c]=temp2Current;
							timeStamp[scanTotal][p][c]=timeStampCurrent;
							scanLogFile->write(toString(x[scanTotal][p][c])+"\t"+toString(y[scanTotal][p][c])+"\t"+toString(z[scanTotal][p][c])+"\t"+toString(temp[scanTotal][p][c])+"\t"+toString(temp2[scanTotal][p][c])+"\t"+timeStamp[scanTotal][p][c],DEBUGSCAN);
						}
						//actually start the acquisition
						/*if ((*g)->imageNumToChanNum.size()==1 && !(*g)->getChan(0).chan->lite().ls->isTriggerable()){//single takePicture
						cont.stg->wait();
						cont.focus->wait();
						getImageProperties(scanTotal,p,0,fileName,comment);
						(*g)->getChan(0).out->cam->takePicture(&((*g)->getChan(0)),workingDirectory+fileName);

						//why does this need to be a special case?
						//}else if((*g)->acquisitionChannels.size()==1 && !(*g)->acquisitionChannels.front().chan->lite().ls->isTriggerable() ){//single picture triggering
						cont.stg->wait();
						cont.focus->wait();
						(*g)->acquisitionChannels.front().on();
						(*g)->acquisitionChannels.front().out->cam->trigger();
						Timer::wait((*g)->acquisitionChannels.front().ap.exp*1000);
						(*g)->acquisitionChannels.front().out->cam->trigger();
						//dummy acquisition
						(*g)->acquisitionChannels.front().off();
						//
						}else*///normal triggering
						
						
						//logFile.write(string("Run Scan: time until just before prepareWaveform was ")+swatch.toString());
						(*g)->acquisitionChannels.front().out->cam->t->prepareWaveform(**g);//write data
						cont.stg->wait();
						cont.focus->wait();
						(*g)->acquisitionChannels.front().wait();
						if (!(*g)->acquisitionChannels.front().chan->lite().ls->isTriggerable())
							(*g)->getChan(0+(*g)->cumNumChans).on(true);

						//this should not be needed anymore since we precalculate the waveform  
						//system("pause");//hack to control timing,
						//cont.df.getDefiniteFocus(11,false);//hack to focus once prior to imaging
						//logFile.write(string("Run Scan: time until just before start generation was ")+swatch.toString());
						SetEvent(readyForStartEvent);
						if (!(*g)->acquisitionChannels.front().out->cam->t->generateWaveform(abortEvent))
							return;//abortEvent was signaled
						SetEvent(generationCompleteEvent);
						if (!(*g)->acquisitionChannels.front().chan->lite().ls->isTriggerable())
							(*g)->getChan(0).off(true);

					}
				}
			}
		}
	}

	logFile.write(string("Scan completed in ")+t.toString(),true);
}

//Ying-Ja's Run Scan with external tracking of scan numbers
void Scan::runScan(int numScan, int scanNum){//scanNum tells runScan how many times the scan has run before this current call (cuz you might need to call runScan several timeStamps and wash in between)
/*
	string fileName,comment;
	int numImages,seriesNum;
	Timer t(true);
	if(scanNum==0) scanLogFile->write("x\ty\tz\tTemp\tTemp2\ttime",DEBUGSCAN);
	if (acquisitionSeries.size()==1 && acquisitionSeries.front().acquisitionGroups.size() >=1 && acquisitionSeries.front().acquisitionGroups.front().acquisitionChannels.size() >= 1){
		for(int n=0;n<numScan;n++){
			if (scanNum==-1) scanNum=scanTotal;
			scanTotal++;
			WaitForSingleObject(hStartScan,180000);
			//numImages = numSpots*(numChannels+numDummies)-1  last dummy not taken
			numImages=numSpots()*(acquisitionSeries.front().numChannels+acquisitionSeries.front().acquisitionGroups.size())-1;
			seriesNum=scanNum+n;
			if(acquisitionChannels.size()!=1) saveSeriesThreads.push_back(acquisitionSeries.front().acquisitionGroups.front().acquisitionChannels.front().out->cam->startSeries(numImages+1,seriesNum,this));
			//acquisitionChannels[0].out->cam->t->triggerSingleLine(1);
			SetEvent(hStartScan);
			vector<vector<int>> spotX,spotY;
			vector<vector<double>> spotZ,spotTemp,spotTemp2;
			vector<vector<string>> spotT;
			x.push_back(spotX);
			y.push_back(spotY);
			z.push_back(spotZ);
			temp.push_back(spotTemp);
			temp2.push_back(spotTemp2);
			timeStamp.push_back(spotT);
			for(int p=0;p<numSpots();p++){
				move2NextSpot(sf);
				vector<int> currentX, currentY;
				vector<double> currentTemp, currentTemp2, currentZ;
				vector<string> currentT;
				x.back().push_back(currentX);
				y.back().push_back(currentY);
				z.back().push_back(currentZ);
				temp.back().push_back(currentTemp);
				temp2.back().push_back(currentTemp2);
				timeStamp.back().push_back(currentT);
				for(vector<AcquisitionGroup>::iterator g=acquisitionSeries.front().acquisitionGroups.begin();g!=acquisitionSeries.front().acquisitionGroups.end();g++){
					(*g).acquisitionChannels.front().on();
					cont.stg->wait();
					cont.focus->wait();
					WaitForSingleObject(hStartScan,INFINITE);
					//Before we figure out how to get out the data for each channel, assign the final value for all AcquisitionChannels in this AcquisitionGroup
					int cnum=0;
					for(vector<AcquisitionChannel>::const_iterator c=(*g).acquisitionChannels.begin();c!=(*g).acquisitionChannels.end();c++,cnum++){
						x.back().back().push_back(cont.stg->getX());
						y.back().back().push_back(cont.stg->getY());
						z.back().back().push_back(cont.focus->getZ());
						temp.back().back().push_back(cont.te.getTemp());
						temp2.back().back().push_back(cont.te.getTemp2());
						timeStamp.back().back().push_back(Timer::getSysTime());
						scanLogFile->write(toString(x.back().back().back())+"\t"+toString(y.back().back().back())+"\t"+toString(z.back().back().back())+"\t"+toString(temp.back().back().back())+"\t"+toString(temp2.back().back().back())+"\t"+timeStamp.back().back().back(),DEBUGSCAN);
					}
					(*g).acquisitionChannels.front().wait();
					if (acquisitionChannels.size()==1){
						getImageProperties(n+scanNum,p,0,fileName,comment);
						acquisitionChannels.front().out->cam->takePicture(&(acquisitionChannels.front()),workingDirectory+fileName);
					}else if((*g).acquisitionChannels.size()==1){
						Timer timer;
						(*g).acquisitionChannels.front().out->cam->t->triggerSingleLine(1);
						timer.wait((*g).acquisitionChannels.front().ap.exp*1000);
						(*g).acquisitionChannels.front().out->cam->t->triggerSingleLine(1);
					}else{
						//if ((*g).acquisitionChannels.front().out->cam->isTriggerable()){
							(*g).acquisitionChannels.front().chan->lite().ls->enterRingBuffer(*g);
							(*g).acquisitionChannels.front().out->cam->t->setWaveform((*g));
							(*g).acquisitionChannels.front().out->cam->t->prepareWaveform();
							(*g).acquisitionChannels.front().out->cam->t->generateWaveform();
						//}else{
						//	cout<<"Trigger not defined for acquisitionGroup channel numbers:\n"
						//		<<acquisitionSeries.front().cumNumChannels<<" to "<<acquisitionSeries.front().cumNumChannels+cnum<<endl;
						//}
					}
					(*g).acquisitionChannels.front().off();
					SetEvent(hStartScan);
					for(int c=0;c<(*g).acquisitionChannels.size();c++){
						getImageProperties(n+scanNum,p,c,fileName,comment);
						cont.te.comment="scan: "+toString(n+scanNum)+" pos: "+toString(p)+" "+fileName;
						cont.te.triggerRecord=true;
					}
				}
			}
		}
	}
	else if (acquisitionChannels.size() >= 1){
		for(int n=0;n<numScan;n++){
			vector<vector<int>> spotX,spotY;
			vector<vector<double>> spotZ,spotTemp,spotTemp2;
			vector<vector<string>> spotT;
			x.push_back(spotX);
			y.push_back(spotY);
			z.push_back(spotZ);
			temp.push_back(spotTemp);
			temp2.push_back(spotTemp2);
			timeStamp.push_back(spotT);
			for(int p=0;p<numSpots();p++){
				/////////////////////////////////////////////////
				move2NextSpot();
				if (sf!=NULL) cont.focus->move(sf->getFocus(cont.stg->getX(),cont.stg->getY()));
				//are x and y even there yet? this should be delegated to the move2NextSpot 
				/////////////////////////////////////////////////////////////
				int snum=0;
				vector<int> currentX, currentY;
				vector<double> currentTemp, currentTemp2, currentZ;
				vector<string> currentT;
				x.back().push_back(currentX);
				y.back().push_back(currentY);
				z.back().push_back(currentZ);
				temp.back().push_back(currentTemp);
				temp2.back().push_back(currentTemp2);
				timeStamp.back().push_back(currentT);
				for(vector<AcquisitionSeries>::iterator si=acquisitionSeries.begin();si!=acquisitionSeries.end();si++,snum++){
					WaitForSingleObject(hStartScan,180000);
					numImages=(*si).numChannels+(*si).acquisitionGroups.size();
					seriesNum=snum+p*acquisitionSeries.size()+(n+scanNum)*numSpots()*acquisitionSeries.size();
					saveSeriesThreads.push_back((*si).acquisitionGroups.front().acquisitionChannels.front().out->cam->startSeries(numImages,seriesNum,this));
					SetEvent(hStartScan);
					for(vector<AcquisitionGroup>::iterator g=(*si).acquisitionGroups.begin();g!=(*si).acquisitionGroups.end();g++){
						(*g).acquisitionChannels.front().on();
						cont.stg->wait();
						cont.focus->wait();//should wait for sf so we can support definite focus
						WaitForSingleObject(hStartScan,INFINITE);
						//Before we figure out how to get out the data for each channel, assign the final value for all AcquisitionChannels in this AcquisitionGroup
						int cnum=0;
						for(vector<AcquisitionChannel>::const_iterator c=(*g).acquisitionChannels.begin();c!=(*g).acquisitionChannels.end();c++,cnum++){
							x.back().back().push_back(cont.stg->getX());
							y.back().back().push_back(cont.stg->getY());
							z.back().back().push_back(cont.focus->getZ());
							temp.back().back().push_back(cont.te.getTemp());
							temp2.back().back().push_back(cont.te.getTemp2());
							timeStamp.back().back().push_back(Timer::getSysTime());
							scanLogFile->write(toString(x.back().back().back())+"\t"+toString(y.back().back().back())+"\t"+toString(z.back().back().back())+"\t"+toString(temp.back().back().back())+"\t"+toString(temp2.back().back().back())+"\t"+timeStamp.back().back().back(),DEBUGSCAN);
						}		
						SetEvent(hStartScan);
						(*g).acquisitionChannels.front().wait();
						if ((*g).acquisitionChannels.size()==1){
							Timer timer;
							(*g).acquisitionChannels.front().out->cam->t->triggerSingleLine(1);
							timer.wait((*g).acquisitionChannels.front().ap.exp*1000);
							(*g).acquisitionChannels.front().out->cam->t->triggerSingleLine(1);
						}else{
							if ((*g).acquisitionChannels.front().out->cam->t!=NULL){
								(*g).acquisitionChannels.front().out->cam->t->setWaveform((*g));
								(*g).acquisitionChannels.front().out->cam->t->prepareWaveform();
								(*g).acquisitionChannels.front().out->cam->t->generateWaveform();
							}else{
								cout<<"Trigger not defined for acquisitionGroup channel numbers:\n"
									<<acquisitionSeries.front().cumNumImages<<" to "<<acquisitionSeries.front().cumNumImages+cnum<<endl;
							}
						}
						(*g).acquisitionChannels.front().off();
						for(int c=0;c<(*g).acquisitionChannels.size();c++){
							getImageProperties(n+scanNum,p,c,fileName,comment);
							cont.te.comment="scan: "+toString(n+scanNum)+" pos: "+toString(p)+" "+fileName;
							cont.te.triggerRecord=true;
						}
					}
				}
			}
		}
	}
	if (acquisitionChannels.size() >= 1){
		acquisitionChannels.back().off();
		logFile.write(string("Scan completed in ")+t.toString(),true);
		cont.currentChannel()=acquisitionChannels.back();
	}
	*/
}

int Scan::numChannels(){
	return chanToSeries.size();
}

AcquisitionChannel Scan::getChan(int chan){
	if (chan>=chanToSeries.size() || chan<0){
		logFile.write("Scan getChan index out of bounds",true);
		return AcquisitionChannel();
	}
	return acquisitionSeries.at(chanToSeries.at(chan))->getChan(chan);//-acquisitionSeries.at(chanToSeries.at(chan))->cumNumChans);
}

//imageNum starts at 0 for the first image saved by the camera
bool Scan::getImageProperties(int imageNum,int seriesNum, ImageProperties &ip,int camNum){//return false if it is not to be saved (i.e. dummy acquisition)
	//NOTE: THE FIRST IMAGE FROM ANDOR External Triggering Kinetics is a DUMMY IMAGE but ANDOR ignores this image so imageNum=0 could be a real image (may be a pause though)
	string name,comment;
	int scanNum;
	int spotNum;
	if (acquisitionSeries.size()==1){
		//each scan, including all spots for that scan, is done as single series
		scanNum=seriesNum;
		seriesNum=0;
		spotNum=imageNum/(acquisitionSeries[0]->_imageNumToChanNum.size()+1);//numChannels+acquisitionSeries[0]->acquisitionGroups.size()-1+1);//-1 because there is one less dummy image than there are acquisition groups and +1 because we did a dummy scan at the end of each series (even if there was only one spot) mod total num channels+num dummies between channels+dummy for moving stage
		spotNum=spotNum%numSpotsGlobal();
		//if (numSpots()==1)//hack for single spot single channel time series acquisition
		//	spotNum=0;
		imageNum=imageNum%(acquisitionSeries[0]->_imageNumToChanNum.size()+1);//numChannels+acquisitionSeries[0]->acquisitionGroups.size());
	}else{
		//each spot has multiple series	
		scanNum=seriesNum/(numSpotsGlobal()*acquisitionSeries.size());
		spotNum=(seriesNum/acquisitionSeries.size())%numSpotsGlobal();//seriesNum%numSpots();
		seriesNum=seriesNum%acquisitionSeries.size();
	}
//	imageNum+=acquisitionSeries[seriesNum]->cumNumImages;
	int chanNum=acquisitionSeries[seriesNum]->imageNumToChanNum(imageNum);//->imageNumToChanNum.at(imageNum);//this should be all that is needed
	if (chanNum==-1)
		return false;
	getImageProperties(scanNum,spotNum,chanNum,name,comment,camNum);
	AcquisitionChannel a=getChan(chanNum);
	ip.Update(a,workingDirectory+name,comment,x[scanNum][spotNum][chanNum],y[scanNum][spotNum][chanNum],z[scanNum][spotNum][chanNum],temp[scanNum][spotNum][chanNum],temp2[scanNum][spotNum][chanNum],this->timeStamp[scanNum][spotNum][chanNum]);
	logFile.write("Saving "+name+" with the below properties",DEBUGSCAN);
	logFile.write(toString(scanNum)+toString(spotNum)+toString(chanNum)+"\t"+toString(x[scanNum][spotNum][chanNum])+"\t"+toString(y[scanNum][spotNum][chanNum])+"\t"+toString(z[scanNum][spotNum][chanNum])+"\t"+toString(temp[scanNum][spotNum][chanNum])+"\t"+toString(temp2[scanNum][spotNum][chanNum])+"\t"+timeStamp[scanNum][spotNum][chanNum]);
	return true;
}
