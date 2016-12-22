// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Thursday, November 17, 2011</created>
// <lastedit>Saturday, April 07, 2012</lastedit>
// ===================================
#include "Reagent.h"
#include "FlowChannel.h"
#include "Controller.h"
#include <sstream>
using namespace std;
extern customProtocol scan;
Reagent::Reagent(SolutionData& sd):Solution(sd){
	if (sd.wash==NULL || sd.air==NULL)
		throw exception(string("Reagent "+sd.name+" creation error: must have a valid wash and air solution").c_str());
	if (sd.dv.v1!=sd.wash->sd.dv.v1 || sd.dv.v2!=sd.wash->sd.dv.v2)
		throw exception(string("Reagent "+sd.name+" creation error: reagent daisy valve and wash solution daisy valve must be the same").c_str());
}

FlowChannel* PullReagent::prime(FlowChannel* chan){
	if (chan==NULL)
		chan=scan.fs.selectChannel();
	if (chan==NULL){
		logFile.write("No Channel selected, reagent was not primed",true);
		return NULL;
	}
	valveSelect();
	//leave air gap
	chan->pull(this->sd.inletTubingVol-1.2*this->sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	this->sd.wash->load(chan);
	chan->waste();
	return chan;
}

FlowChannel* PushReagent::prime(FlowChannel* chan){
	//if (sd.wash->sd.valveNum==-1){//wash is on syringe not daisy valve, use flowchannel for priming
	//	if (chan==NULL)
	//		chan=scan.fs.selectChannel();
	//	if (chan==NULL){
	//		logFile.write("No Channel selected, reagent was not primed",true);
	//		return NULL;
	//	}
	//	valveSelect();
	//	sd.s->pull(sd.syringePort,this->sd.inletTubingVol-this->sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//	this->sd.wash->load(chan);
	//}else{//no channel necessary
		valveSelect();
		
		if (sd.wash->sd.valveNum!=-1){//wash on daisyvalve
			sd.s->pull(sd.syringePort,this->sd.inletTubingVol-this->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume(),sd.uLps);
			this->sd.wash->valveSelect();
			sd.s->pull(sd.syringePort,1.2*(this->sd.wash->sd.dv.getInjectTubingVolPull(this->sd.wash->sd.valveNum)+this->sd.wash->sd.dv.injectTubingVol),sd.wash->sd.uLps);
			sd.s->waste();
		}else{//wash on syringe, use waste
			sd.s->waste();
			double vol=sd.inletTubingVol-sd.air->sd.loadFactor*scan.cham.getChannelVolume();
			pull(vol);
			double washVol=1.5*(sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)-sd.dv.getInjectTubingVolPull(sd.valveNum));
			if (washVol==0)
				washVol=sd.inletTubingVol;
			sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5);
			sd.waste->push(washVol+vol,NULL,sd.wash->sd.uLps*5.0);
			sd.s->waste();
/*
			sd.s->pull(sd.wash->sd.syringePort,1.2*(this->sd.inletTubingVol-this->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume()),sd.wash->sd.uLps*5.0);
			sd.s->pull(sd.syringePort,this->sd.inletTubingVol-this->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume(),sd.uLps);
			sd.waste->valveSelect();
			sd.s->push(sd.waste->sd.syringePort,2.2*(this->sd.waste->sd.dv.getInjectTubingVolPull(this->sd.wash->sd.valveNum)+this->sd.wash->sd.dv.injectTubingVol),sd.wash->sd.uLps);
		*/
		}
	//}
	return chan;
}

