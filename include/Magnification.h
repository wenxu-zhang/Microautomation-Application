// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Wednesday, June 22, 2011</lastedit>
// ===================================
#pragma once
#include "Objective.h"
#include "Optovar.h"
#include <string>
class Magnification{
public:
	Objective& obj;
	Optovar opt;
	Magnification(Objective& obj,Optovar& opt);
	bool operator==(Magnification &right);
	double get() const;
	void set() ;
	void wait();
	std::string toString() const;
	static Magnification* select();
	static Objective* getObjective(std::string str);
//	static Optovar getOptovar(std::string str);
	static Magnification* getMagnification(Objective& obj,Optovar& opt);
	static int getMagnification(std::string str);
};