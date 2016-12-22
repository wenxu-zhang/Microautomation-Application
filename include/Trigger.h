// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include <vector>
#include "AcquisitionGroup.h"
class Trigger{
protected:
	Trigger(){triggerStartEvent=NULL;}
public:
	HANDLE triggerStartEvent;//passed by protocol user to control timing
	void setTriggerStartEvent(HANDLE &triggerStartEvent){this->triggerStartEvent=triggerStartEvent;}
	void disableTriggerStartEvent(){
		this->triggerStartEvent=NULL;
	}
	virtual ~Trigger(){}//destructors cannot be pure virtual
	//virtual void triggerCam();
	//virtual void triggerDG();
	//virtual void triggerBoth() this function should not be needed but could be implemented later
	virtual void prepareDigitalLine(int line){}//could be used to speed up digital triggering slightly
	virtual void prepareDigitalLines(std::vector<int> &lines){}
	virtual void prepareAnalogLine(int line){}//could be used to speed up analog triggering slightly
	virtual void setLineHigh(int line){}
	virtual void setLinesHigh(std::vector<int> &lines){}
	virtual void setLineLow(int line){}
	virtual void setLinesLow(std::vector<int> &lines){}
	virtual void setVoltage(int line, double voltage){}
	virtual double getVoltage(int line){return 0;}
	virtual void getWaveform(AcquisitionGroup& channels){}
	virtual void setWaveform(AcquisitionGroup& channels){}//will store the desired waveform in memory
	virtual void prepareWaveform(AcquisitionGroup& g){}//will send the data to triggering device if applicabale (i.e. DAQ board but not parallel port)
	virtual bool generateWaveform(HANDLE abortEvent=NULL){return true;}//will start generation of the trigger waveform and return when done....waits for trigger start event if it exists or abortEvent to stop generation and return false.
	virtual void waitGenerationStart(){}
	virtual bool isValidLine(int line){return true;}
	virtual bool isValidAnalogOutLine(int line){return true;}
	virtual bool isValidAnalogInLine(int line){return true;}
	void triggerSingleLine(int line){setLineHigh(line);setLineLow(line);}
	void triggerMultipleLines(std::vector<int> lines){setLinesHigh(lines);setLinesLow(lines);}
	void triggerControl();//gui
};
