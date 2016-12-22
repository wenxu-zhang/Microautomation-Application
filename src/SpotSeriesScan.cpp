// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, April 04, 2012</lastedit>
// ===================================
#include "SpotSeriesScan.h"
#include "XYStage.h"
extern Controller cont;
using namespace std;

struct SpotSeriesArg{
	SpotSeriesScan* s;
	int num;
};

void SpotSeriesScan::abort(){
	SetEvent(abortEvent);
	WaitForSingleObject(h,INFINITE);

	Scan::abort();
}

void SpotSeriesScan::runScan(int numScan,bool wait){
	this->enableStartEvent();	
	if (this->sf && (times.size()>0 || period!=0)){
			Scan::runScan(numScan,false);
			SpotSeriesArg* arg=new SpotSeriesArg();
			arg->s=this;
			arg->num=numScan;
			unsigned int threadID;
			h=(HANDLE) _beginthreadex(NULL,0, &spotSeriesThread,arg,0,&threadID);
			cont.threads.addThread(h);
			if (wait){
				HANDLE hs[2]={abortEvent,h};
				DWORD val=WaitForMultipleObjects(2,hs,false,INFINITE);
				DWORD val2=WaitForSingleObject(abortEvent,0);
				if (val==WAIT_OBJECT_0 || val2==WAIT_OBJECT_0)
					return;
				CloseHandle(h);
				h=NULL;
			}
	}
	else Scan::runScan(numScan,wait);

}

unsigned __stdcall SpotSeriesScan::spotSeriesThread(void* param){
	SpotSeriesArg* r=(SpotSeriesArg*) param;

	/*	while(r->s->actualTimes.size()==0){//wait for first startEvent
		if (WAIT_OBJECT_0==WaitForSingleObject(r->s->abortEvent,0))
			return 0;
	}
*/

	if (r->s->times.size()>0){ 
		for(int j=1;j<r->s->times.size();j++){
			if (!r->s->Scan::waitGenerationComplete())
				return 0;
			if (!r->s->Scan::waitReadyForStartEvent())
				return 0; 
			r->s->t.waitTotal(r->s->times.at(j)*1000.0,r->s->abortEvent);
			r->s->sf->updateFocus(true);
			//r->s->sf->wait();
			for(int i=0;i<r->s->numGroups-1;i++){
				r->s->Scan::sendStartEvent();
				r->s->actualTimes.push_back(r->s->t.getTime()/1000.0);
				if (!r->s->Scan::waitGenerationComplete())
					return 0;
				if (!r->s->Scan::waitReadyForStartEvent())
					return 0; 
			}
			r->s->Scan::sendStartEvent();
			r->s->actualTimes.push_back(r->s->t.getTime()/1000.0);
		}
	}else{
		for(int j=1;j<r->s->numSpotsGlobal()*(r->s->numSpotsLocal()+1);j++){
			if (!r->s->Scan::waitGenerationComplete())
				return 0;
			if (!r->s->Scan::waitReadyForStartEvent())
				return 0;
			r->s->t.waitTotal(r->s->period*j*1000.0,r->s->abortEvent);
			r->s->sf->updateFocus(true);
			//r->s->sf->wait();
			r->s->Scan::sendStartEvent();
			r->s->actualTimes.push_back(r->s->t.getTime()/1000.0);
		}
	}

return 0;
	}

void SpotSeriesScan::sendStartEvent(){
		if (this->sf && !t.running){
			if (!Scan::waitReadyForStartEvent())
				return;
			sf->updateFocus(true);		
			t.startTimer();
			if (times.size()>0){
				t.waitTotal(times.at(0),abortEvent);
			}
			for(int i=0;i<numGroups-1;i++){
				Scan::sendStartEvent();
				actualTimes.push_back(t.getTime()/1000.0);
				if (!Scan::waitGenerationComplete())
					return ;
				if (!Scan::waitReadyForStartEvent())
					return ; 
			}
			Scan::sendStartEvent();
			actualTimes.push_back(t.getTime()/1000.0);
		}else
			if (!Scan::waitReadyForStartEvent())
				return;
			Scan::sendStartEvent();
}

