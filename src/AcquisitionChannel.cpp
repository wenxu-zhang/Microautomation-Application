// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, November 14, 2011</lastedit>
// ===================================
#include "Controller.h"
#include "AcquisitionChannel.h"
#include "Camera.h"
#include "Channel.h"
#include "Magnification.h"
#include "Utils.h"
#include <limits>
#include "Light.h"
#include "XYStage.h"
extern Controller cont;
extern Record logFile;
//default acquisition channel will not be useable...any error generated because of this is an error in the calling code

AcquisitionChannel::AcquisitionChannel():
chan(0),
out(0),
m(0),
ap(),
intensity(0),
showScaleBar(false),
showOnScreen(false),
TIRFangle(0)
{
}

AcquisitionChannel& AcquisitionChannel::operator=(const AcquisitionChannel& ac){
chan=ac.chan;
out=ac.out;
m=ac.m;
ap=ac.ap;
intensity=ac.intensity;
showOnScreen=ac.showOnScreen;
showScaleBar=ac.showScaleBar;
TIRFangle=ac.TIRFangle;
validate();
return *this;
}

AcquisitionChannel::AcquisitionChannel(const AcquisitionChannel& ac)
:chan(ac.chan),
out(ac.out),
m(ac.m),
ap(ac.ap),
intensity(ac.intensity),
showOnScreen(ac.showOnScreen),
showScaleBar(ac.showScaleBar),
TIRFangle(ac.TIRFangle){
	if (chan) validate();
}

AcquisitionChannel::AcquisitionChannel(Channel* chan, OutPort* out,Magnification* m, AcquisitionParameters& ap,double intensity, string intensityUnits, bool showOnScreen,bool showScaleBar,double TIRFangle)
:chan(chan),
out(out),
m(m),
ap(ap),
intensity(0),
showOnScreen(false),
showScaleBar(false),
TIRFangle(0)
{
	
	if (chan) this->intensity=chan->lite().ls->getIntensity(intensity,intensityUnits);
	if (chan) validate();
}

AcquisitionChannel::AcquisitionChannel(Channel* chan, OutPort* out,Magnification* m)
:chan(chan),
out(out),
m(m),
ap(),
intensity(0),
showOnScreen(),
showScaleBar(false),
TIRFangle(0)
{
	if (chan) ap=out->cam->getDefaultAcquisitionParameters();
	if (chan && m) intensity=chan->lite().ls->defaultIntensity(&(m->obj));
	else if (chan)
		intensity=chan->lite().ls->defaultIntensity(NULL);
	if (chan) TIRFangle=chan->getTIRFangle(&(m->obj));
	if (chan) validate();
}

AcquisitionChannel::AcquisitionChannel(int channel,
									   int outport,
									   int magnification,
									   AcquisitionParameters& ap,
									   double intensity,
									   string unit,
									   bool showOnScreen,
									   bool showScaleBar,double TIRFangle):
	/*chan(&(cont.channels[channel])),
	out(&(cont.outPorts[out])),
	m(&(cont.mags[mag])),*/
	ap(ap),
	intensity(0),
	showOnScreen(showOnScreen),
	showScaleBar(showScaleBar),
	TIRFangle(TIRFangle)
{
	if (channel<0 || channel>=cont.channels.size()){
		logFile.write(string("Error: attempt to use unsupported channel ")+::toString(channel)+", using default.", true);
		channel=0;
	}
	if (outport<0 || outport>=cont.outPorts.size()){
		logFile.write(string("Error: attempt to use unsupported output port ")+::toString(outport)+", using default.", true);
		outport=0;
	}
	if (magnification<0 || magnification>=cont.mags.size()){
		logFile.write(string("Error: attempt to use unsupported magnification ")+::toString(magnification)+", using default.", true);
		magnification=0;
	}
	chan=&(cont.channels[channel]);
	out=&(cont.outPorts[outport]);
	m=&(cont.mags[magnification]);
	this->intensity=chan->lite().ls->getIntensity(intensity,unit);
	validate();
}

void AcquisitionChannel::validate(){
	if (!this->chan->isOutportSupported(*(this->out))){
		
		logFile.write(string("Outport ")+this->out->toString()+" not supported for Channel "+this->chan->toString(),true);
		this->out=this->chan->getDefaultOutport();
		logFile.write(string("Switched outport to ")+this->out->toString(),true);
	}
	if (!this->out->cam->validate(this->ap))
		logFile.write("AcquisitionParameters are invalid...setting to default", true);

	if (TIRFangle==-20)
		if (chan && m)
			TIRFangle=chan->getTIRFangle(&(m->obj));
		else 
			TIRFangle=0;
}

void AcquisitionChannel::on(bool wait){
	if (m) m->set();
	if (out) out->set();
	if (wait){
		if (m) m->wait();
		if (out) out->wait();
	}
	//if (intensity~=chan->lite().ls->getIntensity(0,"%")) {
	if (chan) {
		if (TIRFangle==-20)
			logFile.write("Error: invalid TIRF angle. acquisition channel was not validated",true);
		chan->on(intensity,wait,TIRFangle);
	}
	//}
}


