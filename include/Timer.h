// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include "Warn.h"
#include <windows.h>
#include <iostream>
#include <string>

class Timer{
public:
	Timer();
	Timer(bool start);
	void startTimer();
	void stopTimer();
	void resetTimer();
	void restart();
	void waitAfterLastStart(double ms,HANDLE abortEvent=NULL);//wait for ms amount of time since last start
	void waitTotal(double ms,HANDLE abortEvent=NULL);//wait for total elapsed time on this timer 
	double getTime();//return time in ms
	double getTimeAfterLastStop();//return time in ms
	std::string toString();//return XXhours:XXmin:XXseconds.XXXms
	bool running;

	static void wait(double ms);//wait for ms amount of time
	static std::string getSysTime();

private:
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	LARGE_INTEGER freq;
	LARGE_INTEGER total;
};