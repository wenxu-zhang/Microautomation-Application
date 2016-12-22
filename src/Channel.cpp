// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#include "Channel.h"
#include "Controller.h"
#include "Utils.h"
#include <limits>
#include "Record.h"
#include <string>
#include "Objective.h"

extern Controller cont;
extern Record logFile;

Channel::Channel(std::string name, std::string desc, Light lite,Selection reflector,Changer* tirf,Selection exFilter,Selection emFilter)
:name(name),
desc(desc),
lites(1,lite),
exFilter(exFilter),
reflector(reflector),
emFilter(emFilter),
tirf(tirf),
prefOut(NULL)
{
	defaultTIRFangles=vector<double>(cont.axio.objectives.size(),0);
	alternates=vector<vector<string>>(cont.outPorts.size());
}//fully defines a channel with an arbitrary setup
Channel::~Channel(){
	//lite.off();
}


Channel::Channel(std::string name,std::string desc ,  vector<Light> lites,Selection reflector,Changer* tirf,Selection exFilter,Selection emFilter)
:name(name),
desc(desc),
lites(lites),
exFilter(exFilter),
reflector(reflector),
emFilter(emFilter),
tirf(tirf),
prefOut(NULL)
{
	defaultTIRFangles=vector<double>(cont.axio.objectives.size(),0);
	alternates=vector<vector<string>>(cont.outPorts.size());
}//fully defines a channel with an arbitrary setup


void Channel::addTIRFangle(double angle,Objective* obj){
	if (obj==NULL){
		logFile.write("Could not add default tirf angle. objective not found",true);
		return;
	}
	int ind=cont.axio.getObjectiveIndex(obj);
	if (ind==-1){
		logFile.write("Error adding TIRF angle: invalid objective",true);
		return;
	}
	defaultTIRFangles.at(ind)=angle;
}

double Channel::getTIRFangle(Objective* obj){
	if (obj==NULL)
		return 0;
	int ind=cont.axio.getObjectiveIndex(obj);
	if (ind==-1){
		logFile.write("Error getting TIRF angle: invalid objective",true);
		return 0;
	}
	return defaultTIRFangles.at(ind);
}


void Channel::on(double intensity, bool wait,double TIRFangle){
	exFilter.set();
	reflector.set();
	emFilter.set();
	if (tirf){
		tirf->set(TIRFangle);
		logFile.write(string("Set TIRF angle for ")+this->toString()+" channel to "+::toString(TIRFangle));
	}
	logFile.write(string("Turned ")+this->toString()+" channel on with intensity "+lite().ls->intensityToString(intensity));
	if (wait){
		exFilter.wait();
		reflector.wait();
		emFilter.wait();
		if (tirf) tirf->wait();
	}
//if (iity!=0)
	lites.front().on(intensity);
	if (wait)
		lites.front().wait();
}

void Channel::on(vector<double>& intensity, bool wait,double TIRFangle){
	if (intensity.size()<lites.size())
		logFile.write("Warning, not enough intensities given to match light sources, the last intensity will be repeated to the remaining light sources",true);
	exFilter.set();
	reflector.set();
	emFilter.set();
	if (tirf){
		tirf->set(TIRFangle);
		logFile.write(string("Set TIRF angle for ")+this->toString()+" channel to "+::toString(TIRFangle));
	}
	if (wait){
		exFilter.wait();
		reflector.wait();
		emFilter.wait();
		tirf->wait();
	}
	//if (intensity!=0)
	int i=0;
	for(vector<Light>::iterator lt=lites.begin();lt!=lites.end();lt++,i++){
		if (i<intensity.size()){
			lt->on(intensity.at(i));
			logFile.write(string("Turned ")+this->toString()+" channel on with intensity "+lite().ls->intensityToString(intensity.at(i)));
		}else{
			lt->on(intensity.back());
			logFile.write(string("Turned ")+this->toString()+" channel on with intensity "+lite().ls->intensityToString(intensity.back()));
		}
	}
	if (wait) {
		for(vector<Light>::iterator lt=lites.begin();lt!=lites.end();lt++){
			lt->wait();
		}
	}
}
	

void Channel::wait(){
	for(vector<Light>::iterator lt=lites.begin();lt!=lites.end();lt++){
		lt->wait();
	}
	exFilter.wait();
	Timer t(true);
	reflector.wait();
	if (t.getTime()>1000)
		logFile.write("Refelctor wait took more than 1 second",true);
	emFilter.wait();
}

void Channel::off(){
	for(vector<Light>::iterator lt=lites.begin();lt!=lites.end();lt++){
		lt->off();
	}
	logFile.write(string("Turned ")+this->toString()+" channel off");
}

Light Channel::lite(int n){
	if (n>=name.size()||n==-1)
		n=0;
	return lites.at(n);
}

