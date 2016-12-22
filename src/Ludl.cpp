// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, March 08, 2011</created>
// <lastedit>Monday, April 11, 2011</lastedit>
// ===================================
// ===================================
#include "Warn.h"
#include "Ludl.h"
#include <iostream>
//#include "CUsbCode.h"

#include "Controller.h"
#include <limits>
#include <conio.h>
#include <iomanip>
using namespace std;
extern Controller cont;

void Ludl::enableHighLevel(){
	unsigned char str[2];
	// Place USB controller in highlevel mode
			//Timer::wait(2000);
			//system("pause");
			str[0] = (char)0xFF;
			str[1] = (char)65;
			CUsb.Write((unsigned char*)str,2);
			//Timer::wait(5000);
			CUsb.DumpRx();
}

void Ludl::enableLowLevel(){
	// Place USB controller in lowlevel mode
			//Timer::wait(2000);
			//system("pause");
	unsigned char str[2];
			str[0] = (char)0xFF;
			str[1] = (char)66;
			CUsb.Write((unsigned char*)str,2);
			//Timer::wait(5000);
			CUsb.DumpRx();
}

Ludl::Ludl(){
	isPresent=true;
	overlap=10;//10 percent overlap

	string Devices[127];
	int found = CUsb.Scan( Devices );	// scans for all the device matching the guid
	if( !found)	{
		cout<<"Ludl NOT PRESENT"<<endl;
		isPresent=false;
		return;
	}

	if( CUsb.Open(Devices[0])  ){		// try to open the first device
			
			//dump rx buffer if any
			unsigned char str[256];
			//get version
			enableHighLevel();
			strcpy((char*)str,string("VER\r").c_str());
			CUsb.Write(str,4);
			Timer::wait(2000);
			DWORD i=CUsb.GetLen();
			if (i>255) i=255;
			CUsb.Read(str,i);
			str[i]=0;
			logFile.write(string("Ludl ")+string((char*)str),true);
			bIsHomed=_isHomed();
			enableLowLevel();
			xAxisID=1; //not sure if these are correct
			yAxisID=2; //may need to query for the device names
			}else{
				cout<<"Unable to open the USB device: USB Error"<<endl;
				system("pause");
				exit(1);
			}

	double normal,faster;
	readJoystickSpeed(normal,faster);
	int currentRes=readMotorRes();
	
	//THIS WILL AFFECT SPEED BUT NOT POSITION
	MotorRes(MOTORRESOLUTION);
	setJoystickSpeed(JOYSTICKSLOW,JOYSTICKFAST);
	
	readJoystickSpeed(normal,faster);
/*
	buf[0]=xAxisID;
	buf[1]=130;
	buf[2]=1;
	buf[3]=':';
	CUsb.Write(buf,4);
	CUsb.Read(buf,1);
	//cout<<"default x axis settle time= "<<(unsigned int)((unsigned char)buf[0])<<endl;

	buf[0]=yAxisID;
	buf[1]=130;
	buf[2]=1;
	buf[3]=':';
	CUsb.Write(buf,4);
	CUsb.Read(buf,1);
	//cout<<"default y axis settle time= "<<(unsigned int)((unsigned char)buf[0])<<endl;
*/
	//cout<<"setting it to auto settling time (255)"<<endl;
	
	pStr[0]=xAxisID;
	pStr[1]=95;
	pStr[2]=1;
	pStr[3]=255;
	pStr[4]=':';
	CUsb.Write(pStr,5);
	pStr[0]=yAxisID;
	CUsb.Write(pStr,5);

	/*
	buf[0]=xAxisID;
	buf[1]=130;
	buf[2]=1;
	buf[3]=':';
	CUsb.Write(buf,4);
	CUsb.Read(buf,1);
	//cout<<"x axis settle time= "<<(unsigned int)((unsigned char)buf[0])<<endl;

	buf[0]=yAxisID;
	buf[1]=130;
	buf[2]=1;
	buf[3]=':';
	CUsb.Write(buf,4);
	CUsb.Read(buf,1);
	//cout<<"y axis settle time= "<<(unsigned int)((unsigned char)buf[0])<<endl;
	*/
	/*
	//NOTE:  IF TOO MUCH VIBRATION DURING IMAGING WE MAY NEED TO ADJUST SOME OF THESE PARAMETERS
	//set starting speed to maximum-->this should eliminate the ramp period
	unsigned char pStr[16];
	pStr[0] = xAxisID;	// device #
	pStr[1] = 82;		// set accel
	pStr[2] = 1;
	pStr[3] = (unsigned char) 65534;
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
	pStr[0] = yAxisID;
	CUsb.Write(pStr,5);
*/
	setAccel(ACCELERATION);//this corresponds to the minimum acceleration time, this may need to be set to 1 (the specifications don't specify)//default would be 20...
	//BUT according to the specifications if we set the start speed greater than the max speed there should be no ramp period and hence no acceleration time (max acceleration)
	setSpeed(MAXVELOCITY);//2764800);//2764800;//this is the maximum value we can send
	//checkStats();
	
	setServoDist(0);
	setServoSpeed(1250);//max is 1250
	servo=true;
	disableServo();
	disableLimits();
//	wait();
	//checkStats();
	//zero the Ludl
	//Home();
	//CUsb.SetPosition(xAxisID,0);
	//CUsb.SetPosition(yAxisID,0);
	//cout<<"Ludl initialized and default velocities and accelerations set"<<endl;
//218123
	//-928165
	double stepSize=getStepSize();
	chamberOffset=STAGECHAMBEROFFSET*1000.0/stepSize;
	chamberWidth=STAGECHAMBERWIDTH*1000.0/stepSize;
	chamberHeight=STAGECHAMBERHEIGHT*1000.0/stepSize;
	channelOffset=STAGECHANNELOFFSET*1000.0/stepSize;
	enableServo();
#ifdef XYLIMITS
		double z=cont.focus->getZ();
	char b;
STAGECALIBRATE:	cout<<"Calibrate stage (y or n)?"<<endl;
	cin>>b;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (b=='n'){
		//enableServo();
		//cout<<"Ludl ready"<<endl;
		goto ENABLESTAGELIMITS;
	}else if (b!='y')
		goto STAGECALIBRATE;

	cont.focus->move(0);
	cont.focus->wait();
#ifdef OBSERVER
	cout<<"Raise Condenser Arm for homing"<<endl;
	system("pause");
#endif
	home();
ENABLESTAGELIMITS:
	int currentX=getX();
	int currentY=getY();
	int chamber=getChamber(currentX,currentY);
	switch(chamber){
			case 0://left
				XYStage::enableLimits(true);
				break;
			case 1://right
				XYStage::enableLimits(false);
				break;
			case -1://no chamber
				move(0,0);
				wait();
				XYStage::enableLimits(true);
				break;
	}
	cont.focus->move(z);
#endif
	if (isHomed())
		logFile.write("Ludl Stage homing has already been performed",true);
	else{
		cout<<"Stage homing has not been performed since last power cycle.\n Perform now? (y or n)...   ";
		while(true){
			char in=::getChar();
			if (in=='y'){
				if (!homeStage()){
					cout<<"Ludl NOT PRESENT"<<endl;
					disableServo();
					stop();
					disableLimits();
					CUsb.Close();
					isPresent=false;
					return;
				}
				break;
			}
			if (in=='n')
				break;
			cout<<"please enter 'y' or 'n'...   ";
		}
	}
	
	getVersion();
	cout<<"Ludl ready"<<endl;
}

