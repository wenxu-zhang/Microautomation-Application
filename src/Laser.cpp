// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#include "Laser.h"
#include "Controller.h"
#include "Light.h"
#include "Utils.h"
extern Controller cont;
double getDefaultIntensity(double defaultIntensitymW,double maxV,double minV,double maxP,double minP){
	if (minP<0){
		logFile.write("Error: min laser power cannot be less than zero...duh!");
		return 0;
	}
	if (maxP>500){
		logFile.write("Error: no way. you want to use more than 500mW of light?");
		return 0;
	}
	if (minV<0){
		logFile.write("Error: in general we don't want to send a negative voltage to our lasers");
		return 0;
	}
	if (maxV>10){
		logFile.write("Error: in general we don't want to send more than 10V to our lasers");
		return 0;
	}
	if (defaultIntensitymW>maxP || defaultIntensitymW<minP){
		int old=defaultIntensitymW;
		defaultIntensitymW=(maxP-minP)/2.0;
		logFile.write(string("Error: incompatible default intensity ")+toString(old)+" setting to 50% or"+toString(defaultIntensitymW)+"mW");
	}
	return (defaultIntensitymW-minP)*(maxV-minV)/(maxP-minP)+minV;
	
		
		
	
}
extern Record logFile;
Laser::Laser(Trigger* t, string name,Galvo* g)
:LightSource(t,name,loadParams(name)),g(g),tempZ(-1),tempObj(NULL),lp((LaserParams*) ncp),clp((ConstLaserParams*)cp)
{
	if (!lp->res){
		logFile.write("Laser "+name+" NOT PRESENT",true);
		isPresent=false;
		return;
	}
	on(1,lp->startingVoltage);
	logFile.write("Laser "+name+" ready",true);
	objEffs=vector<double>(cont.axio.objectives.size(),1);
	addObjEff(clp->eff100x,Magnification::getObjective("100xOil"));
	addObjEff(clp->eff63x,Magnification::getObjective("63xOil"));
}

string getRealLine(ifstream& protocol);