void AcquisitionChannel::off(bool wait){
	chan->off();
	if (wait)
		chan->wait();
}

void AcquisitionChannel::wait(){
	chan->wait();
	m->wait();
	out->wait();
}

void AcquisitionChannel::adjustIntensity(double percentSaturation){
	this->out->cam->adjustIntensity(*this,percentSaturation);
}

void AcquisitionChannel::takePicture(std::string fileName){
	if (ap.gains.size()>1) {
		logFile.write("AcquisitionChannel->takePicture: Don't use this function for Multiple cameras. it is not supported",true);
		return;
	}
	unsigned short ret;
	if (fileName!="") fileName=cont.workingDir+fileName;
	on(true);
	out->cam->takePicture(this,fileName);
	logFile.write(string("AcquisitionChannel: taking picture ")+fileName+" at pos x:"+::toString(cont.stg->getX())+" y:"+::toString(cont.stg->getY()),true);
	off();
	wait();
}

//in microns
double AcquisitionChannel::getFOVHeight(){
	if (out)
		return out->cam->getImageHeight(this->ap)/this->m->get();
	else
		return 0;
}

//in microns
double AcquisitionChannel::getFOVWidth(){
	if (out)
		return out->cam->getImageWidth(this->ap)/this->m->get();
	else 
		return 0; 
}

std::string AcquisitionChannel::toString(){
	return string("channel: ")+chan->toString()+" "+ap.toString()+" intensity:"+chan->lite().ls->intensityToString(intensity)+" magnification:"+m->toString();
}

bool AcquisitionChannel::identical(std::vector<AcquisitionChannel> vac){
	if  (vac.size()<2) return true;	
	for (vector<AcquisitionChannel>::iterator i=vac.begin()+1;i!=vac.end();i++){
		if (!(*(i-1)==*i))
			return false;
	}
	return true;
}

void AcquisitionChannel::modify(){
	char cTemp;
	while(true){
		cout<<"Please confirm acquisition channel settings"<<endl;
		cout<<"1: Change channel            ("<<chan->toString()<<")"<<endl;
		cout<<"2: Change outPort             ("<<out->s.name<<")"<<endl;
		cout<<"3: Change mag                ("<<m->toString()<<")"<<endl;
		cout<<"4: Change Acquisition Params ("<<ap.toString()<<")"<<endl;
		cout<<"5: Change Light Intensity    ("<<chan->lite().ls->intensityToString(intensity)<<")"<<endl;
		cout<<"6: Change showOnScreen       ("<<::toString(showOnScreen)<<")"<<endl;
		cout<<"7: Change showScaleBar       ("<<::toString(showScaleBar)<<")"<<endl;
		cout<<"a: accept current settings"<<endl;
		cin>>cTemp;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(cTemp){
			case '1':
				chan=Channel::select();
				intensity=chan->lite().ls->getIntensity();
				break;
			case '2':
				out=OutPort::select();
				break;
			case '3':
				m=Magnification::select();
				break;
			case '4':
				ap.modify();
				break;
			case '5':{
				double d=0;
				string unit;
				cout<<"input intensity"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"input units"<<endl;
				cin>>unit;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				intensity=this->chan->lite().ls->getIntensity(d,unit);
				break;}
			case '6':
				showOnScreen=!showOnScreen;
				break;
			case '7':
				showScaleBar=!showScaleBar;
				break;
			case 'a':
				return;
				break;
		}
	}
}

vector<AcquisitionChannel> AcquisitionChannel::getAcquisitionChannels(){
	vector<AcquisitionChannel> channels;
	int ind;
	char in;
	while(true){
		cout<<"1:  Add Channel (total so far is "<<channels.size()<<")"<<endl;
		cout<<"2:  Edit Channel"<<endl;
		cout<<"3:  Delete Channel"<<endl;
		cout<<"a:  Accept channels"<<endl;
		cin>>in;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(in){
			case '1':
				if  (channels.size()>0)
					channels.push_back(channels.back());
				else 
					channels.push_back(cont.currentChannel());
				channels.back().modify();
				break;
			case '2':
				cout<<"Enter index to modify (0 to "<<channels.size()-1<<")"<<endl;
				cin>>ind;
				if (ind>=0 && ind<=channels.size()-1){
					channels[ind].modify();
				}
				break;
			case '3':
				cout<<"Enter index to delete (0 to "<<channels.size()-1<<")"<<endl;
				cin>>ind;
				if (ind>=0 && ind<=channels.size()-1){
					vector<AcquisitionChannel>::iterator i=channels.begin();
					channels.erase(i+ind);
				}
				break;
			case 'a':
				return channels;
				break;
		}

	}

}