bool Ludl::_isHomed(){
	CheckExists(false)
	enableHighLevel();
	pStr[0]='i';pStr[1]='s';pStr[2]='t';pStr[3]='a';pStr[4]='t';pStr[5]='\r';
	CUsb.Write(pStr,6);
	string s=checkErrorHighLevel();
	int val=toInt(s);
	bool ret;
	if (val==1)
		ret=true;
	else
		ret=false;
	return ret;
	enableLowLevel();
}

void Ludl::setSpeed(int speed){
	CheckExists()
	CUsb.SetVelocity(xAxisID,speed);
	//cout<<"X velocity is: "<<CUsb.GetVelocity(xAxisID)<<endl;
	CUsb.SetVelocity(yAxisID,speed);
}
void Ludl::setAccel(int accel){
	CheckExists()
	CUsb.SetAccel(xAxisID,accel); 
	//cout<<"X acceleration is: "<<CUsb.GetAccel(xAxisID)<<endl;
	CUsb.SetAccel(yAxisID,accel);
}

void Ludl::getVersion(){
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 127;		// read resolution	
	pStr[2] = 6;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr,6);

	logFile.write(string("Ludl Version: Month is ")
		+toString(int(pStr[1]))
		+" Day is "
		+toString(int(pStr[2]))
		+" Year is "
		+toString(int(pStr[3]))
		+" Version is "
		+ toString(int(pStr[4])),true);
}

