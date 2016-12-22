// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Selection.h"

Selection::Selection(Changer* c,double position,std::string name):c(c),position(position),name(name){}

void Selection::set(){
	if (c)
		c->set(position);
}

void Selection::wait(){
	if (c)
		c->wait();
}