void SpotSeriesScan::setDuration(double timeSecs){
		SpotSeriesScan ssc(getChan(0),"NOTSETYET",timeSecs);
		string workingDir=this->workingDirectory;
		*this=ssc;
		this->workingDirectory=workingDir;
	}

void SpotSeriesScan::getScanRegion(int& x1,int& y1,int &x2,int &y2){
		x1=x2=cont.stg->getX();
		y1=y2=cont.stg->getY();
	}

vector<AcquisitionChannel> SpotSeriesScan::createACvector(AcquisitionChannel ac,double totalTimeSec,double periodSec,ScanFocus* sf){
	if (sf && periodSec!=0) return vector<AcquisitionChannel>(1,ac);
	AcquisitionChannel pause=ac;
	pause.intensity=pause.chan->lite().ls->cp->zeroIntensity;
	vector<AcquisitionChannel> v;
	if (!ac.isTriggerable()) return v;
	if (periodSec==0){//as fast as possible
		v.reserve(totalTimeSec/ac.ap.exp);
		for(int i=0;i<totalTimeSec/ac.ap.exp;i++){
			v.push_back(ac);
		}
	}else{
		v.reserve(totalTimeSec/periodSec+1);
		pause.ap.exp=periodSec-ac.ap.exp;
		if (!pause.isTriggerable()){
			logFile.write(string("Error: cannot do kinetics series with an exposure time of ")+toString(ac.ap.exp)+"sec and a period of "+toString(periodSec)+"sec. Either extend the period, use binning, or reduce the image region."); 
			return vector<AcquisitionChannel>(1,ac);
		}
		v.push_back(ac);
		for(int i=0;i<totalTimeSec/periodSec;i++){
			v.push_back(pause);
			v.push_back(ac);
		}
	}
	return v;
}

vector<AcquisitionChannel> SpotSeriesScan::createACvector(AcquisitionChannel ac,vector<double> times,ScanFocus* sf){
	if (sf) return vector<AcquisitionChannel>(1,ac);
	AcquisitionChannel pause=ac;
	pause.intensity=pause.chan->lite().ls->cp->zeroIntensity;
	vector<AcquisitionChannel> v;
	double totalTime=0;
	if (!ac.isTriggerable()) return v;
	if (times.at(0)==0){
		v.push_back(ac);
		totalTime+=ac.ap.exp;
	}else{
		pause.ap.exp=times.at(0);
		v.push_back(pause);
		v.push_back(ac);
		totalTime+=pause.ap.exp+ac.ap.exp;
	}
	
	for(vector<double>::iterator i=times.begin()+1;i!=times.end();i++){
		pause.ap.exp=*i-totalTime;
		if (pause.ap.exp!=0){
			if (ac.out->cam->isTriggerable(pause.ap)) 
				v.push_back(pause);
			else{
				logFile.write("Error Spot Series Scan: required pause is too short, setting pause to zero",true);
				pause.ap.exp=0;
			}
		}
		v.push_back(ac);
		totalTime+=pause.ap.exp+ac.ap.exp;
	}
	
	return v;
}


void SpotSeriesScan::move2NextSpotGlobal(ScanFocus* sf,int spotNum){
	//definite focus update is called by timing thread
	//if (sf && spotNum!=0){
	//	sf->updateFocus();
	//}

	(*globalFunc)(spotNum);
		if (newFOV){
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
	}
}

void SpotSeriesScan::move2NextSpotLocal(ScanFocus* sf,int spotNum){
	//definite focus update is called by timing thread
	//if (sf && spotNum!=0){
	//	sf->updateFocus();
	//}


}

int SpotSeriesScan::numSpotsGlobal(){
	if (sf && numSpotsLocal()==0){
		if (times.size()>0) return times.size();
		else if (period!=0)
			return totalTime/period+1;
		else 
			return 1;
	}
	else
		return 1;

}