void Ludl::setServoSpeed(int speed){
	//set servo speed
	CheckExists()
	int servoSpeed=65536-(5529600/speed);//1250 pulse per second is max servo speed
	unsigned char buf[16];
	buf[0]=xAxisID;
	buf[1]=93;
	buf[2]=2;
	buf[3]=(0x0000FF&servoSpeed)>>0;
	buf[4]=(0x00FF00&servoSpeed)>>8;
	buf[5]=':';
	CUsb.Write(buf,6);
	buf[0]=yAxisID;
	CUsb.Write(buf,6);
}
void Ludl::waitServo(int axis){
	CheckExists()
		Timer t(true);
	unsigned char buf[16];
	unsigned char result[16];
	int status;
	buf[0]=axis;
	buf[1]=128;
	buf[2]=1;
	buf[3]=':';
	while(true){
		CUsb.Write(buf,4);
		CUsb.Read(result,1);
		if (!(result[0]&0x00000002)) break;	
		if (t.getTime()>STAGETIMEOUT){
			logFile.write(string("Error: Ludl wait for servo took more than ")+toString(STAGETIMEOUT/1000.0)+" seconds",true);
			break;
		}
	}
}
void Ludl::setServoDist(unsigned char i){
	//set servo activation distance
	CheckExists()
	unsigned char buf[16];
	buf[0]=xAxisID;
	buf[1]=79;
	buf[2]=3;
	buf[3]=i;
	buf[4]=0;
	buf[5]=0;
	buf[6]=':';
	CUsb.Write(buf,7);
	buf[0]=yAxisID;
	CUsb.Write(buf,7);
}

bool Ludl::servoEnabled(){
	return servo;
}

void Ludl::enableServo(){
	CheckExists()
	if (servo) return;
	servo=true;
	unsigned char buf[16];
	buf[0]=xAxisID;
	buf[1]=77;//77enables 78disables
	buf[2]=0;
	buf[3]=':';
	CUsb.Write(buf,4);
	buf[0]=yAxisID;
	CUsb.Write(buf,4);
}
void Ludl::disableServo(){
	CheckExists()
	if (!servo) return;
	servo=false;
	unsigned char buf[16];
	buf[0]=xAxisID;
	buf[1]=78;//77enables 78disables
	buf[2]=0;
	buf[3]=':';
	CUsb.Write(buf,4);
	buf[0]=yAxisID;
	CUsb.Write(buf,4);
}
Ludl::~Ludl(){
	CheckExists()
	double normal,faster;
	readJoystickSpeed(normal,faster);
	//MotorRes(20000);//10000 is the default
	//setJoystickSpeed(55848.484848484848,460750.00000000000);
	disableServo();
	stop();
	disableLimits();
	CUsb.Close();
}

double Ludl::getOverlap(){
	return overlap;
}

//return step size in um
double Ludl::getStepSize(){
	return 0.1;//the linear encoder on the stage has 100nm resolution.  This is independent of the resolution of the stepper motor which is deafault 10,000steps per revolution and we use 40,000steps per revolution to get better settling times
}