//param is fractional position in chamber;
void PushReagent::load(FlowChannel* chan,double param, double time){
	if (sd.remainingLoadVolume>0){
		if (chan==NULL)
			chan=sd.chan;
		else if (chan!=sd.chan)
			throw exception(string("Load Completion called on a different channel that what was previous loaded").c_str());
	}
	if (chan==NULL)
		throw exception(string("No Channel specified for load function").c_str());
		
	if (param!=-1 && (param<0 || param>1)){
		throw exception(string("Cannot load "+toString()+" Invalid fractional channel position."+::toString(param)).c_str());
		return;
	}
	if (time!=-1 && time<0){
		throw exception(string("Cannot load "+toString()+" Negative time parameter. Traveling back in time not supported yet!"+::toString(param)).c_str());
		return;
	}
	if (sd.s!=chan->s){
		throw exception(string("Cannot load "+toString()+" Channel Syringe and DaisyValve Syringe must be the same.").c_str());
		return;
	}
	if (sd.syringePort!=chan->syringePos){
		throw exception(string("Cannot load "+toString()+" Channel Syringe Port and DaisyValve Syringe Port must be the same.").c_str());
		return;
	}
	if (param!=-1 && sd.remainingLoadVolume!=0){
		throw exception(string("Cannot perform another fractional load until previous fractional load is completed.").c_str());
		return;
	}
	if (sd.remainingLoadVolume!=0){
		//complete load
		sd.s->pmp->executeCommand(sd.s->devNum);
		if (time==-1){
			sd.remainingLoadVolume=0;
			sd.chan=NULL;
			return;//exit as quickly as possible to the caller
		}
	}else{
	chan->waste();
	}
	int start=this->sd.s->getPosition();
	
	double pushVol=this->getInjectTubingVolume()+chan->getInjectTubingVolPush()+sd.air->sd.loadFactor*chan->cham.getChannelVolume()+0.5*sd.loadFactor*chan->cham.getChannelVolume();
	double minWash=pushVol-(2*sd.air->sd.loadFactor*chan->cham.getChannelVolume()+sd.loadFactor*chan->cham.getChannelVolume());

	double buffer=sd.air->sd.loadFactor*chan->cham.getChannelVolume()+0.5*sd.loadFactor*chan->cham.getChannelVolume();//just a buffer in case the loading is off and we need to adjust it with finaControl
	double extraWash=buffer;

	if (time!=-1 || param!=-1){//we may need extra wash if this is a partial load depending on the next call for load completion so just pull the extra amount just in case
		extraWash+=sd.wash->sd.loadFactor*chan->cham.getChannelVolume();
	}
	if (sd.remainingLoadVolume!=0){
		if (time!=-1){
			sd.s->wait();
			Timer::wait(time*1000.0);
			sd.s->push(sd.syringePort,sd.s->steps2vol(sd.s->getPosition()),sd.wash->sd.uLps);
			sd.s->waste();
		}
		sd.remainingLoadVolume=0;
		sd.chan=NULL;
		return;
	}

	if (extraWash+minWash+3*sd.air->sd.loadFactor*chan->cham.getChannelVolume()+sd.loadFactor*chan->cham.getChannelVolume()>sd.s->steps2vol(sd.s->maxStep-start)){//pulling extra air gap to push out reagent tubing
		extraWash=sd.s->steps2vol(sd.s->maxStep-start)-(minWash+3*sd.air->sd.loadFactor*chan->cham.getChannelVolume()+sd.loadFactor*chan->cham.getChannelVolume());
		logFile.write("Reagent load: syringe not large enough to hold entire wash solution. Using maximum wash of "+::toString(extraWash-buffer)+" uL",true);
	}
	
	sd.remainingLoadVolume=0;
	if (param!=-1)
		sd.remainingLoadVolume=0.5*sd.loadFactor*chan->cham.getChannelVolume()+(0.5-param)*chan->cham.getChannelVolume();
	//pull in enough wash solution to reach the chamber plus some extra to wash with to stop reaction, reagent already has air gap in front
	
	chan->select();
	valveSelect();
	sd.wash->pull(minWash+extraWash,NULL,sd.wash->sd.uLps*5.0);
	//pull in reagent, reagent already has air gap in front
	pull(1*sd.air->sd.loadFactor*chan->cham.getChannelVolume()+sd.loadFactor*chan->cham.getChannelVolume(),NULL,sd.uLps);
	//sd.s->push(sd.syringePort,0.5*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//pull twice the volume of the air gap
	sd.air->valveSelect();
	sd.s->pull(sd.syringePort,2*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//sd.s->push(sd.syringePort,0.5*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//push air back out reagent port
	valveSelect();
	sd.s->push(sd.syringePort,2*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	sd.s->pull(sd.syringePort,sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//push reagent into chamber, flow channel tubing volume should account for any backlash
	chan->push(pushVol-sd.remainingLoadVolume,sd.uLps);
	if (param==-1 && time!=-1){
		Timer::wait(time*1000.0);
		sd.s->push(sd.syringePort,extraWash,sd.wash->sd.uLps,true,0);
	}
	if (sd.remainingLoadVolume>0){
		//prepare for load completion which should be the next command;
		sd.s->pmp->sendCommandNoExecute(sd.s->_push(sd.syringePort,sd.remainingLoadVolume,sd.uLps,0),sd.s->devNum);
		sd.chan=chan;
	}
	
	//move reagent into the chamber, followed by air
//switch to closest air port
//	v->getInjectTubingVol();
//	chan->pull(injectTubingVol,uLps);
}

void PushReagent::coarseCalibrateInlet(){
	cout<<"Pull reagent line out of tube to pull in air (enter to continue)"<<endl;
	getString();
	//pull some air
	valveSelect();
	sd.s->pull(sd.syringePort,2*sd.air->sd.loadFactor*scan.cham.getChannelVolume(),sd.uLps);
	cout<<"Put reagent line in tube with solution (enter to continue)"<<endl;
	getString();
	cout<<"Pull reagent up to valve port and exit when done"<<endl;
	double vol=sd.s->fineControl(sd.syringePort,sd.uLps);
	logFile.write("Coarse Inlet Volume for "+toString()+" is "+::toString(vol),true);
}

void PushReagent::calibrateInlet(){
	if (sd.wash->sd.valveNum!=-1){//wash on daisy valve
		cout<<"Pull reagent line out of tube to pull in air (enter to continue)"<<endl;
		getString();
		//empty reagent inlet tubing 
		valveSelect();
		sd.s->pull(sd.syringePort,1.5*sd.inletTubingVol,sd.uLps);
		cout<<"Put reagent line in tube with solution (enter to continue)"<<endl;
		getString();
		//remove air from tubing
		sd.wash->valveSelect();
		sd.wash->sd.s->pull(sd.wash->sd.syringePort,sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+1.5*sd.wash->sd.dv.injectTubingVol,sd.wash->sd.uLps);
		sd.s->waste();
		//pull solution fixed inlet volume
		valveSelect();
		sd.s->pull(sd.syringePort,sd.inletTubingVol-sd.air->sd.loadFactor*scan.cham.getChannelVolume(),sd.uLps);
		//remove air from tubing
		sd.wash->valveSelect();
		sd.wash->sd.s->pull(sd.wash->sd.syringePort,sd.wash->sd.dv.getInjectTubingVolPull(sd.valveNum)+1.5*sd.wash->sd.dv.injectTubingVol,sd.wash->sd.uLps);
		sd.s->waste();
		//pull air gap into injection loop tubing for measurement
		valveSelect();
		sd.s->pull(sd.syringePort,sd.dv.getInjectTubingVolPull(sd.valveNum)+0.5*sd.dv.injectTubingVol+0.5*sd.air->sd.loadFactor*scan.cham.getChannelVolume(),sd.uLps);
		double airvol=sd.air->sd.loadFactor*scan.cham.getChannelVolume();
		double airlength=::tubingLengthmm(scan.tubingDiameterInches,airvol);
		cout<<"Measure and input air gap length in mm. Should be "<<::toString(airlength)<<" mm ("<<::toString(airvol)<<" uL )"<<endl;
		double d=getDouble();
		logFile.write(toString()+ "coarse inlet tubing volume was "+::toString(sd.inletTubingVol)+" uL. Calibrated volume is "+::toString(sd.inletTubingVol+(d-airlength)*airvol/airlength)+" uL",true);
	}else{//wash on syringe
		double speedFactor=1.0;
		sd.s->waste();
		double vol,washVol;
		washVol=1.5*(sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)-sd.dv.getInjectTubingVolPull(sd.valveNum));
		if (washVol==0)
			washVol=sd.inletTubingVol;
		//put 1.5chambervolume into tube
		vol=1.5*sd.loadFactor*scan.cham.getChannelVolume();
		sd.wash->pull(vol,NULL,sd.wash->sd.uLps*5.0);
		push(vol);

		//put air into tube and clean daisy tubing
		vol=sd.inletTubingVol;
		sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5.0);
		sd.air->pull(2.0*vol);
		push(1.5*vol);
		sd.waste->push(-1,NULL,sd.wash->sd.uLps*5.0);

		//pull fixed inlet volume and waste extra
		/*
		vol=sd.inletTubingVol-sd.air->sd.loadFactor*scan.cham.getChannelVolume();
		sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5.0);
		pull(vol);
		sd.waste->push(washVol+vol,NULL,sd.wash->sd.uLps*5.0);
		*/
		cout<<"Clean tip of "<<toString()<< ", centrifuge tube and reinsert tubing before priming. An air bubble at the bottom of the tube will ruin calibration (press enter to continue)"<<endl;
		getString();
		prime(NULL);

		//pull air gap into injection loop for measurement
		vol=sd.dv.getInjectTubingVolPull(sd.valveNum)+0.5*sd.dv.injectTubingVol+0.5*sd.air->sd.loadFactor*scan.cham.getChannelVolume();
		pull(vol);

		double airvol=sd.air->sd.loadFactor*scan.cham.getChannelVolume();
		double airlength=::tubingLengthmm(scan.tubingDiameterInches,airvol);
		cout<<"Measure and input air gap length in mm. Should be "<<::toString(airlength)<<" mm ("<<::toString(airvol)<<" uL )"<<endl;
		double d=getDouble();
		logFile.write(toString()+ "coarse inlet tubing volume was "+::toString(sd.inletTubingVol)+" uL. Calibrated volume is "+::toString(sd.inletTubingVol+(d-airlength)*airvol/airlength)+" uL",true);
		logFile.write("cleaning up reagent tubing",true);
		//empty reagent tube and clean daisy tubing
		sd.waste->push(-1,NULL,sd.wash->sd.uLps*5.0);//push all the way out waste
		vol=sd.inletTubingVol;
		pull(vol,NULL,sd.wash->sd.uLps*5.0);
		sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5.0);
		sd.waste->push(washVol+vol,NULL,sd.wash->sd.uLps*5.0);
		sd.s->waste();
	}
}

