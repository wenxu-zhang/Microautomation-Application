// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, May 12, 2011</lastedit>
// ===================================
#include "Warn.h"
#include <iostream>
#include <string>
#include "DG.h"
#include <stdio.h>
#include <windows.h>
#include "pt_ioctl.h"
#include "Controller.h"
#include "Timer.h"
#include "Definitions.h"
#include "AcquisitionGroup.h"
//#include "Camera.h"
using namespace std;

extern Controller cont;
extern Record logFile;
#define DEBUGFILTER true;
/*
const bool debug=false;
const UINT Filter::INPUT=0x379;//typical input for LPT1
const UINT Filter::OUTPUT=0x378;//typical output for LPT1
const double Filter::del=200;//microsecond delay for busy signal
*/

void DG::on(int pos, double intensity){
	CheckExists()
		cont.axio.openShutter();
		if (intensity>.75)//half way between 50% and 100%
			this->switchFilter(pos);//output 100%
		else if (intensity>.417)//half way between 33% and 50%
			this->switchFilter(pos+5);//output 50%
		else if (intensity==.1)
			this->switchFilter(4);//HARDCODED FOR KRIS WITH POSITION 4 HAVING A ND 1 FILTER.
		else if (intensity==.01)
			this->switchFilter(5);//HARDCODED FOR KRIS WITH POSITION 5 HAVING A ND 2 FILTER.
		else if (intensity==cp->zeroIntensity)
			off();
		else this->switchFilter(pos+10);//output 33%
}

string DG::intensityToString(double intensity,Objective* obj){
	return toString(intensity*100.0)+"%";
}

double DG::getIntensity(double intensity,string intensityUnit,Objective* obj){
	CheckExists(1);
	if (intensity==-1){
		return 1;
	}
	if (intensityUnit=="")
		return getIntensity(intensity,"f");
	if (intensityUnit=="f"){
		if (intensity>1){
			logFile.write("Error: DG invalid intensity "+toString(intensity)+" must be <= 1",true);
			return 1;
		}
		if (intensity<0){
			logFile.write("Error: DG invalid intensity "+toString(intensity)+" must be >= 0",true);
			return 1;
		}
		if (intensity>0.75)
			return 1;
		if (intensity>.417)
			return .5;
		if (intensity==.1)
			return .1;
		if (intensity==.01)
			return .01;
		return .33;
	}
	if (intensityUnit=="%"){
		return getIntensity(intensity/100.0,"f");		
	}
	logFile.write(string("Error: unsupported intensity unit ")+intensityUnit,true);
	return 1;
}

void DG::off(){this->closeShutter();}

void DG::setWaveform(AcquisitionGroup chans){
	this->channels=chans.acquisitionChannels;
	//enterRingBuffer(chans);
}

void DG::prepareWaveform(){}

void DG::generateWaveform(){
	Timer timetr(true);
	double total=0;
	for(vector<AcquisitionChannel>::iterator i=channels.begin();i!=channels.end();i++){
		//setLineHigh(2);
		//setLineHigh(1);
		//Timer::wait(2);
		//setLineLow(1);
		//setLineLow(2);
		//total+=i->ap.exp;
		//timetr.waitTotal(1000.0*total);
		triggerBoth(0.5,(*i).ap.exp);
	}
		triggerBoth(0.5,2);
	/*setLineHigh(2);
	setLineHigh(1);
	Timer::wait(2);
	setLineLow(1);
	setLineLow(2);
	Timer::wait(2);*/
}

void DG::exitRingBuffer(){
	switchFilter(0);
}

