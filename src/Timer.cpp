// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Timer.h"
#include "Utils.h"
#include "Record.h"
using namespace std;

#define DEBUGTIMER true
extern Record logFile;

void Timer::wait(double ms){
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER current;
	double time=ms*freq.QuadPart/1000.0;
	while(true){
		QueryPerformanceCounter(&current);
		if ((current.QuadPart-start.QuadPart) >= time) break;
	}
}

//can't call constructors from other constructors in C++ (thats only in JAVA)
Timer::Timer(){
	QueryPerformanceFrequency(&freq);
	total.QuadPart=0;
	running=false;
}

Timer::Timer(bool start){
	//cout<<"Timer created"<<endl;
	QueryPerformanceFrequency(&freq);
	total.QuadPart=0;
	running=false;
	if (start) this->startTimer();
}

std::string Timer::getSysTime(){
	SYSTEMTIME t;
	GetLocalTime(&t);
	return ::toString(t.wYear)+"/"+::toString((int)t.wMonth,2)+"/"+::toString((int)t.wDay,2)+" "+::toString((int)t.wHour,2)+":"+::toString((int)t.wMinute,2)+":"+::toString((int)t.wSecond,2)+"."+::toString((int)t.wMilliseconds,3);
}

void Timer::startTimer(){
	if (running) {
		cout<<"timer already running"<<endl;
		return;
	}
	running=true;
	QueryPerformanceCounter(&start);
}

void Timer::stopTimer(){
	if (!running) {
		cout<<"timer already stopped"<<endl;
		return;
	}
	QueryPerformanceCounter(&end);
	total.QuadPart+=end.QuadPart-start.QuadPart;
	running=false;

}

double Timer::getTimeAfterLastStop(){
	QueryPerformanceCounter(&end);
	if (running) return ((double) (end.QuadPart-start.QuadPart))*1000.0/freq.QuadPart;//time in ms
	else return 0.0;
}

void Timer::waitAfterLastStart(double ms, HANDLE abortEvent){
	LARGE_INTEGER current;
	double time=ms*freq.QuadPart/1000.0;
	while(true){
		if (abortEvent && WAIT_OBJECT_0==WaitForSingleObject(abortEvent,0))
			return;
		QueryPerformanceCounter(&current);
		if ((current.QuadPart-start.QuadPart) >= time) break;
	}
}

void Timer::waitTotal(double ms, HANDLE abortEvent){
	if (!running){
		logFile.write("Timer: cannot wait when timer is stopped",DEBUGTIMER);
	}
	LARGE_INTEGER current;
	double time=ms*freq.QuadPart/1000.0;
	while(true){
		if (abortEvent && WAIT_OBJECT_0==WaitForSingleObject(abortEvent,0))
			return;
		QueryPerformanceCounter(&current);
		if ((current.QuadPart-start.QuadPart+total.QuadPart) >= time) break;
	}
}

///return current time in ms
double Timer::getTime(){
	/*allow for time queries without stopping the timer
	if (running) {
		cout<<"please stop timer first"<<endl;
		return 0;
	}
	*/
	QueryPerformanceCounter(&end);
	if (running) return ((double) (total.QuadPart+end.QuadPart-start.QuadPart))*1000.0/freq.QuadPart;//time in ms
	else return ((double) (total.QuadPart))*1000.0/freq.QuadPart;
}

string Timer::toString(){
	double ms=this->getTime();
	int hours=ms/1000.0/60.0/60.0;
	double minTemp=(ms-hours*1000.0*60.0*60.0)/1000.0/60.0;
	int min=minTemp;
	double secondsTemp=(minTemp-min)*60.0;
	int seconds=secondsTemp;
	ms=(secondsTemp-seconds)*1000.0;
	string ret;
	if (hours!=0)
		ret+=::toString(hours)+"hr ";
	if (min!=0)
		ret+=::toString(min,0)+"min ";
	if (seconds!=0)
		ret+=::toString(seconds,2)+"sec ";
	if (ms!=0)
		ret+=::toString(ms)+"ms ";
	return ret.substr(0,ret.size()-1);
}

void Timer::resetTimer(){
	total.QuadPart=0;
	running=false;
}

void Timer::restart(){
	resetTimer();
	startTimer();
}