void PushReagent::calibrateTubingDiameter(){
//	remove air in line
	sd.wash->pull(1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol));
	sd.wash->sd.s->waste();
	//pull air gap
	sd.air->pull(sd.air->sd.loadFactor*scan.cham.getChannelVolume());
	sd.wash->valveSelect();
	//pull wash a total of daisytubing volume plus half inject volume to syringe
	sd.wash->sd.s->pull(sd.wash->sd.syringePort,sd.wash->sd.dv.getInjectTubingVolPull(sd.air->sd.valveNum)+0.5*sd.wash->sd.dv.injectTubingVol-0.5*sd.air->sd.loadFactor*scan.cham.getChannelVolume(),sd.air->sd.uLps);
	double airvol=sd.air->sd.loadFactor*scan.cham.getChannelVolume();
	double airlength=::tubingLengthmm(scan.tubingDiameterInches,airvol);
	cout<<"Enter length of air gap in mm. Should be "<<::toString(airlength)<<" mm ("<<::toString(airvol)<<" uL) corresponding to ID of "+::toString(scan.tubingDiameterInches,3)+"\""<<endl;
	double d=getDouble();
	logFile.write("Calculated inside diameter of tubing is "+::toString(sqrt(4*airvol/d/M_PI)/25.4,3)+"\"",true);
}