Params Laser::loadParams(std::string name){
	ConstLaserParams cp;
	LaserParams lp;
	vector<double> x,y;
	std::ifstream params;
	string filename=string(DEFAULTWORKINGDIR)+"calibrations\\"+name+".txt";
	params.open(filename.c_str(),ifstream::in);
	if (params.fail()){
		logFile.write(string("Error could not find laser parameters file: ")+filename,true);
		params.close();
		lp.res=false;
		return Params(new const ConstLaserParams(cp),new LaserParams(lp));
	}
	string line;
	string line2=getRealLine(params);
	//get last line
	while(line2!=""){
		line=line2;
		line2=getRealLine(params);
	}
	stringstream ss;
	ss<<line;
	try{
	getline(ss,line,',');//timepoint and max cal power
	getline(ss,line,',');
	cp.maxPower=toDouble(line);
	getline(ss,line,',');
	cp.wavelength=toDouble(line);
	getline(ss,line,',');
	lp.minV=toDouble(line);
	getline(ss,line,',');
	lp.maxV=toDouble(line);
	getline(ss,line,',');
	cp.triggerLine=toInt(line);
	getline(ss,line,',');
	cp.analogTriggerLine=toInt(line);
	getline(ss,line,',');
	cp.triggerOption=toInt(line);
	getline(ss,line,',');
	lp.startingVoltage=toDouble(line);
	getline(ss,line,',');
	cp.zeroIntensity=toDouble(line);
	getline(ss,line,',');
	cp.delay=toDouble(line);
	getline(ss,line,',');
	lp.defaultIntensity=toDouble(line);
	lp.units="mW";
	getline(ss,line,',');
	cp.calChan=removeWhite(line);
	getline(ss,line,',');
	lp.calTIRFangle=toDouble(line);
	getline(ss,line,',');
	cp.eff100x=toDouble(line);
	getline(ss,line,',');
	cp.eff63x=toDouble(line);
	getline(ss,line,',');
	lp.numCalSamples=toInt(line);
	}catch(std::ios_base::failure e){
		logFile.write("Could not parse laser parameters file: "+filename,true);
		lp.res= false;
		return Params(new const ConstLaserParams(cp),new LaserParams(lp));
	}
	if (ss.fail()){
		logFile.write("Could not parse laser parameters file: "+filename,true);
		lp.res= false;
		return Params(new const ConstLaserParams(cp),new LaserParams(lp));
	}
	int i=0;
	if (lp.numCalSamples==1)
		i=-1;//read one extra one
	try{
	for(;i<lp.numCalSamples;i++){
		getline(ss,line,',');
		x.push_back(toDouble(line));
		getline(ss,line,',');
		y.push_back(toDouble(line));
	}}catch(std::ios_base::failure e){
		x.clear();y.clear();
		logFile.write("Invalid calibration points in parameters file: "+filename+". Intensities will not be valid",true);
			x.push_back(lp.minV);
			y.push_back(0);
			x.push_back(lp.maxV);
			y.push_back(cp.maxPower);
	}
	if (lp.numCalSamples==0){
		x.push_back(lp.minV);
		y.push_back(0);
		x.push_back(lp.maxV);
		y.push_back(cp.maxPower);
	}
	if (ss.fail() || x.size()<2){
		x.clear();y.clear();
		logFile.write("Not enough calibration points in parameters file: "+filename+". Intensities will not be valid",true);
			x.push_back(lp.minV);
			y.push_back(0);
			x.push_back(lp.maxV);
			y.push_back(cp.maxPower);
	}
	params.close();
	lp.x=x;
	lp.y=y;
	sort(x,y);
	lp.xv=NRVec<DP>(x.size());
	lp.yp=NRVec<DP>(x.size());
	lp.yp2=NRVec<DP>(x.size());
	for(int i=0;i<x.size();i++){
		lp.xv[i]=x.at(i);
		lp.yp[i]=y.at(i);
	}
	NR::spline(lp.xv,lp.yp,1.0e30,1.0e30,lp.yp2);

	sort(y,x);
	lp.xp=NRVec<DP>(x.size());
	lp.yv=NRVec<DP>(x.size());
	lp.yv2=NRVec<DP>(x.size());
	for(int i=0;i<x.size();i++){
		lp.xp[i]=y.at(i);
		lp.yv[i]=x.at(i);
	}
	NR::spline(lp.xp,lp.yv,1.0e30,1.0e30,lp.yv2);

	if ((lp.startingVoltage>max(lp.maxV,lp.minV) || lp.startingVoltage<min(lp.minV,lp.maxV)) && lp.startingVoltage!=cp.zeroIntensity){
		logFile.write("Error in laser parameters file "+filename+": starting voltage out of range",true);
		lp.res= false;
		return Params(new const ConstLaserParams(cp),new LaserParams(lp));
	}
	//set LightSource variables accordingly


	double maxP;
	NR::splint(lp.xv,lp.yp,lp.yp2,lp.maxV,maxP);
	double minP;
	NR::splint(lp.xv,lp.yp,lp.yp2,lp.minV,minP);
	if (lp.defaultIntensity>maxP){
		lp.defaultIntensity=maxP;
		logFile.write("Laser "+name+" default power is too large. Setting to max",true);
	}
	if (lp.defaultIntensity<minP){
		lp.defaultIntensity=minP;
		logFile.write("Laser "+name+" default power is too small. Setting to min",true);
	}
	lp.res= true;
	return Params(new const ConstLaserParams(cp),new LaserParams(lp));
}


Laser::~Laser(){
	off();
}

double Laser::mW2Volts(double mW,Objective* obj){
	double eff=getObjEff(obj);
	mW=mW/eff;
	double v;
	NR::splint(lp->xp,lp->yv,lp->yv2,mW,v);
	return v;
}

void Laser::addObjEff(double eff, Objective* obj){
	if (obj==NULL)
		logFile.write(name+": cannot add objective efficiency. Objective not found",true);
		return;
	int i=cont.axio.getObjectiveIndex(obj);
	if (i==-1)
		logFile.write(name+" error: objective does not exist",true);
		return;
	objEffs.at(i)=eff;
}

double Laser::getObjEff(Objective* obj){
	if (obj==NULL)
		return 1;
	int i=cont.axio.getObjectiveIndex(obj);
	if (i==-1)
		return 1;
	return objEffs.at(i);
}