void Ludl::checkStats(){
	CheckExists()
	CUsb.DumpRx();
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 205;		// smart chip	
	pStr[2] = 40;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	cout<<"Num Read:"<<CUsb.Read(pStr,40)<<endl;
	cout<<"X Motor Type 1=Stepper 200FS/R  2=400FS/R: "<<(unsigned int) pStr[33]<<endl;
	unsigned short int range=(0x00FF&((unsigned short int)pStr[26])) | (0xFF00&(((unsigned short int)pStr[27])<<8));
	cout<<"X axis travel range: "<<range<<endl;
	unsigned short int pitch=(0x00FF&((unsigned short int)pStr[28])) | (0xFF00&(((unsigned short int)pStr[29])<<8));
	cout<<"X leadscrew pitch: "<<pitch<<endl;
	cout<<"X Encoder Type (5 is linear encoder) "<<((unsigned int) pStr[30])<<endl;
	unsigned short int resol=(0x00FF&((unsigned short int)pStr[31])) | (0xFF00&(((unsigned short int)pStr[32])<<8));
	cout<<"X resolution (steps/rev) or (steps/mm) probably the latter because it is an xystage	: "<<resol<<endl;
	cout<<"X Aux Encoder Type (0=none 1=20000rotart 2=10000rotary 3=4000 4=2000 5=linear): "<<(unsigned int)pStr[34]<<endl;
	resol=(0x00FF&((unsigned short int)pStr[35])) | (0xFF00&(((unsigned short int)pStr[36])<<8));
	cout<<"X Aux resolution (steps/rev) or (steps/mm) probably the latter because it is an xystage	: "<<resol<<endl;
	
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 205;		// smart chip	
	pStr[2] = 40;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	cout<<"Num Read:"<<CUsb.Read(pStr,40)<<endl;
	cout<<"Y Motor Type 1=Stepper 200FS/R  2=400FS/R: "<<(unsigned int) pStr[33]<<endl;
	range=(0x00FF&((unsigned short int)pStr[26])) | (0xFF00&(((unsigned short int)pStr[27])<<8));
	cout<<"Y axis travel range: "<<range<<endl;
	pitch=(0x00FF&((unsigned short int)pStr[28])) | (0xFF00&(((unsigned short int)pStr[29])<<8));
	cout<<"Y leadscrew pitch: "<<pitch<<endl;
	cout<<"Y Encoder Type (5 is linear encoder) "<<(unsigned int) pStr[30]<<endl;
	resol=(0x00FF&((unsigned short int)pStr[31])) | (0xFF00&(((unsigned short int)pStr[32])<<8));
	cout<<"Y resolution (steps/rev) or (steps/mm) probably the latter because it is an xystage	: "<<resol<<endl;
	cout<<"Y Aux Encoder Type (0=none 1=20000rotary 2=10000rotary 3=4000 4=2000 5=linear): "<<(unsigned int)pStr[34]<<endl;
	resol=(0x00FF&((unsigned short int)pStr[35])) | (0xFF00&(((unsigned short int)pStr[36])<<8));
	cout<<"Y Aux resolution (steps/rev) or (steps/mm) probably the latter because it is an xystage	: "<<resol<<endl;
}

void Ludl::setJoystickSpeed(double normal, double faster){
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 69;		// read resolution	
	pStr[2] = 1;
	pStr[3] = (1843000/normal+.5);
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
		
	pStr[0] = yAxisID;	// x axis
	pStr[1] = 69;		// read resolution	
	pStr[2] = 1;
	pStr[3] = (1843000/normal+.5);
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command

		pStr[0] = xAxisID;	// x axis
	pStr[1] = 70;		// read resolution	
	pStr[2] = 1;
	pStr[3] = (1843000/faster+.5);
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
		
	pStr[0] = yAxisID;	// x axis
	pStr[1] = 70;		// read resolution	
	pStr[2] = 1;
	pStr[3] = (1843000/faster+.5);
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
}

void Ludl::readJoystickSpeed(double& normal, double& faster){
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 101;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr,1);
	normal=1843000.0/pStr[0];

	pStr[0] = xAxisID;	// x axis
	pStr[1] = 102;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr,1);
	faster=1843000.0/pStr[0];
}

int Ludl::readMotorRes(){
	pStr[0] = yAxisID;	// x axis
	pStr[1] = 160;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 1);
	int i;
	switch (pStr[0]){
		case 0:
			i=40000;
			break;
		case 1:
			i=20000;
			break;
		case 2:
			i=10000;
			break;
		case 3:
			i=5000;
			break;
		case 4:
			i=2000;
			break;
		case 5:
			i=1000;
			break;
		case 6:
			i=400;
			break;
		default:
			logFile.write(string("Error: Ludl returned invalid motor res:")+toString((int)pStr[0]),true);
			return -1;
	}
	return i;
}