void PushReagent::calibrateDV(){//daisyInjectTubingVolume
	if (sd.wash->sd.valveNum!=-1){
		Solution* air1,*air2;
		if (sd.wash->sd.valveNum!=2 || (sd.air->sd.valveNum!=1 && sd.air->sd.valveNum!=2)){
			logFile.write("Cannot calibrate daisy valve with a reagent whose wash is not on valve #2. Select a different Reagent",true);
			return;
		}
		if (sd.air->sd.valveNum==1){
			air1=sd.air;
			if ((air2=scan.fs.getSolution("PushAir2"))==NULL){
				logFile.write("Cannot find air solution on valve 2. Cannot calibrate daisy valve",true);
				return;
			}
		}else {
			air2=sd.air;
			if ((air1=scan.fs.getSolution("PushAir"))==NULL){
				logFile.write("Cannot find air solution on valve 1. Cannot calibrate daisy valve",true);
				return;
			}
		}
		PushSolution* pair1=dynamic_cast<PushSolution*>(air1);
		PushSolution* pair2=dynamic_cast<PushSolution*>(air2);
		PushSolution* wash=dynamic_cast<PushSolution*>(sd.wash);
		//pull wash into tubing to remove air
		wash->pull(1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol));
		wash->sd.s->waste();
		//pull air gap
		pair2->pull(air2->sd.loadFactor*scan.cham.getChannelVolume());
		sd.wash->valveSelect();
		//pull wash a total of daisytubing volume plus an air gap volume
		sd.wash->sd.s->pull(sd.wash->sd.syringePort,sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+air2->sd.loadFactor*scan.cham.getChannelVolume(),air2->sd.uLps);
		Timer::wait(1000);
		//pull air half the inject volume to syringe minus half and air gap 
		pair1->pull(0.5*sd.wash->sd.dv.injectTubingVol-0.5*air2->sd.loadFactor*scan.cham.getChannelVolume());
		//solution plug should be an air gap wide and we need to center on the tubing
		cout<<"Center wash solution plug in inject volume between syringe and daisy valve"<<endl;
		pair1->sd.s->fineControl(pair1->sd.syringePort,pair1->sd.uLps);
		double airvol=air2->sd.loadFactor*scan.cham.getChannelVolume();
		double airlength=::tubingLengthmm(scan.tubingDiameterInches,airvol);
		cout<<"Enter length of solution plug between air gaps in mm. Should be "<<::toString(airlength)<<" mm ("<<::toString(airvol)<<" uL)"<<endl;
		double d=getDouble();
		logFile.write(toString()+ " coarse daisy injection tubing volume was "+::toString(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum))+" uL. Calibrated volume is "+::toString(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)-(d-airlength)*airvol/airlength)+" uL",true);
	}else{
		Solution* air1,*air2;
		if (sd.air->sd.valveNum==1){
			air1=sd.air;
			if ((air2=scan.fs.getSolution("PushAir2"))==NULL){
				logFile.write("Cannot find air solution on valve 2. Cannot calibrate daisy valve",true);
				return;
			}
		}else {
			air2=sd.air;
			if ((air1=scan.fs.getSolution("PushAir"))==NULL){
				logFile.write("Cannot find air solution on valve 1. Cannot calibrate daisy valve",true);
				return;
			}
		}
		PushSolution* pair1=dynamic_cast<PushSolution*>(air1);
		PushSolution* pair2=dynamic_cast<PushSolution*>(air2);
		PushSolution* wash=dynamic_cast<PushSolution*>(sd.wash);
		sd.s->waste();
		//push wash into tubing to remove air
		double vol=1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)+sd.waste->sd.dv.injectTubingVol)+air2->sd.loadFactor*scan.cham.getChannelVolume()+sd.waste->sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)+air2->sd.loadFactor*scan.cham.getChannelVolume()+0.5*sd.waste->sd.dv.injectTubingVol-0.5*air2->sd.loadFactor*scan.cham.getChannelVolume();
		sd.s->pull(sd.wash->sd.syringePort,vol,sd.wash->sd.uLps*5);
		sd.waste->valveSelect();
		sd.s->push(sd.waste->sd.syringePort,vol,sd.waste->sd.uLps);
		sd.s->waste();
		//pull air gap
		pair2->pull(air2->sd.loadFactor*scan.cham.getChannelVolume());
		sd.waste->valveSelect();
		//pull wash a total of daisytubing volume plus an air gap volume
		sd.waste->sd.s->pull(sd.waste->sd.syringePort,sd.waste->sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)+air2->sd.loadFactor*scan.cham.getChannelVolume(),air2->sd.uLps);
		Timer::wait(1000);
		//pull air half the inject volume to syringe minus half and air gap 
		pair1->pull(0.5*sd.wash->sd.dv.injectTubingVol-0.5*air2->sd.loadFactor*scan.cham.getChannelVolume());
		//solution plug should be an air gap wide and we need to center on the tubing
		//cout<<"Center wash solution plug in inject volume between syringe and daisy valve"<<endl;
		//pair1->sd.s->fineControl(pair1->sd.syringePort,pair1->sd.uLps);
		double airvol=air2->sd.loadFactor*scan.cham.getChannelVolume();
		double airlength=::tubingLengthmm(scan.tubingDiameterInches,airvol);
		cout<<"Enter length of solution plug between air gaps in mm. Should be "<<::toString(airlength)<<" mm ("<<::toString(airvol)<<" uL)"<<endl;
		double d=getDouble();
		logFile.write(toString()+ " coarse daisy injection tubing volume was "+::toString(sd.waste->sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum))+" uL. Calibrated volume is "+::toString(sd.waste->sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)-(d-airlength)*airvol/airlength)+" uL",true);
		logFile.write("cleaning up daisy tubing",true);
		sd.waste->push();
		sd.wash->pull(vol);
		sd.waste->push();
		sd.s->waste();
	}
}