void DG::enterRingBuffer(AcquisitionGroup& ag){
	CheckExists()
	if (bufferMode) exitRingBuffer();
	bufferMode=true;
	outportb(OUTPUT,0xDF);
	//Sleep(60);
	delay(del);
	wait();
	int j=0;
	unsigned char add=0x00;
	for(vector<AcquisitionChannel>::iterator i=ag.acquisitionChannels.begin();i!=ag.acquisitionChannels.end();i++){
		if (i->chan->lite().ls!=this) {logFile.write("Error: wrong light source for triggering"); return;}
		if (i->intensity>.75)//half way between 50% and 100%
			outportb(OUTPUT,(BYTE) i->chan->lite().position+add);//output 100%
		else if (i->intensity>.417)//half way between 33% and 50%
			outportb(OUTPUT,(BYTE) i->chan->lite().position+5+add);//output 50%
		else if (i->intensity==.1)
			outportb(OUTPUT,(BYTE) 4+add);//HARDCODED FOR KRIS WITH POSITION 4 HAVING A ND 1 FILTER.
		else if (i->intensity==.01)
			outportb(OUTPUT,(BYTE) 5+add);//HARDCODED FOR KRIS WITH POSITION 5 HAVING A ND 2 FILTER.
		else if (i->intensity==0)
			outportb(OUTPUT,(BYTE) 0+add);//shutter during pause
		else outportb(OUTPUT,(BYTE) i->chan->lite().position+10);//output 33%
		Sleep(5);
		delay(del);
		wait();
		j++;
		if (j%2==1)
			add=0x10;
		else
			add=0x00;
	}
	outportb(OUTPUT,(BYTE)0); //put shutter after each series of colors
	Sleep(5);
	delay(del);
	wait();
	//put another shutter after each series of colors since we need to trigger again to exit ring buffer
	outportb(OUTPUT,0x10);//zero filter position (only 4 bits are read so this should be zero)
//	Sleep(5);
	//Sleep(60);
//	delay(del);
//	wait();
	//outportb(OUTPUT,(BYTE)0); 
	delay(del);
	wait();
	outportb(OUTPUT,0xF0);
	//Sleep(60);
	delay(del);//Sleep(1);
	wait();
	//enable strobe trigger;
	outportb(OUTPUT,0xCA);
	//Sleep(60);
	delay(del);//	Sleep(100);
	wait();
	outportb(OUTPUT,0xF1);//ring buffer mode
	Timer::wait(2000);
	//wait(); cannot wait here because the DG4 has not received a trigger
	chan=-1;//not to be confused with BF in controller class

}

void DG::setLineHigh(int line){
	switch(line){
		case 1:
			outportb(0x37A,0x08);
			break;
		case 0:
			outportb(0x37A,0x04);
			break;
		default:
			logFile.write(string("Parallel Port: unsupported trigger line ")+toString(line),true);
			break;
	}
	Timer::wait(2);
}

void DG::setLineLow(int line){
	if (line!=0 && line!=1)
		logFile.write(string("Parallel Port: unsupported trigger line ")+toString(line),true);
	outportb(0x37A,0x00);
}

void DG::triggerBoth(double d, float exps){
	double pulseWidth=2;//width of each pulse in ms;
	//for(int i=0;i<=numchan;i++)
	//{
		outportb(0x37A,0x04);
		Timer::wait(d);
		outportb(0x37A,0x0C);
		Timer::wait(pulseWidth);
		outportb(0x37A,0x00);
		Timer::wait(exps*1000-pulseWidth);
	//}
}
DG::DG(Trigger* t, int triggerLine,double defaultIntensity):LightSource(t,"DG5",triggerLine,defaultIntensity){
#ifdef OBSERVER
	if (cont.axio.isManualLamp()){
		isPresent=false;
		logFile.write("\"Other Lamp\" is selected in MTB2004.  Did not attempt to communicate with DG5",true);
		return;
	}
#endif
	logFile.write("\"Empty Lamp Port\" is selected in MTB2004.  Attempting to communicate with DG5",true);
	if (!t) t= (Trigger*) this;
	INPUT=0x379;//typical input for LPT1
	OUTPUT=0x378;//typical output for LPT1
	del=1000;//microsecond delay for busy signal
	bufferMode=false;
	isPresent=true;
	chan=0;
	OpenPortTalk();
	outportb(OUTPUT,0);//close shutter immediately if possible
	outportb(0x37A,16);//may be needed on some computers to enable input
	outportb(OUTPUT,0xF2);//end ring buffer mode just in case
	delay(del);
	t->triggerSingleLine(triggerLine);
	//this->triggerDG();//send trigger to flush command
	delay(del);
	Sleep(60);
	BYTE in=inportb(INPUT);
	bool busy=!(in&0x80);
	if (busy){
		logFile.write("DG NOT PRESENT",true,": Parallel Port Control Problem");
		isPresent=false;
		return;
	}
	//computer control
	outportb(OUTPUT,0xEE);
	Timer timeoutTimer(true);
	//wait for busy
	while(!busy){
		in=inportb(INPUT);
		busy=!(in&0x80);
		if (timeoutTimer.getTime()>DGTIMEOUT){
			logFile.write("DG NOT PRESENT",true,": could not communicate with device");
			isPresent=false;
			return;
		}
	}
	//wait for not busy
	while(busy){
		in=inportb(INPUT);
		busy=!(in&0x80);
		if (timeoutTimer.getTime()>DGTIMEOUT){
			logFile.write("DG NOT PRESENT",true,": could not communicate with device");
			isPresent=false;
			return;
		}
	}
	outportb(OUTPUT,0);//close shutter immediately
	delay(del);
	wait();
	//disable video sync
	outportb(OUTPUT,0xCD);
	delay(del);
	wait();
	//disable video sync strobe
	outportb(OUTPUT,0xCF);
	delay(del);
	wait();
	//enable strobe trigger;
	outportb(OUTPUT,0xCA);
	delay(del);
	wait();
	//turbo blanking on
	outportb(OUTPUT,0xBA);
	delay(del);
	wait();
	cont.rl.on(0,1);
	logFile.write(string("DG5 ready"),true);
}

