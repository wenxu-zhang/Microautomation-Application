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
class Camera;

//container class so the same camera can be used on multiple outports;
class OutPort{
public:
	OutPort(Selection& s,Camera* cam);
	Selection s;
	Camera* cam;
	void set();
	void wait();
	std:: string toString() const;
	static OutPort* select();
};