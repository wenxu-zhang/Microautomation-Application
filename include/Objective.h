// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, May 25, 2011</lastedit>
// ===================================
#pragma once
#include "Selection.h"
#include <iostream>
#include <string>

class Objective{
public:
	Selection s;
	static bool cleanOil;	
	Objective():mag(1),dof(0),wd(1000),isOil(false),s(){}
	Objective(double mag,double dof, double wd,bool isOil, Changer* c, int position, std::string name);
	bool isOil;
	bool needsOil;
	double mag;
	double dof;//depth of focus
	double wd;//working distance
	double getDOF() const;
	double getWorkingDist() const;
	void set();
	void wait();
	std::string toString();
};

