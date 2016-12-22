// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#include "LightSource.h"
#include "Controller.h"
#include "Channel.h"
extern Controller cont;
LightSource::LightSource(Trigger* t,string name,int triggerLine, double defaultIntensity,double delay, int analogTriggerLine,int triggerOption,double zeroIntensity):isPresent(true),t(t),name(name),ncp(new NonConstParams(defaultIntensity)),cp(new const ConstParams(triggerLine,delay,analogTriggerLine,triggerOption,zeroIntensity)){
		if (t && triggerOption!=1 && !t->isValidLine(triggerLine)){
			logFile.write(std::string("Error: Light source digital trigger line ")+toString(triggerLine)+" is not valid");
			isPresent=false;
		}
		if (t && triggerOption>0 && !t->isValidAnalogOutLine(analogTriggerLine)){
			logFile.write(std::string("Error: Light source analog trigger line ")+toString(analogTriggerLine)+" is not valid");
			isPresent=false;
		}
	}



LightSource::LightSource(Trigger* t, string name,Params &p):isPresent(true),t(t),name(name),ncp(p.ncp),cp(p.cp){}

LightSource::~LightSource(){delete cp;delete ncp;}//must delete it here since it was only created for this object.

void LightSource::trigger(){
		if (isTriggerable()) {
			t->setLineLow(cp->triggerLine);
			t->setLineHigh(cp->triggerLine);
			t->setLineLow(cp->triggerLine);
		}
}

void LightSource::on(int pos, double intens){}

void LightSource::off(){}
void LightSource::wait(){
	Timer::wait(cp->delay);
}
void LightSource::enterRingBuffer(AcquisitionGroup& ag){}
bool LightSource::supportsRingBuffer(){return false;}
void LightSource::exitRingBuffer(){}//exit ring buffer if necessary
//double LightSource::getIntensity(double intensity,std::string intensityUnit,Objective* obj){return 0;}//passing no parameters will give default intensity for this light source default intensityUnit "" is chosen by the LightSource "V" for halogen "mW" for laser  and "f" (fraction from 0 to 1) for DG, "%" should also be supported.
//std::string LightSource::intensityToString(double intensity,Objective* obj){return "UnknownIntensity";}

bool LightSource::isTriggerable(){if (t) return true; else return false;}

bool LightSource::operator==(LightSource& right){
		if (right.t==t && right.ncp->defaultIntensity==ncp->defaultIntensity && right.cp->triggerLine==cp->triggerLine)
			return true;
		else return false;
}

void LightSource::calibrate(){logFile.write(name+" calibration not supported",true);return ;}
void LightSource::quickCalibrate(){logFile.write(name+" quick calibration not supported",true);return ;}

LightSource* LightSource::select(){
	cout<<"Please select a Light Source"<<endl;
	int ind=1;
	for(vector<LightSource*>::iterator i=cont.lightSources.begin();i!=cont.lightSources.end();i++){
			cout<<ind<<": "<<(*i)->name<<endl;
			ind++;
	}
	ind=getInt();
	if (ind<1 || ind>cont.lightSources.size()){
		cout<<"Invalid selection...try again"<<endl;
		return select();
	}
	return cont.lightSources.at(ind-1);
}