void PushReagent::coarseCalibrateDV(){
	if (sd.wash->sd.valveNum!=-1){
		if (sd.wash->sd.valveNum!=2 || sd.air->sd.valveNum!=2){
			logFile.write("Cannot coarse calibrate daisy valve with a reagent whose wash and air are not on valve #2. Select a different Reagent",true);
			return;
		}
		sd.wash->valveSelect();
		cout<<"Pull wash solution into syringe to remove any air in daisy tubing (exit when done)"<<endl;
		sd.wash->sd.s->fineControl(sd.wash->sd.syringePort,sd.wash->sd.uLps);
		sd.s->waste();
		sd.air->valveSelect();
		cout<<"Pull air to fill daisy inject tubing (exit when done"<<endl;
		double vol=	sd.air->sd.s->fineControl(sd.air->sd.syringePort,sd.air->sd.uLps);
		logFile.write("Coarse daisy injection tubing volume was measured as "+::toString(vol)+" uL.",true);
	}else{
		if (sd.air->sd.valveNum!=2){
			logFile.write("Cannot coarse calibrate daisy valve with a reagent whoseair are not on valve #2. Select a different Reagent",true);
			return;
		}
		sd.s->pull(sd.wash->sd.syringePort,sd.s->volume,sd.waste->sd.uLps);
		sd.waste->valveSelect();
		cout<<"Push wash solution into waste to remove any air in daisy tubing (exit when done)"<<endl;
		sd.waste->sd.s->fineControl(sd.waste->sd.syringePort,sd.waste->sd.uLps);
		sd.s->waste();
		sd.air->valveSelect();
		cout<<"Pull air to fill daisy inject tubing (exit when done)"<<endl;
		double vol=	sd.air->sd.s->fineControl(sd.air->sd.syringePort,sd.air->sd.uLps);
		logFile.write("Coarse daisy injection tubing volume was measured as "+::toString(vol)+" uL.",true);

	}
}

void PushReagent::calibrateDV2(){//injectTubingVolume
	logFile.write("Fine calibration of daisy valve inject volume is not supported yet",true);
}
void PushReagent::coarseCalibrateDV2(){
	if (sd.wash->sd.valveNum!=-1){
		cout<<"Pull wash solution up to syringe to remove air (exit when done)"<<endl;
		sd.wash->valveSelect();
		sd.wash->sd.s->fineControl(sd.wash->sd.syringePort,sd.wash->sd.uLps);
		sd.wash->sd.s->waste();
		Solution* air1=scan.fs.getSolution("PushAir");
		if (air1==NULL){
			logFile.write("Could not find air on valve 1 of push daisy valve. Cannot calibrate daisy valve inject volume",true);
			return;
		}
		cout<<"Pull air up to port on syringe (exit when done)"<<endl;
		air1->valveSelect();
		double vol=air1->sd.s->fineControl(air1->sd.syringePort,air1->sd.uLps);
		logFile.write("Coarse Push Daisy Valve injection tubing volume is "+::toString(vol)+" uL",true);
	}else{
		sd.s->pull(sd.wash->sd.syringePort,sd.s->volume,sd.wash->sd.uLps*5);
		sd.waste->valveSelect();
		cout<<"Push wash solution into waste to remove air (exit when done)"<<endl;
		sd.waste->valveSelect();
		sd.waste->sd.s->fineControl(sd.waste->sd.syringePort,sd.waste->sd.uLps);
		sd.wash->sd.s->waste();
		Solution* air1=scan.fs.getSolution("PushAir");
		if (air1==NULL){
			logFile.write("Could not find air on valve 1 of push daisy valve. Cannot calibrate daisy valve inject volume",true);
			return;
		}
		cout<<"Pull air up to port on syringe (exit when done)"<<endl;
		air1->valveSelect();
		double vol=air1->sd.s->fineControl(air1->sd.syringePort,air1->sd.uLps);
		logFile.write("Coarse Push Daisy Valve injection tubing volume is "+::toString(vol)+" uL",true);
	}
}