void SpotSeriesScan::getImageProperties(int scanNum,int spotNum,int chanNum, string& fileName,string& comment,int camNum){
	//spotNum will always be 0
	//chanNum will be the timepoint for this time series
	AcquisitionChannel a=getChan(chanNum);
	AcquisitionChannel* i=&a;
	fileName=i->chan->toString(i->out,camNum)+"_m"+i->m->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain(camNum))+"i"+i->chan->lite().ls->intensityToString(i->intensity)+"a"+::toString(i->TIRFangle,1)+string("Scan")+toString(scanNum)+"T"+toString(spotNum*numChannels()/numGroups+int(chanNum/numGroups),4);
	comment="Single spot time series scan";
}

SpotSeriesScan::SpotSeriesScan(AcquisitionChannel ac, string workingDir,double timeSecs,double period,ScanFocus* sf):Scan(createACvector(ac,timeSecs,period,sf),workingDir,sf),period(period),totalTime(timeSecs),globalFunc(nothing),numGroups(1){}


vector<AcquisitionChannel> custFunct(AcquisitionChannel ac, vector<double> timeSecsFirst, vector<double> timeSecsSecond, ScanFocus* sf){
	if (!sf){
		timeSecsFirst.insert(timeSecsFirst.end(),timeSecsSecond.begin(),timeSecsSecond.end());	
		return SpotSeriesScan::createACvector(ac,timeSecsFirst,sf);
	}
	return SpotSeriesScan::createACvector(ac,timeSecsFirst);
}

SpotSeriesScan::SpotSeriesScan(AcquisitionChannel ac, string workingDir,vector<double> timeSecsFirst,vector<double> timeSecsSecond,ScanFocus* sf):Scan(custFunct(ac,timeSecsFirst,timeSecsSecond,sf),workingDir,sf),globalFunc(nothing),numGroups(1){
	if (!sf)
		times=timeSecsFirst;
	else
		times.push_back(timeSecsFirst.front());
	times.insert(times.end(),timeSecsSecond.begin(),timeSecsSecond.end());
	if (sf)
		addAcquisitionGroup(createACvector(ac,timeSecsSecond,sf),timeSecsSecond.size());
}


SpotSeriesScan::SpotSeriesScan(AcquisitionChannel ac, string workingDir,vector<double> timeSecs,ScanFocus* sf, bool newFOV, int direction,double FOVspacing,void (*f)(int)):Scan(createACvector(ac,timeSecs,sf),workingDir,sf),times(timeSecs),newFOV(newFOV),direction(direction),FOVspacing(FOVspacing),globalFunc(f),numGroups(1){}

SpotSeriesScan::SpotSeriesScan(vector<AcquisitionChannel> ac, string workingDir,vector<double> timeSecs,ScanFocus* sf, bool newFOV, int direction,double FOVspacing,void (*f)(int)):Scan(ac,workingDir,sf),times(timeSecs),newFOV(newFOV),direction(direction),FOVspacing(FOVspacing),globalFunc(f),numGroups(0){
	for(vector<AcquisitionSeries*>::iterator si=acquisitionSeries.begin();si!=acquisitionSeries.end();si++){
		numGroups+=(*si)->acquisitionGroups.size();
	}
}


SpotSeriesScan::~SpotSeriesScan(){
	WaitForSingleObject(h,INFINITE);
	wait();
	if (!actualTimes.empty()){
	if (this->times.size()>0){
		scanLogFile->write("Original times:",true);
		scanLogFile->write(string("           ")+toString(times),true);
		scanLogFile->write("Actual times:",true);
		scanLogFile->write(string("           ")+toString(actualTimes),true);
	}else{
		scanLogFile->write(string("Timer Period: ")+toString(period)+"  Total Time: "+toString(totalTime),true);
		scanLogFile->write("Actual times:",true);
		scanLogFile->write(string("           ")+toString(actualTimes),true);
	}
	}
}