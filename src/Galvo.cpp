// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, June 20, 2011</lastedit>
// ===================================
#include "Controller.h"
#include "Galvo.h"
#include "Utils.h"
extern Controller cont;
extern Record logFile;

const double Galvo::VELTHRESH=.01;
const double Galvo::POSTHRESH=.01;//10mV accuracy
const double Galvo::LINEARITY=.75;//10V in gives 7.5V out

Galvo::Galvo(Trigger* t,string paramsFileName,int GALVOPOSIN,int GALVOVELIN):Laser(t,paramsFileName),GALVOPOSIN(GALVOPOSIN),GALVOVELIN(GALVOVELIN){
	if (clp->triggerOption!=1){
		logFile.write("Error: galvanometer is analog only trigger control");
		isPresent=false;
	}
	/*int volt=1;
	set(volt);
	analogIn=false;
	if (t->isValidAnalogInLine(GALVOPOSIN)){
		analogIn=true;
		if (get()>volt*LINEARITY+POSTHRESH || get()<volt*LINEARITY-POSTHRESH){
			analogIn=false;
			logFile.write("Galvonometer Mirror Feedback Error",true,"...check linearity constant and position accuracy");
		}
	}
	*/
	//off();
	
}

void Galvo::set(double pos){
	CheckExists()
	LaserParams* lp=(LaserParams*) ncp;
	t->setVoltage(this->cp->analogTriggerLine,pos);
}

double Galvo::get(){
	if (!analogIn)
		logFile.write("No voltage input for galvonometer",true);
	CheckExists(0);
	return t->getVoltage(GALVOPOSIN);
}

/*
void Galvo::on(int pos,double intensity){
	CheckExists()
	set(intensity);
}

void Galvo::off(){
	CheckExists()
	set(cp->zeroIntensity);
}
*/