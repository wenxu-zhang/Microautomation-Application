// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Selection.h"
#include <string>

class Optovar{
public:
	Optovar(double mag,Changer* c, int position, std::string name);
	Selection s;
	double mag;
	void set() ;
	void wait() ;
	bool operator==(Optovar right);
};