std::string Channel::toString(OutPort* outPort, int n) const{
	if (!outPort)
		return name;
	int j=0,index=-1;
	for(vector<OutPort>::iterator i=cont.outPorts.begin();i!=cont.outPorts.end();i++,j++){
		if (i->s.name==outPort->s.name){
			index=j;
			break;
		}
	}
	if (index==-1 && n==-1)
		return name;
	
	/*what is this?
	if (index==-1){
		int bs=alternates.at(index).size();
		if (n>=bs)
			return name+"-unknown channel";
	}*/

	if (index!=-1 && n==-1 && alternates.at(index).size()==1)
		return alternates.at(index).front();
	if (index!=-1 && n==-1 && alternates.at(index).size()>1)
		return name;
	if (index!=-1 && n==-1 && alternates.at(index).empty())
		return name;
	if (index!=-1 && n!=-1 && alternates.at(index).empty()){
		if (n==0)
			return name+"-reflected";
		if (n==1)
			return name+"-transmitted";
		return name+"-unknown channel";
	}
	return alternates.at(index).at(n);
}

bool Channel::isOutportSupported(OutPort& out){
	int j=0;
	int res=-1;
	for(vector<OutPort>::iterator i=cont.outPorts.begin();i!=cont.outPorts.end();i++,j++){
		if (i->s.name==out.s.name){
			res=j;
			break;
		}
	}
	if (res==-1)
		return false;
	else
		if (alternates.at(res).size()==1 && alternates.at(res).front()=="Deny")
			return false;
		else return true;
}

OutPort* Channel::getDefaultOutport(){
	if (prefOut!=NULL)
		return prefOut;
	int j=0;
	int res=-1;
	for(vector<vector<string>>::iterator i=alternates.begin();i!=alternates.end();i++,j++){
		if (!i->empty() && i->front()!="Deny"){
			res=j;
			break;
		}
	}
	if (res==-1 && cont.OUT_CAM!=-1 && alternates.at(cont.OUT_CAM).empty()){
		res=cont.OUT_CAM;
	}
	if (res==-1){
		j=0;
		res=-1;
		for(vector<vector<string>>::iterator i=alternates.begin();i!=alternates.end();i++,j++){
		if (i->empty()){
			res=j;
			break;
		}
		}

	}
	if (res==-1){
		logFile.write("No supported outports for this channel. This should never happen");
		return NULL;
	}else
		return &(cont.outPorts.at(res));
}

void Channel::addAlternateName(OutPort& out, std::vector<std::string> alt){
	if (prefOut==NULL)
		prefOut=&out;
	int j=0;
	int res=-1;
	for(vector<OutPort>::iterator i=cont.outPorts.begin();i!=cont.outPorts.end();i++,j++){
		if (i->s.name==out.s.name){
			res=j;
			break;
		}
	}
	if (res==-1){
		logFile.write(string("OutPort ")+out.toString()+ " does not exist",true);
		return;
	}else
		alternates.at(res)=alt;
}

void Channel::denyOutport(OutPort& out){
	int j=0;
	int res=-1;
	for(vector<OutPort>::iterator i=cont.outPorts.begin();i!=cont.outPorts.end();i++,j++){
		if (i->s.name==out.s.name){
			res=j;
			break;
		}
	}
	if (res==-1){
		logFile.write(string("OutPort ")+out.toString()+ " does not exist",true);
		return;
	}else
		alternates.at(res)=vector<string> (1,"Deny");
}

Channel* Channel::select(){
	cout<<"Please select a Channel"<<endl;
	int ind=0;
	for(vector<Channel>::const_iterator i=cont.channels.begin();i!=cont.channels.end();i++){
		cout<<::toString(ind,2)<<": "<<i->toString()<<endl;
		ind++;
	}
	cin>>ind;
	if (cin.fail()){
		cin.clear();
		cout<<"Invalid entry Please enter a number from 0 to "<<cont.channels.size()-1<<endl;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		return select();
	}
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (ind>cont.channels.size()-1 || ind<0){
		logFile.write("invalid channel selected. try again",true);
		return select();
	}
	return &(cont.channels[ind]);
}

int Channel::get(string name){
	int j=0;
	for(vector<Channel>::iterator i=cont.channels.begin();i!=cont.channels.end();i++,j++){
		if (i->name==name)
			return j;
	}
	return -1;
}

//bool Channel::addLightSource(Light lite){
//	vector<Light>::iterator i;
//	for(i=lt.begin();i!=lt.end();i++){
//		if (lite.ls==(*i).ls) break;
//	}
//	if (i==lt.end()){ 
//		lt.push_back(lite);
//		return true;
//	}else return false;
//}
/*
int Controller::selectChannel(){
	char c;
	double halogenVoltage;
	int chan;
	//focus->closeShutter();
	while(true){
		cout<<"Please select a channel to use"<<endl;
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
		cout<<"Z: Zeiss Cy5 Filter Set 43"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '0':
				cout << "Please enter Halogen lamp voltage: \n";
				cin >> halogenVoltage;
				focus->SetHalogenVoltage(halogenVoltage);
				chan= BF;
				break;
			case '1': chan= DAPI;break;
			case '2': chan= FITC;break;
			case '3': chan= CY3;break;
			case '4': chan= CY5;break;
			case '5': chan= UV;break;
			case '6': chan= Controller::FITCZ;break;
			case '7': chan= Controller::FITCZCY3Z;break;
			case '8': chan= Controller::FITCZCY5Z;break;
			case '9': chan= Controller::CY3Z;break;
			case 'Z': chan= Controller::CY5Z;break;
			default:
				return 0;//cout<<c<<" is not a valid option"<<endl<<endl;
				break;
		}
		return chan;
	}	
}*/