DG::~DG(){
	CheckExists()
	closeShutter();
	delay(del);//Sleep(1);
	wait();
	//turn display on
	outportb(OUTPUT,0xDB);
	delay(del);
	wait();
	outportb(OUTPUT,0x0);
	ClosePortTalk();
}

void DG::errorExit(){
	//outportb(filt->OUTPUT,0xDB);
	ClosePortTalk();
}
void DG::closeShutter(){
	switchFilter(0);//filter zero is the shutter
}

void DG::switchFilter(BYTE pos){
	CheckExists()
	if (bufferMode) {
		outportb(OUTPUT,0xF2);//end ring buffer mode
		//delay(del);
		Timer::wait(1);
		t->triggerSingleLine(cp->triggerLine);
		//this->triggerDG();//send trigger to flush command
		//delay(del);
		Timer::wait(1);
		wait();
		bufferMode=false;
	}
	outportb(OUTPUT, pos);
	wait();
	chan=pos;
}

//wait for BUSY line to go low, but since the line is hardware inverted we check to see if the register is HIGH
void DG::wait(){
	CheckExists()
	if (bufferMode) return;
	BYTE seeIt;
	seeIt=inportb(INPUT);
	//BYTE in=inportb(OUTPUT);
	//cout<<"output is 0x"<<hex<<(unsigned int) in<<dec<<endl;
	while(!(seeIt&0x80)){//but 0123 not used, bit 7 is what we want
	//	cout<<"seeIt: "<<hex<<(unsigned int)seeIt<<dec<<endl;
	//	cout<<"bit 7: "<<((seeIt&0x80)>>7)<<endl;
	//	cout<<"bit 6: "<<((seeIt&0x40)>>6)<<endl;
	//	cout<<"bit 5: "<<((seeIt&0x20)>>5)<<endl;
	//	cout<<"bit 4: "<<((seeIt&0x10)>>4)<<endl;
	//	cout<<"bit 3: "<<((seeIt&0x8)>>3)<<endl;
	//	cout<<"bit 2: "<<((seeIt&0x4)>>2)<<endl;
	//	cout<<"bit 1: "<<((seeIt&0x2)>>1)<<endl;
	//	cout<<"bit 0: "<<(seeIt&0x1)<<endl;
	//	system("pause");
		seeIt=inportb(INPUT);
	}
	cont.rl.wait();
	
}

void DG::delay(double microsec){
	LARGE_INTEGER start,end, freq;
	QueryPerformanceCounter(&start);
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&end);
	while(((double)(end.QuadPart-start.QuadPart))/freq.QuadPart < microsec/1000000){
		QueryPerformanceCounter(&end);
	}
}

void DG::DGControl(){
	AcquisitionGroup ag;
		char c;
	string periodSec;
	int pos;
	double intensity;
	string t3;
	while(true){
		cout<<"Please select a task"<<endl;
		cout<<"0: Light On"<<endl;
		cout<<"1: Light Off"<<endl;
		cout<<"2: Create AcquisitionGroup"<<endl;
		cout<<"3: Enter ring buffer mode"<<endl;
		cout<<"4: Trigger Once"<<endl;
		cout<<"5: Trigger Waveform"<<endl;
		cout<<"6: Exit ring buffer mode"<<endl;
		cout<<"e: Exit DG Control"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '0'://Light on
				cout<<"Please enter desired position"<<endl;
				cin>>pos;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Please enter desired intensity (0-1)"<<endl;
				cin>>intensity;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				on(pos,intensity);
				break;
			case '1'://Light off
				off();
				break;
			case '2'://create acquisition group
				ag=AcquisitionGroup::getAcquisitionGroup();
				break;
			case '3'://enter ring buffer mode
				enterRingBuffer(ag);
				break;
			case '4'://Trigger Once
				this->t->triggerSingleLine(this->cp->triggerLine);
				break;
			case '5'://save current focus position data
				this->t->setWaveform(ag);
				this->t->prepareWaveform(ag);
				this->t->generateWaveform();
				break;
			case '6':
				this->exitRingBuffer();
				break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
	

}