//old version
void PushReagent::calibrateFlowChannel(FlowChannel* chan){
	if (chan==NULL){
		logFile.write("A valid flow channel is required for calibration",true);
		return;
	}
	if (sd.s!=chan->s){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe and DaisyValve Syringe must be the same.",true);
		return;
	}
	if (sd.syringePort!=chan->syringePos){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe Port and DaisyValve Syringe Port must be the same.",true);
		return;
	}
	
	chan->waste();
	chan->select();
	//remove air from tubing
	sd.wash->pull(1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol));
	chan->waste();
	//pull in enough wash solution to reach the chamber
	sd.wash->pull(1.5*(this->getInjectTubingVolume()+chan->getInjectTubingVolPush()));
	//pull in air gap
	sd.air->pull(sd.air->sd.loadFactor*chan->cham.getChannelVolume());
	//push air gap up to the entrance of the chamber
	chan->push(this->getInjectTubingVolume()+chan->getInjectTubingVolPush()+sd.air->sd.loadFactor*chan->cham.getChannelVolume()-0.5*chan->cham.getChannelVolume(),sd.uLps);
	cout<<"Push start of solution to the center of the channel (exit when done)"<<endl;
	double vol=chan->s->fineControl(chan->syringePos,sd.uLps);
	logFile.write(chan->toString()+" push injection tubing volume is "+::toString(chan->getInjectTubingVolPush()+vol-0.5*chan->cham.getChannelVolume())+" uL",true);
}

//new version that uses two air gaps, looks like it must be primed, so we shall prime it
void PushReagent::calibrateFlowChannel2(FlowChannel* chan){
	if (chan==NULL){
		logFile.write("A valid flow channel is required for calibration",true);
		return;
	}
	if (sd.s!=chan->s){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe and DaisyValve Syringe must be the same.",true);
		return;
	}
	if (sd.syringePort!=chan->syringePos){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe Port and DaisyValve Syringe Port must be the same.",true);
		return;
	}
	sd.s->waste();
	if (sd.wash->sd.valveNum==-1){
		//from calibrateInletTubingVolume
		double vol,washVol;
		washVol=1.5*(sd.dv.getInjectTubingVolPull(sd.waste->sd.valveNum)-sd.dv.getInjectTubingVolPull(sd.valveNum));
		if (washVol==0)
			washVol=sd.inletTubingVol;
		//put 1.5chambervolume into tube
		vol=1.5*sd.loadFactor*scan.cham.getChannelVolume();
		sd.wash->pull(vol,NULL,sd.wash->sd.uLps*5.0);
		push(vol);

		//put air into tube and clean daisy tubing
		vol=sd.inletTubingVol;
		sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5.0);
		sd.air->pull(2.0*vol);
		push(1.5*vol);
		sd.waste->push(-1,NULL,sd.wash->sd.uLps*5.0);

		//pull fixed inlet volume and waste extra
		/*
		vol=sd.inletTubingVol-sd.air->sd.loadFactor*scan.cham.getChannelVolume();
		sd.wash->pull(washVol,NULL,sd.wash->sd.uLps*5.0);
		pull(vol);
		sd.waste->push(washVol+vol,NULL,sd.wash->sd.uLps*5.0);
		*/
		cout<<"Clean tip of "<<toString()<< ", centrifuge tube and reinsert tubing before priming. An air bubble at the bottom of the tube will ruin calibration (press enter to continue)"<<endl;
		getString();
		prime(NULL);
	}
	this->load(chan,.5);
	sd.s->wait();

	cout<<"Enter distance in mm that the reagent has reached in the channel (should be "<<chan->cham.channelLength*0.5<<"/"<<chan->cham.channelLength<<" mm)"<<endl;
	double d=getDouble();
	double vol=chan->getInjectTubingVolPush()+chan->cham.getChannelVolume()*(chan->cham.channelLength*.5-d)/chan->cham.channelLength;
	logFile.write(chan->toString()+" push injection tubing volume was "+::toString(chan->getInjectTubingVolPush())+" uL. Calibrated volume is "+::toString(vol)+" uL",true);
}