double Laser::volts2mW(double volts,Objective* obj){
	double mW;
	double eff=getObjEff(obj);
	NR::splint(lp->xv,lp->yp,lp->yp2,volts,mW);
	mW=mW*eff;
	return mW;
}

double Laser::maxPower(Objective* obj){
	return this->volts2mW(lp->maxV,obj);
}

double Laser::minPower(Objective* obj){
	return this->volts2mW(lp->minV,obj);
}

void Laser::on(int pos, double intensity){
	CheckExists()
	isValid(intensity);
	switch(cp->triggerOption){
		case 0://digital only
			t->setLineHigh(cp->triggerLine);
			break;
		case 1://analog only
			t->setVoltage(cp->analogTriggerLine,intensity);
			break;
		case 2://both
			t->setVoltage(cp->analogTriggerLine,intensity);
			t->setLineHigh(cp->triggerLine);
			break;
	}
}


string Laser::intensityToString(double intensity,Objective *obj){
	return toString(volts2mW(intensity,obj),1)+"mW";
}
double Laser::getIntensity(double intensity,string intensityUnit,Objective* obj){
	switch((int)intensity){
		case -1:
			return mW2Volts(lp->defaultIntensity,obj);
			break;
		case -2:
			return mW2Volts(maxPower(obj),obj);
			break;
		case -3:
			return mW2Volts(minPower(obj),obj);
			break;
		case -4:
			return (lp->maxV-lp->minV)/2.0;
			break;
	}
	if (intensityUnit==""){
		return getIntensity(intensity,"V");
	}
	if (intensityUnit=="mW"){
		if (intensity<0.99*minPower(obj)){//minV>maxV?
			logFile.write("Error: Laser power "+toString(intensity)+"mW must be >= "+toString(minPower(obj)),true); 
			return lp->minV;
		}
		if (intensity>1.01*maxPower(obj)){
			logFile.write("Error: Laser power "+toString(intensity)+"mW must be <= "+toString(maxPower(obj)),true); 
			return lp->maxV;
		}
		return mW2Volts(intensity,obj);
	}
	if (intensityUnit=="f"){
		return getIntensity(intensity*maxPower(obj),"mW",obj);
	}
	if (intensityUnit=="%"){
		return getIntensity(intensity*maxPower(obj)/100.0,"mW",obj);
	}
	if (intensityUnit=="V"){
		isValid(intensity);
		return intensity;
	}
	logFile.write("Error: Laser Intensity Unit "+intensityUnit+" not supported",true);
	return lp->defaultIntensity;
}

bool Laser::isValid(double& intensity){
	if (lp->minV>lp->maxV)
		if (intensity<lp->maxV){
			logFile.write(name+" invalid intensity. setting to max power");
			intensity=lp->maxV;
			return false;
		}else if (intensity>lp->minV && intensity!=clp->zeroIntensity){
			logFile.write(name+" invalid intensity. setting to min power");
			intensity=lp->minV;
			return false;
		}else
			return true;
	else 
		if (intensity>lp->maxV){
			logFile.write(name+" invalid intensity. setting to max power");
			intensity=lp->maxV;
			return false;
		}else if (intensity<lp->minV && intensity!=clp->zeroIntensity){
			logFile.write(name+" invalid intensity. setting to min power");
			intensity=lp->minV;
			return false;
		}else
			return true;
}

void Laser::off(){
	if (g)
		return;
	switch(cp->triggerOption){
		case 0://digital only
		case 2://both
			t->setLineLow(cp->triggerLine);
			break;
		case 1://analog only
			t->setVoltage(cp->analogTriggerLine,cp->zeroIntensity);
			break;
	}
}

void Laser::enterRingBuffer(AcquisitionGroup& channels){}

void Laser::exitRingBuffer(){}
/*bool Laser::isValidIntensity(double intensity){
	if (intensity>=minP && intensity<=maxP)
		return true;
	return false;
}
*/

