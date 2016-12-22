#pragma once
#include <windows.h>
#include "Trigger.h"
namespace NIDAQmx{
#include "NIDAQmx.h"
}
//using namespace NIDAQmx;
class DAQPCI6733:public Trigger{
public:
	double getVoltage(int line);
	void prepareDigitalLine(int line);
	void prepareAnalogLine(int line);
	void setVoltage(int line,double voltage);
	void setLineHigh(int line);
	void setLineLow(int line);
	void setLine(int line,bool high);
	void setWaveform(AcquisitionGroup channels);
	void prepareWaveform();
	void generateWaveform();
	bool isValidLine(int line){return true;}
	bool isValidAnalogOutLine(int line){return true;}
	bool isValidAnalogInLine(int line){return true;}

	bool isPresent;
	DAQPCI6733();
	~DAQPCI6733();

private:
	void GetError(bool b);
	NIDAQmx::TaskHandle AO0;//single analog voltage out and hold during idle AO0
	NIDAQmx::TaskHandle AO1;//single analog voltage out and hold during idle AO1
	NIDAQmx::TaskHandle AO2;//single analog voltage out and hold during idle AO2
	NIDAQmx::TaskHandle AO3;//single analog voltage out and hold during idle AO3
	NIDAQmx::TaskHandle AO4;//single analog voltage out and hold during idle AO4
	NIDAQmx::TaskHandle DO0;//single TTL pulse generation on DO0
	NIDAQmx::TaskHandle DO1;//single TTL pulse generation on DO1
	NIDAQmx::TaskHandle DO2;//single TTL pulse generation on DO2
	NIDAQmx::TaskHandle DO3;//single TTL pulse generation on DO3
	NIDAQmx::TaskHandle DO4;//single TTL pulse generation on DO4

	NIDAQmx::TaskHandle analogGen;//analog portion of waveform sent to AO0
	NIDAQmx::TaskHandle digitalGen;//digital portion of waveform sent to DO0, DO1,DO2,DO3 and DO4
	NIDAQmx::int32 sampsPerChan;
	NIDAQmx::float64* analogWaveform;
	int numAnalog;
	int numDigital;
	NIDAQmx::uInt8* digitalWaveform;
	
	NIDAQmx::float64 rate;//in Hz
	char errBuff[2048];
	NIDAQmx::int32 error;
};