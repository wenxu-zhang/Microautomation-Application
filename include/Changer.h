// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
class Trigger;
class Changer{
public:
	virtual void set(double pos){}
	virtual double get(){return 1;}
	virtual void wait(){}
	virtual Trigger* getTrigger(){return 0;}
	virtual int getTriggerLine(){return -1;}
};