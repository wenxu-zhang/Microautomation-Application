// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "AcquisitionGroup.h"
#include "Camera.h"
#include "Channel.h"
#include "Controller.h"
extern Controller cont;
AcquisitionGroup::AcquisitionGroup(int cumNumChans,int numLocalSpotChanges):cumNumChans(cumNumChans),numLocalSpotChanges(numLocalSpotChanges),analogWaveform(NULL),digitalWaveform(NULL){}
AcquisitionGroup::~AcquisitionGroup(){
	delete[] analogWaveform;
	delete[] digitalWaveform;
}

int AcquisitionGroup::numChans(){
	return chanNumToImageNum.size()*max(numLocalSpotChanges,1);
}

AcquisitionChannel AcquisitionGroup::getChan(int chanNum){
	chanNum=(chanNum-cumNumChans);
	if (chanNum< 0 || chanNum>numChans()){
		logFile.write("Error: chanNum not in this group",true);
		return AcquisitionChannel();
	}
	return acquisitionChannels.at(chanNumToImageNum.at(chanNum%chanNumToImageNum.size()));
}

bool AcquisitionGroup::addAcquisitionChannel(AcquisitionChannel a){
	if(acquisitionChannels.empty()){
		lights.insert(a.chan->lite().ls);
		acquisitionChannels.push_back(a);
		if (a.intensity==a.chan->lite().ls->cp->zeroIntensity){
			imageNumToChanNum.push_back(-1);//pause image
		}
		else{
			chanNumToImageNum.push_back(acquisitionChannels.size()-1);
			imageNumToChanNum.push_back(cumNumChans);
		}
		return true;
	}else {
		
		
		//camera compatibility, only one camera can be supported per acquisitionGroup
		bool b1=a.out->cam==acquisitionChannels.back().out->cam;
		//camera settings must be the same
		bool b2=a.ap.bin==acquisitionChannels.back().ap.bin;
		bool b3=a.ap.getGain()==acquisitionChannels.back().ap.getGain();
		bool b4=a.ap.imageRegion==acquisitionChannels.back().ap.imageRegion;
		//exposure time must be compatible with frametransfer mode external triggering (minimumum exposure time is ~38ms with full frame no binning)
		bool b5=a.out->cam->isTriggerable(a.ap);

		//lightsource compatibility, different light sources are acceptable as long as they have the same trigger or if a light source is not triggerable then it needs to be the same as all the other channels in this acquisition group (no light switching necessary)
		bool b6=a.chan->lite().ls->isTriggerable() || a.chan->lite()==acquisitionChannels.back().chan->lite();
		bool b7=a.chan->lite().ls->t==acquisitionChannels.back().chan->lite().ls->t;

		//cannot change microscope configuration within an AcquisitionGroup
		bool b8=a.out->s==acquisitionChannels.back().out->s;
		bool b9=a.m==acquisitionChannels.back().m;
		bool b10=a.chan->exFilter==acquisitionChannels.back().chan->exFilter;
		bool b11=a.chan->reflector==acquisitionChannels.back().chan->reflector;
		bool b12=a.chan->emFilter==acquisitionChannels.back().chan->emFilter;

		//camera and lightsource must have the same trigger unless the light source does not need to be triggered
		bool b13=a.out->cam->t==a.chan->lite().ls->t || a.chan->lite()==acquisitionChannels.back().chan->lite();
		
		if(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13){
			//are we doing TIRF?
			if (a.chan->tirf){//make sure the trigger is the same
				if (a.chan->tirf->getTrigger() != acquisitionChannels.back().out->cam->t){
					return false;
				}
			}
			lights.insert(a.chan->lite().ls);
			acquisitionChannels.push_back(a);
			if (a.intensity==a.chan->lite().ls->cp->zeroIntensity)
				imageNumToChanNum.push_back(-1);//pause image
			else{
				chanNumToImageNum.push_back(acquisitionChannels.size()-1);
				imageNumToChanNum.push_back(chanNumToImageNum.size()-1+cumNumChans);
			}
			return true;
		}else return false;
	}
}

AcquisitionGroup AcquisitionGroup::getAcquisitionGroup(){
		AcquisitionGroup ag;
		int ind;
		char in;
		AcquisitionChannel c=cont.currentChannel();
		while(true){
			cout<<"0:  Change current channel ("<<c.toString()<<endl;
			cout<<"1:  Add current channel (total so far is "<<ag.acquisitionChannels.size()<<")"<<endl;	
			cout<<"2:  Delete a channel"<<endl;
			cout<<"a:  Accept AcquisitionGroup"<<endl;
			cin>>in;
			std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			switch(in){
			case '0':c.modify();
				break;
			case '1':
				if (!ag.addAcquisitionChannel(c))
					cout<<"Channel is incompatible with this acquisition group (change current channel)"<<endl;
				break;
			case '2':
				cout<<"Enter index to delete (0 to "<<ag.acquisitionChannels.size()-1<<")"<<endl;
				cin>>ind;
				if (ind>=0 && ind<=ag.acquisitionChannels.size()-1){
					vector<AcquisitionChannel>::iterator i=ag.acquisitionChannels.begin();
					ag.acquisitionChannels.erase(i+ind);
				}
				break;
			case 'a':
				return ag;
				break;
			}
		}
	}
