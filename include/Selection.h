// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Changer.h"
#include <string>

class Selection{
public:
	Selection(Changer* c=NULL,double position=0,std::string name="");
	Changer* c;
	double position;
	std::string name;
	void set() ;
	//	void get()(return position;)  why do we need this
	void wait() ;
	bool operator==(const Selection& sel){
		return (c==sel.c && position==sel.position);
	}
};