void AcquisitionChannel::takeMultiplePics(vector<AcquisitionChannel>& channels,std::string prefix,std::string suffix){
	int num=0;
	for(vector<AcquisitionChannel>::iterator i=channels.begin();i!=channels.end();i++,num++){
		if (i->chan->tirf){
			i->takePicture(prefix+"Image"+::toString(num,2)+i->chan->toString()+"m"+i->m->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain())+"i"+i->chan->lite().ls->intensityToString(i->intensity)+"a"+::toString(i->TIRFangle)+suffix);
		}else
			i->takePicture(prefix+"Image"+::toString(num,2)+i->chan->toString()+"m"+i->m->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain())+"i"+i->chan->lite().ls->intensityToString(i->intensity)+suffix);
	}
}

void AcquisitionChannel::takeMultipleAccumulations(vector<AcquisitionChannel> &channels,int num,string prefix,string suffix){
	for(vector<AcquisitionChannel>::iterator i=channels.begin();i!=channels.end();i++){
		i->takeAccumulation(num,prefix+i->chan->toString()+"e"+::toString(i->ap.exp)+"g"+::toString(i->ap.getGain())+suffix);
	}
}

void AcquisitionChannel::takeAccumulation(int num,string fileName){
	out->cam->takeAccumulation(this,num,cont.workingDir+fileName);	
}

bool AcquisitionChannel::isTriggerable(){
	bool b5=out->cam->isTriggerable(ap);
	bool b6=chan->lite().ls->isTriggerable();
	bool b13=out->cam->t==chan->lite().ls->t;
	return b5&&b6&&b13;
}

/*
vector<int> Controller::selectMultipleChannels(){
	char c;
	double halogenVoltage;
	vector<int> chans;
	//focus->closeShutter();
	string token,line;
	stringstream iss;
	int sel;
	cout<<"Please select channels to use separated by commas (e.g. 1,2,3,4 for DAPI, FITC, CY3, CY5) or press enter for those defaults"<<endl;
	cout<<"0: Bright field"<<endl;
	cout<<"1: DAPI, Alexa 350, Alexa 405  Ex(387nm) Em(440nm)"<<endl;
	cout<<"2: FITC, Alexa 488             Ex(485nm) Em(520nm)"<<endl;
	cout<<"3: Cy3, TRITC, Alexa 568       Ex(560nm) Em(606nm)"<<endl;
	cout<<"4: Cy5, Alexa 680, Alexa 660   Ex(650nm) Em(699nm)"<<endl;
	cout<<"5: UV Exposure for photocleaving (no light should get to camera)"<<endl;
	cout<<"6: Zeiss FITC Filter Set 10 (Fluosphere Yellow-Green)"<<endl;
	cout<<"7: TransFluosphere 488/605 (Ex: Zeiss FITC Em: Zeiss Cy3"<<endl;
	cout<<"8: TransFluosphere 488/645 (Ex: Zeiss FITC Em: Zeiss Cy5"<<endl;
	cout<<"9: Zeiss Cy3 Filter Set 26 (Fluosphere Red)"<<endl;
	cout<<"Z: Zeiss Cy5 Filter Set 43\n";
	getline(cin, line);
	getline(cin, line);
	if (line=="") {
		chans.push_back(DAPI);
		chans.push_back(FITC);
		chans.push_back(CY3);
		chans.push_back(CY5);	
		ch[0]=FITC;
		del[0]=0.5;
		ch[1]=CY3;
		del[1]=0.5;
		ch[2]=CY5;
		del[2]=0.5;
		numChannels=3;
	}else{
		iss << line;
		int flcounter=0;
		numChannels=0;
		while ( getline(iss, token, ',') )
		{
			c=token[0];
			switch(c){
				case '0':
					cout << "Please enter Halogen lamp voltage: \n";
					cin >> halogenVoltage;
					focus->SetHalogenVoltage(halogenVoltage);
					sel= BF;
					break;
				case '1': sel= DAPI;break;
				case '2': sel= FITC;break;
				case '3': sel= CY3;break;
				case '4': sel= CY5;break;
				case '5': sel= UV;break;
				case '6': sel= Controller::FITCZ;break;
				case '7': sel= Controller::FITCZCY3Z;break;
				case '8': sel= Controller::FITCZCY5Z;break;
				case '9': sel= Controller::CY3Z;break;
				case 'Z': sel= Controller::CY5Z;break;
				default:
					cout<<c<<" is not a valid option"<<endl<<endl;
					sel=0;
					break;
			}
			//if (sel==0 || std::find(chans.begin(),chans.end(),sel)!=chans.end()) continue;
			chans.push_back(sel);//cout << token << endl;
			if (sel==BF){
				acquireBF=true;
			}else{
				ch[flcounter]=sel;
				if (sel>0){
					if (del[flcounter]!=200) del[flcounter]=0.5; //if need to change cube back to pinkel
				}else{
					del[flcounter]=200; //assume it takes 200ms to switch reflector cubes.
					del[flcounter+1]=200;
				}
				flcounter++;
			}
			numChannels++;
		}
		iss.clear();
	}
	return chans;
}*/
