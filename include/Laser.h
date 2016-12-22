// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#pragma once
#include "Trigger.h"
#include "LightSource.h"

class LaserParams:public NonConstParams{
public:
	double minV;
	double maxV;
	double startingVoltage;
	//deprecated now stoed as default intensity/units in LightSource NonConstParams double defaultPower;
	//double slope;
	//double yintercept;
	bool res;//successfully loaded?
	double calTIRFangle;
	vector<double> x;
	vector<double> y;
	NRVec<DP> xp;//powers as input sorted
	NRVec<DP> yv;//voltages as output sorted
	NRVec<DP> xv;//voltages as input
	NRVec<DP> yp;//powers   as output
	NRVec<DP> yv2;//2nd derivates of voltages as output
	NRVec<DP> yp2;//2nd derivates of powers as output
	int numCalSamples;
};

class ConstLaserParams:public ConstParams{
public:
	double eff100x;
	double eff63x;
	double maxPower;//power measured after last alignment
};

class Galvo;
class Laser:public LightSource{
public:
	static const int MAXPOWER=-2;
	static const int MINPOWER=-3;
	static const int MEDPOWER=-4;
	void on(int pos,double intensity);
	void off();
	//void wait();
	void enterRingBuffer(AcquisitionGroup& channels);
	void exitRingBuffer();
	std::string intensityToString(double intensity,Objective* obj=NULL);
	double getPower(double intensity, string& units,Objective* obj=NULL);
	double getIntensity(double intensity=-1,std::string intensityUnit="",Objective* obj=NULL);
	//bool isValidIntensity(double intensity);
	Laser(Trigger* t,std::string name,Galvo* g=NULL);
//	Laser(Trigger* t,ConstParams cp);
	~Laser();
	bool isValid(double& intensity);
	double mW2Volts(double mW,Objective* obj);
	double volts2mW(double volts,Objective* obj);
	double maxPower(Objective* obj);
	double minPower(Objective* obj);
	//LaserParams* lp(){return lp;}
	//ConstLaserParams* clp(){return (ConstLaserParams*)cp;}
	LaserParams* lp;
	const ConstLaserParams* const clp;
	Galvo* g;
	static Params loadParams(std::string name);
	void quickCalibrate();
	void calibrate();
	double checkMaxPower();
	bool prepareCalChan();
	void onAlign();
	void endCalChan();
	vector<double> objEffs;//one for each objective;
	void addObjEff(double eff, Objective* obj);
	double getObjEff(Objective* obj);
private:
	void saveParams();
	void updateCalibration();
	double tempZ;
	Objective* tempObj;
	void checkAlignment(double maxP);
	void checkCalibration(double maxP);
};
