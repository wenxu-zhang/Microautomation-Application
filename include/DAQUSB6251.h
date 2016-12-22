#pragma once
#include <windows.h>
#include "Trigger.h"
namespace NIDAQmx{
#include "NIDAQmx.h"
}
//using namespace NIDAQmx;
class DAQUSB6251:public Trigger{
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

	bool isPresent;
	DAQUSB6251();
	~DAQUSB6251();

private:
	void GetError(bool b);
	NIDAQmx::TaskHandle AO0;//single analog voltage out and hold during idle AO0
	NIDAQmx::TaskHandle AO1;//single analog voltage out and hold during idle AO1
	NIDAQmx::TaskHandle AI0;//read single value from AnalogIn0
	NIDAQmx::TaskHandle AI1;//read single value from AnalogIn1
	NIDAQmx::TaskHandle DO0;//single TTL pulse generation on DO0
	NIDAQmx::TaskHandle DO1;//single TTL pulse generation on DO1
	NIDAQmx::TaskHandle DO2;//single TTL pulse generation on DO2
	NIDAQmx::TaskHandle DO3;//single TTL pulse generation on DO3
	NIDAQmx::TaskHandle DO4;//single TTL pulse generation on DO4

	NIDAQmx::TaskHandle analogGen;//analog portion of waveform sent to AO0
	NIDAQmx::TaskHandle digitalGen;//digital portion of waveform sent to DO0, DO1,DO2,DO3 and DO4
	NIDAQmx::int32 sampsPerChan;
	NIDAQmx::float64* analogWaveform;
	NIDAQmx::uInt8* digitalWaveform;
	
	NIDAQmx::float64 rate;//in Hz
	char errBuff[2048];
	NIDAQmx::int32 error;
};