// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Thursday, November 17, 2011</created>
// <lastedit>Thursday, March 15, 2012</lastedit>
// ===================================
#include "Solution.h"
class Reagent:virtual public Solution{
public:
	//virtual Solution* clone(FluidicsSetup& fs)=0;


	//virtual void load(FlowChannel* chan)=0;
	//virtual FlowChannel* prime(FlowChannel* chan)=0;//leave air gap
	virtual std::string toString();
protected:
	Reagent(SolutionData& sd);//could potentially throw exception
};

class PushReagent:public PushSolution,public Reagent{
public:
	void load(FlowChannel* chan,double param=-1, double time=-1);
	FlowChannel* prime(FlowChannel* chan);//leave air gap
	std::string toString(){return "Push"+Reagent::toString();}
	Solution* clone(FluidicsSetup& fs);
	PushReagent(SolutionData& sd);
	void coarseCalibrateInlet();
	void calibrateInlet();
	void calibrateDV();//daisyInjectTubingVolume
	void coarseCalibrateDV();
	void calibrateDV2();//injectTubingVolume
	void coarseCalibrateDV2();
	void coarseCalibrateFlowChannel(FlowChannel* chan);
	void calibrateFlowChannel(FlowChannel* chan);
	void calibrateFlowChannel2(FlowChannel* chan);
	void calibrateTubingDiameter();
	FlowSol clean(double inletFactor,double uLps,FlowChannel* f=NULL,Solution* s=NULL);
};

class PullReagent:public PullSolution,public Reagent{
public:
	void load(FlowChannel* chan,double param=-1, double time=-1);
	FlowChannel* prime(FlowChannel* chan);//leave air gap
	std::string toString(){return "Pull"+Reagent::toString();}
	Solution* clone(FluidicsSetup& fs);
	PullReagent(SolutionData& sd);
	void coarseCalibrateInlet();
	void calibrateInlet();
	void calibrateDV();//daisyInjectTubingVolume
	void coarseCalibrateDV();
	void calibrateDV2();//injectTubingVolume
	void coarseCalibrateDV2();
	void coarseCalibrateFlowChannel(FlowChannel* chan);
	void calibrateFlowChannel(FlowChannel* chan);
};
