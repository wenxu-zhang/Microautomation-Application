// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, March 08, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include <string>
#include "CUsbCode.h"
#include "CUsbCodeVCOM.h"
#include "XYStage.h"

class Ludl:public XYStage{

public:
	void loadCenterPosition(int val, int deviceID);
	int getCenterPosition(int deviceID);
	unsigned char pStr[256];
	std::string checkErrorHighLevel(double msTimeout=10000);
	void enableHighLevel();
	void enableLowLevel();
	//implementation of abstract XYStage Class
	void moveY(int y); //move to position y with the y axis, x position remains the same
	void moveX(int x);//move to position x with the x axis, y position remains the same
	int getX();
	int getY();
	void wait(double TimeoutMilliseconds=STAGETIMEOUT);//return when the stage has finish moving
	bool home(int& x, int& y);
	bool isHomed(){return bIsHomed;}
	bool _isHomed();
	bool bIsHomed;
	void disableCounterCapture(int deviceID);
	void enableCounterCapture(int deviceID);
	void center(int deviceID,double speed);
	void readJoystickSpeed(double& normal, double& faster);
	void setJoystickSpeed(double normal, double faster);
	int  readMotorRes();
	void home2();
	void setXstep(int dx){setXinc(dx);}
	void setYstep(int dy){setYinc(dy);}
	void stepRight(){decX();}
	void stepLeft(){incX();}
	void stepDown(){decY();}
	void stepUp(){incY();}
	double getStepSize();
	void enableLimits(int minX,int minY,int maxX,int maxY);
	void disableLimits();
	void stop();
	void fineControl();
	void setPosition(int x,int y);

	void waitServo(int dev);

	bool isPresent;
	bool getIsPresent(){return isPresent;}
	Ludl(); //initialize USB communications and verify axis x and axis y are present.  Establish zero position and go there.
			//zero position will always be identified by placing the objective on the upper left corner of the sample window.
			//x axis position will increase as the objective moves to the right on the sample (stage moving to the left).
			//y axis position will increase as the objective moves down on the sample (stage moving up).
	~Ludl(); // return to zero position and then close USB communications
	//void move(int x, int y){moveX(x);moveY(y);}; //move to position x with the x axis and position y with the y axis.
	void gotoEndLimit(int deviceID,int Speed);
	void checkStats();
	void MotorRes(int res);
	void getVersion();
	double getOverlap();
	int res;
	void XStepper(int x);
	void YStepper(int y);
	void disableServo();
	void enableServo();
	bool servoEnabled();
	bool servo;//need to keep track of the servo mode cause the controller won't tell us
	void setServoDist(unsigned char i);
	void setAccel(int accel);
	void setSpeed(int speed);
	void setServoSpeed(int speed);
	/* Implement some scanning features here.  possibly set upperleft set lowerright, step field etc.
	advanceRight(int obj); //move 
	advanceDown(int obj);
	*/
	void incX();
	void decX();
	void setXinc(int dx);
	void setYinc(int dy);
	void incY();
	void decY();
	void incX(int dx);//increse x axis position by um micrometers  um could be negative
	void incY(int dy);//increase y axis position by um micrometers  um could be negative

private:
	double overlap;//defined in Controller

	int xAxisID; //X axis object
	int yAxisID; //Y axis object
	//CUsbCodeVCOM CUsb;//USB communications object
	CUsbCode CUsb;
};