double Laser::getPower(double intensity, string& units,Objective* obj){
	units="mW";
	return volts2mW(intensity,obj);
}
bool Laser::prepareCalChan(){
	CheckExists(false);
	int ind=Channel::get(cp->calChan);
	if (ind==-1){
		logFile.write("Cannot prepare for alignment. Specified channel "+cp->calChan+" does not exist.",true);
		return false;
	}
	Channel chan=cont.channels.at(ind);
	
	if (!g && chan.lite().ls!=this){
		logFile.write("Cannot prepare for alignment. Specified channel "+cp->calChan+" has a different light source "+chan.lite().ls->name+". Channel must use the same light source as this light source "+name,true);
		return false;
	}
	if (g && chan.lite().ls!=g){
		logFile.write("Cannot prepare for alignment. Specified channel "+cp->calChan+" has a different light source "+chan.lite().ls->name+". Channel must use the same light source as this light source "+g->name,true);
		return false;
	}
	
	if (g)
		chan.on(g->cp->zeroIntensity,true,lp->calTIRFangle);
	else
		chan.on(cp->zeroIntensity,true,lp->calTIRFangle);
	tempObj=cont.axio.getCurrentObjective();
	tempZ=cont.focus->getZ();
	cont.pm.setObjective();
	//cont.pm.setDarkCurrent();
	cont.pm.setWavelength(cp->wavelength);
	if (cont.OUT_BLOCKED!=-1)
		cont.outPorts.at(cont.OUT_BLOCKED).set();

	if (g){
		chan.on(g->getIntensity(1,"f"),true,lp->calTIRFangle);
	}
	
	return true;
}

void Laser::onAlign(){
	on(1,lp->maxV);
	wait();
}

void Laser::endCalChan(){
	on(1,lp->startingVoltage);
	if (g)
		g->off();
		//move back to where we were
	if (tempObj) tempObj->set();
	if (tempZ!=-1) cont.focus->move(tempZ);
}

void Laser::checkAlignment(double maxP){
	if (maxP<0.8*clp->maxPower)
		logFile.write(name+" max power has dropped by more than 20% max.  You should perform a realignment followed by a full recalibration",true); 
}

void Laser::checkCalibration(double p){
	if (p<0.8*maxPower(NULL) || p>1.2*maxPower(NULL)){
		logFile.write(name+" calibration is off by more than 20%.  You should perform a full recalibration",true); 
		cout<<endl<<"Do you want to perform a full recalibration now? (y or n)"<<endl;
		char c=getChar();
		if (c=='y')
			calibrate();
		else
			cout<<name<<" calibration skipped"<<endl<<endl;
	}
	else if (p<0.9*maxPower(NULL) || p>1.1*maxPower(NULL)){
		logFile.write(name+" calibration is off by more than 10%.  You should perform a quick recalibration",true);
		cout<<endl<<"Do you want to perform a quick recalibration now? (y or n)"<<endl;
		char c=getChar();
		if (c=='y')
			quickCalibrate();
		else
			cout<<name<<" calibration skipped"<<endl<<endl;
	}
}

double Laser::checkMaxPower(){
	CheckExists(1);
	if (lp->numCalSamples==0){
		logFile.write(name+" alignment not check becaused number of calibration samples is zero",true);
		return 1;
	}
	if (!prepareCalChan())
		return 1;
	onAlign();
	Timer::wait(1000);
	double p=cont.pm.readPower();
	endCalChan();
	logFile.write(name+" alignment check: max power is "+toString(p,1)+" mW out of"+toString(clp->maxPower,1)+" mW ("+toPercent(p/clp->maxPower)+")",true);
	checkAlignment(p);
	checkCalibration(p);

	return p/clp->maxPower;
}

void Laser::quickCalibrate(){
	CheckExists();
	if (lp->numCalSamples==0){
		logFile.write(name+" alignment not check becaused number of calibration samples is zero",true);
		return ;
	}
	if (!prepareCalChan())
		return;
	onAlign();
	Timer::wait(1000);
	double p=cont.pm.readPower();
	endCalChan();

	if (p<0.8*clp->maxPower)
		logFile.write(name+" max power has dropped by more than 20% max.  You should perform a realignment followed by a full recalibration",true); 
	
	//scale y accordingly;
	double scale=p/lp->y.back();
	for(vector<double>::iterator i=lp->y.begin();i!=lp->y.end();i++){
		*i=*i*scale;
	}

	updateCalibration();
	saveParams();
}