void PushReagent::coarseCalibrateFlowChannel(FlowChannel* chan){
	if (chan==NULL){
		logFile.write("A valid flow channel is required for calibration",true);
		return;
	}
	if (sd.s!=chan->s){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe and DaisyValve Syringe must be the same.",true);
		return;
	}
	if (sd.syringePort!=chan->syringePos){
		logFile.write("Cannot calibrate "+chan->toString()+"with "+toString()+". Channel Syringe Port and DaisyValve Syringe Port must be the same.",true);
		return;
	}
	if (sd.wash->sd.valveNum!=-1){
		chan->waste();
		chan->select();
		//remove air from tubing
		sd.wash->pull(1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol));
		chan->waste();
		//pull in enough wash solution to reach the chamber
		sd.wash->pull(chan->s->volume-2*sd.air->sd.loadFactor*chan->cham.getChannelVolume());//1.5*(this->getInjectTubingVolume()+chan->getInjectTubingVolPush()));
		//pull in air gap
		sd.air->pull(sd.air->sd.loadFactor*chan->cham.getChannelVolume());
		chan->push(this->getInjectTubingVolume()+sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.wash->sd.uLps);
		cout<<"Push start of solution to the center of the channel (exit when done)"<<endl;	
		double vol=chan->s->fineControl(chan->syringePos,sd.wash->sd.uLps);
		logFile.write(chan->toString()+" coarse push injection tubing volume is "+::toString(vol)+" uL",true);
	}else{
		chan->waste();
		chan->select();
		//remove air from tubing
		sd.s->pull(sd.waste->sd.syringePort,1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol),sd.waste->sd.uLps);
		sd.waste->valveSelect();
		sd.waste->sd.s->push(sd.wash->sd.syringePort,1.5*(sd.wash->sd.dv.getInjectTubingVolPull(sd.wash->sd.valveNum)+sd.wash->sd.dv.injectTubingVol),sd.wash->sd.uLps);

		//pull in enough wash solution to reach the chamber
		sd.wash->pull(chan->s->volume-2*sd.air->sd.loadFactor*chan->cham.getChannelVolume());//1.5*(this->getInjectTubingVolume()+chan->getInjectTubingVolPush()));
		//pull in air gap
		sd.air->pull(sd.air->sd.loadFactor*chan->cham.getChannelVolume());
		chan->push(this->getInjectTubingVolume()+sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.wash->sd.uLps);
		cout<<"Push start of solution to the center of the channel (exit when done)"<<endl;	
		double vol=chan->s->fineControl(chan->syringePos,sd.wash->sd.uLps);
		logFile.write(chan->toString()+" coarse push injection tubing volume is "+::toString(vol)+" uL",true);
	}
}

FlowSol PushReagent::clean(double inletFactor,double uLps, FlowChannel* f,Solution* s){
	FlowSol fs;
	fs.f=f;
	fs.s=s;
	if (s==NULL)
		s=sd.wash;
	if (s->sd.dv.v1!=sd.dv.v1 || s->sd.dv.v2!=sd.dv.v2){
		logFile.write("Error in Push Reagent clean: wash solution is not on the same daisy valve as this reagent. cleaning will not be performed",true);
		return fs;
	}

	double vol=sd.inletTubingVol*inletFactor+abs(s->getInjectTubingVolume()-this->getInjectTubingVolume());
	double pos=sd.s->steps2vol(sd.s->getPosition());
	while(vol>sd.s->volume-pos){
		s->pull(sd.s->volume-pos,fs.f,uLps);
		valveSelect();
		this->sd.s->push(sd.syringePort,sd.s->volume-pos,uLps,true,0);
		vol=vol-(sd.s->volume-pos);
	}
	s->pull(vol,f,uLps);
	valveSelect();
	this->sd.s->push(sd.syringePort,vol,uLps,true,0);

	if (sd.air->sd.inletTubingVol!=0 && s!=sd.air){
		logFile.write("Reagent has air solution with non-zero inlet tubing volume. Washing air tubing also",true);
		sd.air->clean(inletFactor,uLps,f,s);
	}
	return fs;
}

void PullReagent::coarseCalibrateInlet(){
	
}

void PullReagent::calibrateInlet(){
	FlowChannel* f=scan.fs.selectChannel();
	cout<<"Pull reagent line out of tube to pull in air (enter to continue)"<<endl;
	getString();
	//empty reagent inlet tubing 
	valveSelect();
	f->pull(sd.inletTubingVol,sd.uLps);
	cout<<"Put reagent line in tube with solution (enter to continue)"<<endl;
	getString();
	//remove air from flow channel
	sd.wash->load(f);
	f->waste();
	//pull solution fixed inlet volume
	valveSelect();
	f->pull(sd.inletTubingVol,sd.uLps);
	//pull air gap
	sd.air->valveSelect();
	f->pull(sd.air->sd.loadFactor*f->cham.getChannelVolume(),sd.uLps);
	//pull wash
	sd.wash->valveSelect();
	f->pull(sd.inletTubingVol*0.75+sd.dv.getInjectTubingVolPull(sd.valveNum),sd.uLps);
}

