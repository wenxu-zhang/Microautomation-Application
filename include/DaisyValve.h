// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, November 15, 2011</created>
// <lastedit>Tuesday, December 13, 2011</lastedit>
// ===================================
#pragma once
class Valve;
class Solution;
#include "Windows.h"
#include <string>
class DaisyValve{
public:
	DaisyValve(Valve* v1=NULL,Valve* v2=NULL);//not checked
	DaisyValve(Valve* v1,Valve* v2,int daisyPort,double daisyInjectTubingVol,int outPort=-1,double injectTubingVol=-1);//may throw exception
	//bool operator==(const DaisyValve& dv);
	double injectTubingVol;//for push daisy valve this is the tubing volume from v1 to syringe, for pull daisy valve this is the tubing volume from v2 to waste
	double daisyInjectTubingVol;//additional volume for 2nd valve;
	int daisyPort;
	int outPort;//port on last valve going to chamber for pushDaisyValve;
	Valve* v1;
	Valve* v2;
	double getInjectTubingVolPull(int valveNum);
	double getInjectTubingVolPush(int valveNum);
	bool isValid(int valveNum, int valvePos);
	void select(Solution* s);//int valveNum,int valvePos);
	void selectOut();//only valid for pushDaisyValve;
	void parse(std::string line);
};
