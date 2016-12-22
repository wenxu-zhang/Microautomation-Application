// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "LightSource.h"
#include <string>
class Light{
public:
	Light(LightSource* ls,int position, std::string name):ls(ls),position(position),name(name){}
	LightSource* ls;
	int position;
	std::string name;
	void on(double intensity){ls->on(position,intensity);}
	void off(){ls->off();}
	void wait(){ls->wait();}
	bool operator==(Light &l){
		return this->ls==l.ls && this->position==l.position && this->name==l.name;
	}
	bool operator!=(Light &l){
		return (!(this->operator==(l)));
	}
};