void PullReagent::calibrateDV(){//daisyInjectTubingVolume
}
void PullReagent::coarseCalibrateDV(){
}
void PullReagent::calibrateDV2(){//injectTubingVolume
}
void PullReagent::coarseCalibrateDV2(){
}
void PullReagent::coarseCalibrateFlowChannel(FlowChannel* chan){
}
void PullReagent::calibrateFlowChannel(FlowChannel* chan){
}


void PullReagent::load(FlowChannel* chan,double param, double time){
	//pull in chamber volume of reagent, already has air gap in front
	chan->waste();
	chan->select();
	this->sd.dv.select(this);
	chan->pull(sd.air->sd.loadFactor*chan->cham.getChannelVolume()+sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//pull twice the volume of the air gap
	sd.air->valveSelect();
	chan->pull(2*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//push air back out reagent port
	valveSelect();
	chan->push(1.6*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	chan->pull(0.3*sd.air->sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//pull wash behind
	sd.wash->valveSelect();
	chan->pull(this->getInjectTubingVolume()+chan->getInjectTubingVolPull()-sd.air->sd.loadFactor*chan->cham.getChannelVolume()-0.5*sd.loadFactor*chan->cham.getChannelVolume(),sd.uLps);
	//move reagent into the chamber, followed by air
//switch to closest air port
//	v->getInjectTubingVol();
//	chan->pull(injectTubingVol,uLps);
}


void SolutionData::parsePullReagent(string line,FluidicsSetup& fs){
//parse line and update this reagent to match
	parseReagent(line,fs);
	if (!wash)
	wash=fs.getSolution("PullWash");
	if (!air)
		if (valveNum==1)
			air=fs.getSolution("PullAir");
		else
			air=fs.getSolution("PullAir2");
}

void SolutionData::parsePushReagent(string line,FluidicsSetup& fs){
//parse line and update this reagent to match
	parseReagent(line,fs);
	if (!wash)
	wash=fs.getSolution("PushWash");
	if (!air)
		if (valveNum==1)
			air=fs.getSolution("PushAir");
		else
			air=fs.getSolution("PushAir2");
	if (wash->sd.valveNum==-1)
		waste=fs.getSolution("Waste");
	else waste=NULL;
}

void SolutionData::parseReagent(string line,FluidicsSetup& fs){
	//get part of line that is the same as solution
	stringstream ss;
	ss<<line;
	string solLine;
	int i=0,numMax=6;
	while(i<numMax){		
		getline(ss,line,',');
		if (ss.fail())
			break;
		solLine+=line+",";
		i++;
	}
	//remove final comma
	solLine=solLine.substr(0,solLine.size()-1);
	parseSolution(solLine);
	
	//now get things unique to Reagent
	
	//Dedicated Wash
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!="")
		this->wash=fs.getSolution(line);
	//Dedicated Air
	getline(ss,line,',');
	if (!ss.fail() && removeWhite(line)!="")
		this->air=fs.getSolution(line);
	getline(ss,line,',');
	if (!ss.fail())
		throw exception(string("Reagent "+this->name+" parse error: too many input parameters").c_str());
}

PullReagent::PullReagent(SolutionData& sd):Solution(sd),Reagent(sd),PullSolution(sd){}
PushReagent::PushReagent(SolutionData& sd):Solution(sd),Reagent(sd),PushSolution(sd){
	if (sd.valveNum!=sd.air->sd.valveNum)
		throw exception(string("Cannot Create "+toString()+" . PushReagent must be on the same valve as the air solution.").c_str());
	if (sd.valveNum==-1)
		throw exception(string("Cannot Create "+toString()+" . PushReagents cannot be connected directly to the Syringe. They will contaminate the Syringe.").c_str());
	if (sd.valveNum!=-1 && sd.wash->sd.valveNum==-1 && sd.waste==NULL)
		throw exception(string("Cannot Create "+toString()+" . When PushReagent has a wash located on a syringe, a 'Waste' Solution must be defined for cleaning up after priming").c_str());
	if (sd.waste && sd.waste->sd.valveNum<sd.valveNum)
		throw exception(string("Cannot Create "+toString()+" . Waste must be on a valve after this PushReagent.").c_str());
}

Solution* PullReagent::clone(FluidicsSetup& fs){
	SolutionData tempSD(this->sd);
	tempSD.wash=fs.getSolution(this->sd.wash->sd.name);
	tempSD.air=fs.getSolution(this->sd.air->sd.name);
	return new PullReagent(tempSD);
}

Solution* PushReagent::clone(FluidicsSetup& fs){
	SolutionData tempSD(this->sd);
	tempSD.wash=fs.getSolution(this->sd.wash->sd.name);
	tempSD.air=fs.getSolution(this->sd.air->sd.name);
	if (tempSD.waste)
		tempSD.waste=fs.getSolution(this->sd.waste->sd.name);
	return new PushReagent(tempSD);
}

string Reagent::toString(){
	return "Reagent "+toString_();
}