void Laser::calibrate(){
	CheckExists();
	if (lp->numCalSamples==0){
		logFile.write(name+" calibration skipped. Number of calibration samples is zero",true);
		return;
	}
	if (!prepareCalChan())
		return;

	Record calLog(string(DEFAULTWORKINGDIR)+"calibrations\\"+name+"calLog.txt");
	calLog.write("Voltage(V)\t\t,Power(mW)",false,"",false);
	double v,inc=0;
	lp->x.clear();
	lp->y.clear();
	if (lp->numCalSamples==1){
		lp->x.push_back(lp->minV);
		lp->y.push_back(0);
	}else{
		v=lp->minV;
		inc=(lp->maxV-lp->minV)/(lp->numCalSamples-1.0);
	}
	for(int i=0;i<lp->numCalSamples;v=v+inc){
		if (i==lp->numCalSamples-1)
			v=lp->maxV;//make it exact
		on(1,v);
		wait();
		Timer::wait(1000);
		lp->x.push_back(v);
		double p=cont.pm.readPower();
		lp->y.push_back(p);
		calLog.write(toString(v)+"\t\t,"+toString(p),false,"",false);
		i++;
	}
	endCalChan();
	updateCalibration();
	saveParams();
}

void Laser::saveParams(){
	//save to calibration data file
	string points;
	for(int i=0;i<lp->x.size();i++){
		points+=toString(lp->x.at(i))+","+toString(lp->y.at(i))+",";
	}
	ofstream params;
	params.open(string(string(DEFAULTWORKINGDIR)+"calibrations\\"+name+".txt").c_str(),ios::app);
	params<<endl<<Timer::getSysTime()<<"    Max is "<<toString(maxPower(NULL),1)<<" mW ("<<toPercent(maxPower(NULL)/clp->maxPower,1)<<")\t\t,"<<clp->maxPower<<","<<cp->wavelength<<","<<lp->minV<<","<<lp->maxV<<","<<cp->triggerLine<<","<<cp->analogTriggerLine<<","<<cp->triggerOption<<","<<lp->startingVoltage<<","<<cp->zeroIntensity<<","<<cp->delay<<","<<lp->defaultIntensity<<","<<cp->calChan<<","<<lp->calTIRFangle<<","<<clp->eff100x<<","<<clp->eff63x<<","<<lp->numCalSamples<<","<<points<<endl;
	params.close();
}

void Laser::updateCalibration(){
//usually x is increasing in order but this is not necessarily true (e.g. Galvo) y so we will sort it
	//copy so the order of lp->x and lp->y is always the same
	vector<double> x(lp->x);
	vector<double> y(lp->y);
	if (x.size()<2){
		x.clear();y.clear();
		logFile.write(name+"Error: Not enough calibration points. Intensities will not be valid",true);
			x.push_back(lp->minV);
			y.push_back(0);
			x.push_back(lp->maxV);
			y.push_back(clp->maxPower);
	}
	sort(x,y);
	lp->xv=NRVec<DP>(x.size());
	lp->yp=NRVec<DP>(x.size());
	lp->yp2=NRVec<DP>(x.size());
	for(int i=0;i<x.size();i++){
		lp->xv[i]=x.at(i);
		lp->yp[i]=y.at(i);
	}

	//sort for y because powers may not be increasing
	sort(y,x);
	lp->xp=NRVec<DP>(x.size());
	lp->yv=NRVec<DP>(x.size());
	lp->yv2=NRVec<DP>(x.size());
	for(int i=0;i<x.size();i++){
		lp->xp[i]=y.at(i);
		lp->yv[i]=x.at(i);
	}

		
	NR::spline(lp->xv,lp->yp,1.0e30,1.0e30,lp->yp2);
	
	NR::spline(lp->xp,lp->yv,1.0e30,1.0e30,lp->yv2);

	if (lp->defaultIntensity>maxPower(NULL)){
		lp->defaultIntensity=maxPower(NULL);
		logFile.write("Laser "+name+" default power is too large. Setting to max",true);
	}
	if (lp->defaultIntensity<minPower(NULL)){
		lp->defaultIntensity=minPower(NULL);
		logFile.write("Laser "+name+" default power is too small. Setting to min",true);
	}
	checkAlignment(maxPower(NULL));
}
