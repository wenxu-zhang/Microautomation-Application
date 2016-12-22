// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#pragma once
//#include "LightSource.h"
#include "Light.h"
#include "Selection.h"
#include "OutPort.h"
#include <string>
#include <vector>
class Objective;

class Channel{
public:
	std::string name;//official unique name identifier (i.e. reflector cube name and, if applicable, the imaging dichroic being used)
	std::vector<std::vector<std::string>> alternates;//one for each supported outport. image names for each output go from low wavelength (i.e. reflected or donor) to high wavelength (i.e. transmitted or acceptor)
	std::vector<double> defaultTIRFangles;//one for each objective, can be overridden by acquisition channel tirf angle 
	bool isOutportSupported(OutPort& out);
	OutPort* getDefaultOutport();
	std::string desc;
	Light lite(int n=-1);
	Changer* tirf;
	Selection exFilter,reflector,emFilter;
	Channel(std::string name, std::string desc, Light lite,Selection reflector,Changer* tirf=NULL,Selection exFilter=Selection(),Selection emFilter=Selection());//fully defines a channel with an arbitrary setup
	Channel(std::string name, std::string desc, std::vector<Light> lites,Selection reflector,Changer* tirf=NULL,Selection exFilter=Selection(),Selection emFilter=Selection());//fully defines a channel with an arbitrary setup
	~Channel();
	void on(double intensity,bool wait=true,double TIRFangle=-20) ;
	void on(std::vector<double>& intensity,bool wait=true,double TIRFangle=-20);
	void wait();
	void off() ;
	std::string toString(OutPort* out=NULL, int n=-1) const;
	static Channel* select();
	static int get(std::string name);
	std::vector<Light> lites;//all lites that need to be triggered for this lite source
	void addAlternateName(OutPort&, std::vector<std::string> alt);
	void denyOutport(OutPort& deniedPort);//add altermate name "Deny" to prevent use
	void addTIRFangle(double angle,Objective* obj);
	double getTIRFangle(Objective* obj);
	OutPort* prefOut;//preferred outport for this channel if any
private:
	
	

	//The following records all of the light sources so we can turn them all off to ensure only a single channel gets turned on.
	//static bool addLightSource(Light lite);
};


