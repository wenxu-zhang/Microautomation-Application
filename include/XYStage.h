// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, April 11, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
class XYStage{

protected:
	XYStage(){}//this function should zero the stage to the center of the left chamber
int getChamber(int currentX,int currentY);//helper function
void enableLimits(bool isLeftChamber);//helper function true is left chamber false is right chamber
public:
	
	int chamberOffset;//in encoder steps
	int chamberWidth;//in encoder steps
	int chamberHeight;//in encoder steps
	int channelOffset;//in encoder steps


	virtual bool getIsPresent()=0;

	virtual ~XYStage()=0{}
	void move(int x,int y);

	/*
	void setXstep(double um){
		int x=um/stepSize;
		setXstep(x);
	}
	void setYstep(double um){
		int y=um/stepSize;
		setYstep(y);
	}
*/

	void incX(int dx){moveX(getX()+dx);}
	void incY(int dy){moveY(getY()+dy);}

	virtual void stop()=0;
	virtual void enableLimits(int minX,int minY,int maxX,int maxY)=0;
	virtual void disableLimits()=0;
	virtual void disableServo()=0;
	virtual void enableServo()=0;

	virtual void moveX(int x)=0;
	virtual void moveY(int y)=0;
	virtual int getX()=0;
	virtual int getY()=0;
	virtual void wait(double TimeoutMilliseconds=STAGETIMEOUT)=0;
	virtual bool home(int &x, int &y)=0;
	virtual bool isHomed()=0;//tell us whether the stage has been homed or not, this is a critical function for safety

	virtual void setXstep(int dx)=0;
	virtual void setYstep(int dy)=0;
	virtual void stepRight()=0;
	virtual void stepLeft()=0;
	virtual void stepDown()=0;
	virtual void stepUp()=0;
	virtual void setPosition(int x, int y)=0;

	virtual void setSpeed(int speed)=0;

	//return whether it was successful or not
	virtual bool homeStage(bool setZero=true,bool moveBack=true);

	//this moves the stage to the start of a section of the device
	//chamber: 0 is left 1 is right
	//channel: 0 is left 1 is middle 2 is right
	//section: 0 is bottom 1 is middle 2 is top
	void chamberMove(int chamber,int channel,int section);
	virtual double getStepSize()=0;//return step size in microns
	
	//GUI functions
	void stageControl();

	//virtual void test()=0;
	virtual void fineControl()=0;

private:
	
	
	};
