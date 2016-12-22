// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include <windows.h>
#include "Trigger.h"
namespace NIDAQmx{
#include "NIDAQmx.h"
}
#include <vector>
class NIDAQ:public Trigger{
public:
	double getVoltage(int line);
	void prepareDigitalLine(int line);
	void prepareDigitalLines(std::vector<int> &lines);
	void prepareAnalogLine(int line);
	void setVoltage(int line,double voltage);
	void setLineHigh(int line);
	void setLinesHigh(std::vector<int> &lines);
	void setLineLow(int line);
	void setLinesLow(std::vector<int> &lines);
	void setLine(int line,bool high);
	void setLines(std::vector<int> &lines,bool high);
	void getWaveform(AcquisitionGroup& channels);//calculate and save waveform data within AcquisitionGroup class
	void setWaveform(AcquisitionGroup& channels);//set configuration and timing for generation
	void prepareWaveform(AcquisitionGroup& g);//write data in preparation for generation
	bool generateWaveform(HANDLE abortEvent=NULL);
	void waitGenerationStart();
	bool isValidLine(int line);
	bool isValidAnalogOutLine(int line);
	bool isValidAnalogInLine(int line);

	bool isPresent;
	NIDAQ(std::string dev);
	~NIDAQ();

private:
	HANDLE generationStart;
	void visualizeDigitalWaveform(AcquisitionGroup& g);
	void visualizeAnalogWaveform(AcquisitionGroup& g);
	void GetError(bool b);
	std::string dev;
	std::vector<NIDAQmx::TaskHandle> analogOutLines;//single analog voltage out and hold during idle
	std::vector<NIDAQmx::TaskHandle> analogInLines;//single analog voltage in
	std::vector<NIDAQmx::TaskHandle> digitalOutLines;//single TTL pulse generation
//	NIDAQmx::TaskHandle analogOutLineReserved;//hack in case a generation does not have an analog out line. we will use this one for the generation
	NIDAQmx::TaskHandle analogGen;//analog portion of waveform sent during scanning
	NIDAQmx::TaskHandle digitalGen;//digital portion of waveform sent during scanning
	NIDAQmx::TaskHandle multiDigitalOutLines;//multiple simultaneous TTL pulse generation
	std::vector<int> lines;//lines used for current multiDigitalOutLines;
	
	NIDAQmx::float64 rate;//in Hz
	char errBuff[2048];
	NIDAQmx::int32 error;
	
	
};