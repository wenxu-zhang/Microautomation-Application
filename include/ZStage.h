// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
class ZStage{
protected:
	ZStage(){}
public:
	virtual ~ZStage(){}
	virtual void move(double z)=0;
	virtual bool velocityMove(double umpsec)=0;
	virtual double getVelocityMoveStart()=0;
	virtual double getZ()=0;
	virtual void setMaxZ(double softLimitZ)=0;//tell us where to stop
	virtual double getMaxZ();//so we know where to stop
	virtual void stop(){}
	virtual void wait(){}
	void focusControl();
	virtual bool doesExist()=0;
};