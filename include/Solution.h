// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Saturday, April 07, 2012</lastedit>
// ===================================
/*
Solution: Either reagent or wash solution. A solution is unique when given a tag
		Variables:
			bool recyclable; //if recyclable then the reagent will be returned before next washing or reagent filling.  If not recyclable the reagent volume should only be as large as the minimum required to fill the chamber given the overshoot. 
			int tag; //index for this solution type (reagent or wash) starting from 0 (should be labeled on tubing e.g. W0 R1 etc.)
			bool reagent; //true if reagent, false if wash
			valve port position
			valve number
			tubing volume to valve
			name
			fluid velocity
		Methods:
		-Constructor:  
			Input:	int tag
				bool reagent
				valve port position
				valve number (1 or 2)
				tubing volume to valve
				Name
				fluid velocity.  Defaults for reagents and washes are specified in control GUI
		-modify:  GUI to adjust internal data of Solution
		-toString:  displays pertinent information
*/
#pragma once
#include <string>
#include <vector>
class FlowChannel;
class Valve;
class Syringe;
#include "FluidicsSetup.h"
#include "DaisyValve.h"
class Solution;
#include "Record.h"
extern Record logFile;
class SolutionData{
public:
	SolutionData():isInletPrimed(false),dv(),valvePos(-1),valveNum(-1),inletTubingVol(-1),loadFactor(-1),uLps(-1),name(""),s(NULL),syringePort(-1),wash(NULL),air(NULL),waste(NULL),remainingLoadVolume(0),chan(NULL){}
	//class variables
	bool isInletPrimed;		//is tubing from bulk into valve primed
	DaisyValve dv;
	//int solValve;
	int valvePos;
	int valveNum;
	double inletTubingVol;
	double loadFactor;
	double uLps;		//flow rate
	std::string name;

	//used for push solutions
	Syringe* s;//so we know what to use for priming
	int syringePort;

	//used for Reagent
	Solution* wash;
	Solution* air;
	double remainingLoadVolume;//after fractional load what is needed to complete load
	FlowChannel* chan;//for completion of fractional loading
	Solution* waste;//necessary to clean up after priming
	//parsing functions
	
	void parsePushSolution(std::string s);
	void parsePullSolution(std::string s){parseSolution(s);}

	void parsePushReagent(std::string s,FluidicsSetup& fs);
	void parsePullReagent(std::string s,FluidicsSetup& fs);
private:
	void parseSolution(std::string);
	void parseReagent(std::string,FluidicsSetup& fs);
};
class Solution;
class FlowChannel;
struct FlowSol{
	FlowChannel* f;
	Solution* s;
	FlowSol():f(NULL),s(NULL){}
};
class Solution{

public:
	virtual ~Solution(){}
	SolutionData sd;
	void setFlowVelocity(double mmps,Chamber* c);
	void setFlowRate(double uLps);
	
	virtual bool conflict(Solution& s);
	//methods
	virtual Solution* clone(FluidicsSetup &fs)=0;
	//double getTubingLoadVolume();
	virtual double getInjectTubingVolume()=0;//volume from 
	virtual FlowChannel* prime(FlowChannel* chan=NULL)=0;//gui version asks for flow channel if necessary (i.e. pull solution and chan==NULL) returns used flowchannel or NULL if none was necessary
	virtual void valveSelect();
	virtual std::string toString();
	virtual void load(FlowChannel* chan,double param=-1, double time=-1)=0;//will be the wash function in Solution class
	//virtual double getRemainingLoadVolume();//used by reagent to complete a partial loading using the load function with param!=-1
	virtual void coarseCalibrateInlet(){logFile.write("Function not supported",true);}
	virtual void calibrateInlet(){logFile.write("Function not supported",true);}
	virtual void calibrateDV(){logFile.write("Function not supported",true);}//daisyInjectTubingVolume
	virtual void coarseCalibrateDV(){logFile.write("Function not supported",true);}
	virtual void calibrateDV2(){logFile.write("Function not supported",true);}//injectTubingVolume
	virtual void coarseCalibrateDV2(){logFile.write("Function not supported",true);}
	virtual void coarseCalibrateFlowChannel(FlowChannel* chan){logFile.write("Function not supported",true);}
	virtual void calibrateFlowChannel(FlowChannel* chan){logFile.write("Function not supported",true);}
	virtual void calibrateFlowChannel2(FlowChannel* chan){logFile.write("Function not supported",true);}
	virtual void pull(double vol=-1, FlowChannel* f=NULL, double uLps=-1){logFile.write("Function not supported",true);} 
	virtual void calibrateTubingDiameter(){logFile.write("Function not supported",true);}
	virtual FlowSol clean(double inletFactor, double uLps, FlowChannel* chan=NULL, Solution* s=NULL){logFile.write("Function not supported",true);return FlowSol();}
	virtual void push(double vol=-1, FlowChannel* f=NULL, double uLps=-1){logFile.write("Function not supported",true);} 
protected:
	
	std::string toString_();
	Solution(SolutionData& sd); //a real solution that can throw exception
};

class PullSolution:virtual public Solution{
public:
	virtual double getInjectTubingVolume();
	virtual FlowChannel* prime(FlowChannel* chan);
	//void valveSelect();
	virtual std::string toString(){return "Pull"+Solution::toString();}
	virtual void load(FlowChannel* chan,double param=-1, double time=-1);//will be the wash function in Solution class
	PullSolution(SolutionData& sd);//a real solution that can throw exception
	virtual Solution* clone(FluidicsSetup &fs);
	//virtual void calibrateInlet();
	//virtual void coarseCalibrateInlet();
};


class PushSolution:virtual public Solution{
public:
	

	virtual double getInjectTubingVolume();
	virtual FlowChannel* prime(FlowChannel* chan);//if chan!=NULL generates warning that flow channel will be ignored
	virtual void valveSelect();
	virtual std::string toString(){return "Push"+Solution::toString();}
	virtual void load(FlowChannel* chan,double param=-1, double time=-1);//will be the wash function in Solution class
	PushSolution(SolutionData& sd); //a real solution that can throw exception
	virtual Solution* clone(FluidicsSetup &fs);
	//virtual void calibrateInlet();
	//virtual void coarseCalibrateInlet();
	void pull(double vol,FlowChannel* f=NULL, double uLps=-1);
	void push(double vol,FlowChannel* f=NULL, double uLps=-1);
	virtual FlowSol clean(double inletFactor,double uLps,FlowChannel* f=NULL,Solution* s=NULL);
};