void Ludl::MotorRes(int res){
	CheckExists()
	unsigned char i;
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 160;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 1);
	//logFile.write(string("Ludl Stage: X axis resolution starts at")+toString((int)pStr[0]),true);
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 160;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 1);
	//logFile.write(string("Ludl Stage: Y axis resolution starts at")+toString((int)pStr[0]),true);
	switch (res){
		case 40000:
			i=0;
			break;
		case 20000:
			i=1;
			break;
		case 10000:
			i=2;
			break;
		case 5000:
			i=3;
			break;
		case 2000:
			i=4;
			break;
		case 1000:
			i=5;
			break;
		case 400:
			i=6;
			break;
		default:
			cout<<res<<" is not a valid resolution"<<endl;
			return;
	}

	pStr[0] = xAxisID;	// x axis
	pStr[1] = 40;		// set resolution	
	pStr[2] = 1;
	pStr[3] = i;
	pStr[4] = ':';
	CUsb.Write(pStr,5);		// send the command

	pStr[0] = yAxisID;	// y axis
	pStr[1] = 40;		// set resolution	
	pStr[2] = 1;
	pStr[3] = i;
	pStr[4] = ':';
	CUsb.Write(pStr,5);		// send the command

	pStr[0] = xAxisID;	// x axis
	pStr[1] = 160;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 1);
	if (pStr[0]!=i){
		logFile.write(string("Error: X axis resolution not set. Return code is: ")+toString((int) pStr[0]),true);
	}else{
		logFile.write(string("Ludl Stage: X axis resolution set to")+toString((int)pStr[0]),false);
	}
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 160;		// read resolution	
	pStr[2] = 1;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 1);
	if (pStr[0]!=i){
		logFile.write(string("Error: Y axis resolution not set. Return code is: ")+toString((int) pStr[0]),true);
	}else{
		logFile.write(string("Ludl Stage: Y axis resolution set to")+toString((int)pStr[0]),false);
	}
	
}


