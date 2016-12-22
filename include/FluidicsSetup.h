// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, December 19, 2011</lastedit>
// ===================================

#pragma once
#include <string>
#include <vector>
#include <set>
class Valve;
class Solution;
class Reagent;
class FlowChannel;
class Syringe;
#include "Chamber.h"


class FluidicsSetup{
public:
	FluidicsSetup(){}
	FluidicsSetup(const FluidicsSetup& fs);
	FluidicsSetup& operator=(const FluidicsSetup& fs);
	~FluidicsSetup();

	bool addSolution(Solution& s);
	bool addReagent(Solution& s);
	bool addChannel(FlowChannel& f);
	Solution* getSolution(std::string name);
	Solution* getReagent(std::string name);
	FlowChannel* getChannel(std::string name);
	Solution* selectReagent();
	std::vector<Solution*> selectReagents();
	Solution* selectSolution();
	std::vector<Solution*> selectSolutions();
	Solution* selectSolutionReagent();
	FlowChannel* selectChannel();
	std::vector<FlowChannel*> selectChannels();
	void primeAll(std::string wash,std::string chan);
	std::string toString();
	void fluidicsControl();
	void calibrateControl();
	void testWaitError();
	//DaisyValve dv;
	
	std::vector<Solution*> solutions;
	std::vector<Solution*> reagents;
	std::vector<FlowChannel*> channels;

private:
	


};