void Ludl::gotoEndLimit(int deviceID,int Speed){
	//pStr[0] = deviceID;	// x axis
	//pStr[1] = 39;		// go to end limit	
	//pStr[2] = 0;
	//pStr[3] = ':'; 
	//CUsb.Write(pStr,4);		// send the command
	Speed = 8388608-(5529600/Speed);
		pStr[0] = deviceID;	// x axis
	pStr[1] = 47;		// go to end limit	
	pStr[2] = 3;
	pStr[3] = (0x0000FF&Speed)>>0;		
	pStr[4] = (0x00FF00&Speed)>>8;	
	pStr[5] = (0xFF0000&Speed)>>16;	
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

void Ludl::center(int deviceID,double speed){
	int val=8388608-(5529600/speed);
	pStr[0] = deviceID;	// x axis
	pStr[1] = 48;		// center
	pStr[2] = 3;
	pStr[3] = (0x0000FF&val)>>0;; 
	pStr[4] = (0x00FF00&val)>>8;; 
	pStr[5] = (0xFF0000&val)>>16;; 
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

string Ludl::checkErrorHighLevel(double msTimeout){
	string s;
	CUsb.Read(pStr,2);
	if (pStr[0]!=':' || pStr[1]!='A'){
		logFile.write("Error, did not receive correct response from Ludl",true);
	}
	Timer t(true);
	pStr[1]='\0';
	while(t.getTime()<msTimeout){
		CUsb.Read(pStr,1);
		if (*pStr=='\n')
			return s;
		s=s+string((char*)pStr);
	}
	logFile.write("Error, Ludl timed out waiting for response from controller",true);
	return s;
}

void Ludl::enableCounterCapture(int deviceID){
	pStr[0] = deviceID;	// x axis
	pStr[1] = 30;		// go to end limit	
	pStr[2] = 0;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command	
}

void Ludl::loadCenterPosition(int val, int deviceID){
	pStr[0] = deviceID;	// x axis
	pStr[1] = 91;		// go to end limit	
	pStr[2] = 3;
	pStr[3] = (0x0000FF&val)>>0;		
	pStr[4] = (0x00FF00&val)>>8;		
	pStr[5] = (0xFF0000&val)>>16;	
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command	
}

int Ludl::getCenterPosition(int deviceID){
	int pos=0;
	pStr[0] = deviceID;	// device #
	pStr[1] = 123;		// read motor position
	pStr[2] = 3;	
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command
	CUsb.Read(pStr, 3);
	pos =  0x000000FF&((unsigned int)pStr[0]);
	pos |= 0x0000FF00&((unsigned int)pStr[1]<<8);
	pos |= 0x00FF0000&((unsigned int)pStr[2]<<16);
	if(pos & 0x00800000) pos |= 0xFF000000;	// make it negative
	return(pos);
}

void Ludl::disableCounterCapture(int deviceID){
	pStr[0] = deviceID;	// x axis
	pStr[1] = 31;		// go to end limit	
	pStr[2] = 0;
	pStr[3] = ':'; 
	CUsb.Write(pStr,4);		// send the command	
}

//identify and set zero of stage and return to starting position
void Ludl::home2(){
	CheckExists()
		static int trys(0);
	int	startX=this->getX();
	int startY=this->getY();
	wait();
	int speed=50000;
	//CUsb.SetVelocity(xAxisID,HOMINGVELOCITYCOARSE);
	//CUsb.SetVelocity(yAxisID,HOMINGVELOCITYCOARSE);
	gotoEndLimit(yAxisID,HOMINGVELOCITYCOARSE);
	gotoEndLimit(xAxisID,HOMINGVELOCITYCOARSE);
	wait();
	CUsb.SetVelocity(xAxisID,HOMINGVELOCITYFINE);
	CUsb.SetVelocity(yAxisID,HOMINGVELOCITYFINE);
	gotoEndLimit(yAxisID,HOMINGVELOCITYFINE);
	gotoEndLimit(xAxisID,HOMINGVELOCITYFINE);
	wait();
	int x1=getX();
	int y1=getY();
	gotoEndLimit(yAxisID,HOMINGVELOCITYFINE);
	gotoEndLimit(xAxisID,HOMINGVELOCITYFINE);
	wait();
	int x2=getX();
	int y2=getY();
	gotoEndLimit(yAxisID,HOMINGVELOCITYFINE);
	gotoEndLimit(xAxisID,HOMINGVELOCITYFINE);
	wait();
	int x3=getX();
	int y3=getY();
	
	//speed=2764800;
	//CUsb.SetVelocity(xAxisID,speed);
	//CUsb.SetVelocity(yAxisID,speed);
	CUsb.SetVelocity(xAxisID,MAXVELOCITY);
	CUsb.SetVelocity(yAxisID,MAXVELOCITY);
	double meanX=(x1+x2+x3)/3.0;
	double meanY=(y1+y2+y3)/3.0;
	double stdDevX=sqrt((square(x1-meanX)+square(x2-meanX)+square(x3-meanX))/3.0)/0.855;//0.855 is unbiased correction factor for n=3
	double stdDevY=sqrt((square(y1-meanY)+square(y2-meanY)+square(y3-meanY))/3.0)/0.855;
	if (stdDevX*getStepSize()>LIMITCUSHION || stdDevY*getStepSize()>LIMITCUSHION){
		trys++;
		logFile.write(string("Homing failed. X StdDev was ")+toString(stdDevX*getStepSize())+"um Y StdDev was "+toString(stdDevY*getStepSize())+"um Retrying...",true);
		if (trys>=3){
			logFile.write("Error: Ludl could not perform homing...absolute stage positioning could be off",true);
			moveX(startX);
			moveY(startY);
			return;
		}else {home(x1,y1); return;}
	}
	trys=0;
	logFile.write(string("XY Stage Homing Succesful. X StdDev was ")+toString(stdDevX*getStepSize())+"um Y StdDev was "+toString(stdDevY*getStepSize())+"um",true);
		
	setPosition(-XOFFSET,-YOFFSET);
	moveX(startX-meanX-XOFFSET);
	moveY(startY-meanY-YOFFSET);
	wait();	
}

bool Ludl::home(int& x, int& y){
	int yTemp=0xFF800000;//most negative 3 byte position
	int xTemp=0xFF800000;
	int val=0x007FFFFF;//most positive 3 byte position
	loadCenterPosition(val,xAxisID);
	loadCenterPosition(val,yAxisID);
	yTemp=getCenterPosition(yAxisID);
	xTemp=getCenterPosition(xAxisID);
	
	if (val!=xTemp || val!=yTemp){
		logFile.write("Error: Ludl failed to set center position",true);
		return false;
	}
	
	//CUsb.SetVelocity(xAxisID,HOMINGVELOCITYCOARSE);
	//CUsb.SetVelocity(yAxisID,HOMINGVELOCITYCOARSE);
	disableServo();
	//moveY(val);
	gotoEndLimit(yAxisID,-HOMINGVELOCITYCOARSE);
	//moveX(val);
	gotoEndLimit(xAxisID,-HOMINGVELOCITYCOARSE);
	wait(30000);
	enableServo();
	enableCounterCapture(xAxisID);
	
	center(xAxisID,HOMINGVELOCITYFINE);
	wait(30000);
	wait();
	xTemp=getX();
	x=getCenterPosition(xAxisID);
	disableCounterCapture(xAxisID);

	enableCounterCapture(yAxisID);
	center(yAxisID,HOMINGVELOCITYFINE);
	wait(30000);
	wait();
	yTemp=getY();
	y=getCenterPosition(yAxisID);
	disableCounterCapture(yAxisID);
	
	
	if (x==val){
		logFile.write("Ludl error: Did not find X center position",true);
		x=xTemp;
		return false;
	}
	if (y==val){
		logFile.write("Ludl error: Did not find Y center position",true);
		y=yTemp;
		return false;
	}
	if (x!=xTemp){
		logFile.write("Ludl X-Axis: Settled position differed from captured position. "+toString(xTemp)+"vs"+toString(x),true);
	}
	if (y!=yTemp){
		logFile.write("Ludl Y-Axis: Settled position differed from captured position "+toString(yTemp)+"vs"+toString(y),true);
	}
	enableHighLevel();
	pStr[0]='i';pStr[1]='s';pStr[2]='t';pStr[3]='a';pStr[4]='t';pStr[5]=' ';pStr[6]='1';pStr[7]='\r';
	CUsb.Write(pStr,8);
	checkErrorHighLevel();
	enableLowLevel();
	return true;
}

void Ludl::setPosition(int x,int y){
	CheckExists()
	CUsb.SetPosition(xAxisID,x);
	CUsb.SetPosition(yAxisID,y);
}

void Ludl::XStepper(int x){
	CheckExists()
	pStr[0] = xAxisID;	// y axis
	pStr[1] = 46;		// 	
	pStr[2] = 3;
	pStr[3] = 0x000000FF & x;
	pStr[4] = (0x0000FF00 & x)>>8;
	pStr[5] = (0x00FF0000 & x)>>16;
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

void Ludl::YStepper(int y){
	CheckExists()
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 46;		// 
	pStr[2] = 3;
	pStr[3] = 0x000000FF & y;
	pStr[4] = (0x0000FF00 & y)>>8;
	pStr[5] = (0x00FF0000 & y)>>16;
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

void Ludl::stop(){
	CheckExists()
	CUsb.Stop(xAxisID);
	CUsb.Stop(yAxisID);
}

void Ludl::enableLimits(int minX, int minY, int maxX, int maxY){
	CheckExists()
	//write limits
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 17;		// 
	pStr[2] = 6;
	pStr[3] = 0x000000FF & minX;
	pStr[4] = (0x0000FF00 & minX)>>8;
	pStr[5] = (0x00FF0000 & minX)>>16;
	pStr[6] = 0x000000FF & maxX;
	pStr[7] = (0x0000FF00 & maxX)>>8;
	pStr[8] = (0x00FF0000 & maxX)>>16;
	pStr[9] = ':'; 
	CUsb.Write(pStr,10);		// send the command
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 17;		// 
	pStr[2] = 6;
	pStr[3] = 0x000000FF & minY;
	pStr[4] = (0x0000FF00 & minY)>>8;
	pStr[5] = (0x00FF0000 & minY)>>16;
	pStr[6] = 0x000000FF & maxY;
	pStr[7] = (0x0000FF00 & maxY)>>8;
	pStr[8] = (0x00FF0000 & maxY)>>16;
	pStr[9] = ':'; 
	CUsb.Write(pStr,10);		// send the command

	//activate limits
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 16;		// 
	pStr[2] = 1;
	pStr[3] = 1;
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 16;		// 
	pStr[2] = 1;
	pStr[3] = 1;
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command

}

void Ludl::disableLimits(){
	CheckExists()
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 16;		// 
	pStr[2] = 1;
	pStr[3] = 0;
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 16;		// 
	pStr[2] = 1;
	pStr[3] = 0;
	pStr[4] = ':'; 
	CUsb.Write(pStr,5);		// send the command
}

void Ludl::moveX(int x){
	CheckExists()
	CUsb.Goto(xAxisID,x);

}

void Ludl::moveY(int y){
	CheckExists()
	CUsb.Goto(yAxisID,y);
}

int Ludl::getX(){
	CheckExists(0)
	return CUsb.GetPosition(xAxisID);
}

int Ludl::getY(){
	CheckExists(0)
	return CUsb.GetPosition(yAxisID);
}

void Ludl::wait(double TimeoutMilliseconds){
	CheckExists()
		Timer t(true);
	int i=0;
	while(CUsb.IsBusy(xAxisID)){
		i++;
		Timer::wait(50);
		if (t.getTime()>TimeoutMilliseconds){
			logFile.write(string("Error: Ludl wait for stepper motors took more than ")+toString(TimeoutMilliseconds/1000.0)+" seconds",true);
			::clickAbort();
			break;
		}
	}
	Timer::wait(50);
	i=0;
	while(CUsb.IsBusy(yAxisID)){
		i++;
		Timer::wait(50);
		if (t.getTime()>TimeoutMilliseconds){
			logFile.write(string("Error: Ludl wait for stepper motors took more than ")+toString(TimeoutMilliseconds/1000.0)+" seconds",true);
			::clickAbort();
			break;
		}
	}
	waitServo(xAxisID);
	waitServo(yAxisID);
}

void Ludl::incX(int dx){
	CheckExists()
	moveX(getX()+dx);
}
void Ludl::incX(){
	CheckExists()
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 43;		// 	
	pStr[2] = 0;
	pStr[3] = ':';
	CUsb.Write(pStr,4);		// send the command
}

void Ludl::decX(){
	CheckExists()
	pStr[0] = xAxisID;	// x axis
	pStr[1] = 45;		// 	
	pStr[2] = 0;
	pStr[3] = ':';
	CUsb.Write(pStr,4);		// send the command
}
void Ludl::setXinc(int dx){
	CheckExists()
	pStr[0] = xAxisID;	// y axis
	pStr[1] = 68;		// 	
	pStr[2] = 3;
	pStr[3] = 0x000000FF & dx;
	pStr[4] = (0x0000FF00 & dx)>>8;
	pStr[5] = (0x00FF0000 & dx)>>16;
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

void Ludl::setYinc(int dy){
	CheckExists()
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 68;		// 	
	pStr[2] = 3;
	pStr[3] = 0x000000FF & dy;
	pStr[4] = (0x0000FF00 & dy)>>8;
	pStr[5] = (0x00FF0000 & dy)>>16;
	pStr[6] = ':'; 
	CUsb.Write(pStr,7);		// send the command
}

void Ludl::incY(){
	CheckExists()
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 43;		// 	
	pStr[2] = 0;
	pStr[3] = ':';
	CUsb.Write(pStr,4);		// send the command
}

void Ludl::decY(){
	CheckExists()
	pStr[0] = yAxisID;	// y axis
	pStr[1] = 45;		// 	
	pStr[2] = 0;
	pStr[3] = ':';
	CUsb.Write(pStr,4);		// send the command
}
void Ludl::incY(int dy){
	CheckExists()
	moveY(getY()+dy);
}
/*//MAIN FUNCTION FOR TESTING
int main(int argc, char* argv[])
{
	return 0;
}*/



void Ludl::fineControl(){
	int c;
	MotorRes(40000);
	bool servo=servoEnabled();
	disableServo();
	std::cin.ignore(std::numeric_limits<streamsize>::max(),'\n');
	cout<<"Welcome to Fine Stage Control (use the arrow keys to move the stage in 50nm steps)"<<endl<<"Press e to exit"<<endl;
	while(true){
		cout<<"Welcome to Fine Stage Control (use the arrow keys to move the stage in 50nm steps)"<<endl<<"Press e to exit"<<endl;
		c=getch();
		switch(c){
			case 224://arrow pressed
				c=getch();
				switch(c){
					case 72://up
						YStepper(1);
						break;
					case 80://down
						YStepper(-1);
						break;
					case 75://left
						XStepper(-1);
						break;
					case 77://right
						XStepper(1);
						break;
				}
				wait();
				cout<<"X: "<<getX()<<" Y: "<<getY()<<" Press e to exit"<<endl;
				break;
			case 101:
				//filt->closeShutter();
				MotorRes(res);
				if (servo) enableServo();
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
				wait();
				cout<<"X: "<<getX()<<" Y: "<<getY()<<" Press e to exit"<<endl;
		}
	}
}
/*
void Controller::testScan(){
	int x=stg->getX();
	int y=stg->getY();
	char s[10];
	
	for(int i=0;i<10;i++){
		filt->switchFilter(5);
	cam->takePicture(.03,100,1);
	cam->waitIdle();
	filt->closeShutter();
	CImg<unsigned short>* c=cam->getNewPic();
	cam->saveTiff("D:\\testScan3\\cy5t00"+string(itoa(i,s,10))+".tif",c);
	delete c;
	stg->move(x+30000/stg->getStepSize(),y+30000/stg->getStepSize());
	stg->wait();
	stg->move(x,y);
	stg->wait();
	}
}*/