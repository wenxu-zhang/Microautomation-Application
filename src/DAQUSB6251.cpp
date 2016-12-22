#include "Warn.h"
#include "DAQUSB6251.h"
#include <iostream>
#include "CImg.h"
#include "Timer.h"
#include "Controller.h"
extern Controller cont;
extern Record logFile;
using namespace std;
using namespace NIDAQmx;
//using namespace cimg_library;

#ifdef DEBUGDAQUSB6251
#define DAQmxErrChk(functionCall) GetError(DAQmxFailed(functionCall));
#else
#define DAQmxErrChk(functionCall) functionCall;
#endif
void DAQUSB6251::GetError(bool b){
	if (b){
	DAQmxGetExtendedErrorInfo(errBuff,2048);
	logFile.write(string("Error: DAQmx: ")+errBuff,true);
	clickAbort();
	}
}

void DAQUSB6251::prepareAnalogLine(int line){
	switch(line){
	case 0:
		DAQmxErrChk(DAQmxStopTask(analogGen))
		DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))
		DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Commit))
		return;
	case 1:
		//don't need to unreserve analogGen because this task doesn't use AO0 and it is on-demand software timed so it doesn't use the sample clock
		DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Commit))
		return;
	default:
		logFile.write(string("Error DAQUSB6251: Analog Out Line ")+toString(line)+" is not supported on this device",true);
	}
}

void DAQUSB6251::setVoltage(int line, double voltage){
	CheckExists()
	switch(line){
	case 0:
		DAQmxErrChk(DAQmxStopTask(analogGen))
		DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))//could already be unreserved so this may be very quick
		DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxErrChk(DAQmxWriteAnalogScalarF64(AO0,true,0,voltage,NULL))
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO0,1)) //is this necessary for writing only one value?
		return;
	case 1:
		//DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve);//could already be unreserved so this may be very quick
		DAQmxErrChk(DAQmxTaskControl(AO1,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxErrChk(DAQmxWriteAnalogScalarF64(AO1,true,0,voltage,NULL))
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO1,1))
		return;
	default:
		logFile.write(string("Error DAQUSB6251: Analog Out Line ")+toString(line)+" is not supported on this device",true);
	}
}

double DAQUSB6251::getVoltage(int line){
	CheckExists(0);
	float64 ret;
	switch(line){
	case 0:
		DAQmxErrChk(DAQmxReadAnalogScalarF64(AI0,0,&ret,NULL))
		DAQmxErrChk(DAQmxWaitUntilTaskDone(AI0,1))
		//DAQmxErrChk(DAQmxStopTask(AI0))
		return ret;
	case 1:
		DAQmxErrChk(DAQmxReadAnalogScalarF64(AI1,0,&ret,NULL))
		DAQmxErrChk(DAQmxWaitUntilTaskDone(AI1,1))
		//DAQmxErrChk(DAQmxStopTask(AI1))
		return ret;
	default:
		cout<<"Analog In Line "<<line<<" is not supported on this device"<<endl;
		return 0;
	}
}

void DAQUSB6251::prepareDigitalLine(int line){
	switch(line){
		case 0:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))//may not be necessary since startTask is never called for this task 
			DAQmxErrChk(DAQmxTaskControl(DO0,DAQmx_Val_Task_Commit))
			return;
		case 1:
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO1,DAQmx_Val_Task_Commit))
			return;
		case 2:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO2,DAQmx_Val_Task_Commit))
			return;
		case 3:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO3,DAQmx_Val_Task_Commit))
			return;
		case 4:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO4,DAQmx_Val_Task_Commit))
				return;
		default:
			cout<<"Digital Out Line "<<line<<" is not supported on this device"<<endl;
			return;
	}
}

void DAQUSB6251::setLineHigh(int line){
	CheckExists()
	setLine(line,true);
}
void DAQUSB6251::setLineLow(int line){
	CheckExists()
	setLine(line, false);
}

void DAQUSB6251::setLine(int line, bool high){
	uInt32 val=0;
	if (high) val=0xFFFFFFFF; 
	switch(line){
		case 0:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO0,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO0,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO0,1))
			//DAQmxErrChk(DAQmxStopTask(DO0))
			return;
		case 1:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO1,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO1,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO1,1))
			//DAQmxErrChk(DAQmxStopTask(DO1))
			return;
		case 2:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO2,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO2,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO2,1))
			//DAQmxErrChk(DAQmxStopTask(DO2))
			return;
		case 3:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO3,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO3,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO3,1))
			//DAQmxErrChk(DAQmxStopTask(DO3))
			return;
		case 4:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO4,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO4,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO4,1))
			//DAQmxErrChk(DAQmxStopTask(DO4))
			return;
		default:
			cout<<"Digital Out Line "<<line<<" is not supported on this device"<<endl;
			return;
	}
}

void DAQUSB6251::setWaveform(AcquisitionGroup channels){
	CheckExists()
	delete analogWaveform;
	delete digitalWaveform;
	sampsPerChan=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		sampsPerChan+=i->ap.exp*rate;
	}
	DAQmxErrChk(DAQmxCfgSampClkTiming(digitalGen,"/Dev1/ao/SampleClock",rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sampsPerChan+2))
	DAQmxErrChk(DAQmxCfgSampClkTiming(analogGen,"",rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sampsPerChan+2))
	
	//allocate analog waveform and initialize to starting voltage
	sampsPerChan=sampsPerChan+2;//need two more samples to return to default
	analogWaveform=new float64[sampsPerChan+2];
	for(int i=0;i<sampsPerChan+2;i++){
		analogWaveform[i]=channels.acquisitionChannels[0].chan->TIRFangle.position;
	}
	//allocate digital waveform array and initialize to zero
	digitalWaveform=new uInt8[sampsPerChan+2];
	for(int i=0;i<sampsPerChan+2;i++){
		digitalWaveform[i]=0;
	}
	int32 currentSample=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		//trigger light source leaving one sample low at end for DG suppport
		for(int32 startSample=currentSample;startSample<currentSample+i->ap.exp*rate-1;startSample++){
			digitalWaveform[startSample]=(1<<i->chan->lite.ls->triggerLine);
		}
		//wait for light
		currentSample+=i->chan->lite.ls->delay*rate;
		//trigger cam
		digitalWaveform[currentSample]+=(1<<i->out->cam->triggerLine);
		//hold TIRF angle
		double position=i->chan->TIRFangle.position;
		for(int32 startSample=currentSample;currentSample<startSample+i->ap.exp*rate+i->chan->lite.ls->delay*rate && currentSample<sampsPerChan;currentSample++){
			analogWaveform[currentSample]=position;
		}
	}
	//start dummy camera aquisition and advance ring buffer
	digitalWaveform[sampsPerChan]+=(1<<channels.acquisitionChannels.back().chan->lite.ls->triggerLine);
	digitalWaveform[sampsPerChan]+=(1<<channels.acquisitionChannels.back().out->cam->triggerLine);
	//maintain TIRF angle for next cycle
	analogWaveform[sampsPerChan]=analogWaveform[sampsPerChan-1];
	analogWaveform[sampsPerChan+1]=analogWaveform[sampsPerChan];
/*
	for (int i=0;i<sampsPerChan+2;i++){
		cout<<toString(i)<<": "<<toBinary(digitalWaveform[i])<<endl;
	}
	*/
}

void DAQUSB6251::prepareWaveform(){
	CheckExists()
	int32 sampsWritten;
	DAQmxErrChk(DAQmxStopTask(digitalGen))
	DAQmxErrChk(DAQmxStopTask(analogGen))
	DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(DO0,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(DO1,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(DO2,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(DO3,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(DO4,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxWriteDigitalU8(digitalGen,sampsPerChan,true,-1,DAQmx_Val_GroupByChannel,digitalWaveform,&sampsWritten,NULL))
	DAQmxErrChk(DAQmxWriteAnalogF64(analogGen,sampsPerChan,false,-1,DAQmx_Val_GroupByChannel,analogWaveform,&sampsWritten,NULL))
	DAQmxErrChk(DAQmxStartTask(digitalGen))	
	DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Commit))
}

void DAQUSB6251::generateWaveform(){
	CheckExists()
	DAQmxErrChk(DAQmxStartTask(analogGen))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(digitalGen,-1))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(analogGen,-1))
}


DAQUSB6251::DAQUSB6251(){
	rate=2000.0;//500usec or 2kHz
	sampsPerChan=2;
	analogWaveform=NULL;
	digitalWaveform=NULL;
	isPresent=true;
	error=0;
	errBuff[0]='\0';
	error=DAQmxCreateTask("AO0",&AO0);
	if (DAQmxFailed(error)){
		isPresent=false;
		cout<<"DAQUSB6251 board NOT PRESENT"<<endl;
		return;
	}
	error=DAQmxCreateAOVoltageChan(AO0,"Dev1/ao0", NULL,-10,10,DAQmx_Val_Volts ,NULL);
	if (DAQmxFailed(error)){
		isPresent=false;
		DAQmxClearTask(AO0);
		cout<<"DAQUSB6251 board NOT PRESENT"<<endl;
		return;
	}
	DAQmxErrChk(DAQmxCreateTask("AO1",&AO1))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AO1,"Dev1/ao1","",-10,10,DAQmx_Val_Volts ,""))
	DAQmxErrChk(DAQmxCreateTask("AI0",&AI0))
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AI0,"Dev1/ai0","",DAQmx_Val_NRSE ,-10,10,DAQmx_Val_Volts ,NULL))
	DAQmxErrChk(DAQmxCreateTask("AI1",&AI1))
	DAQmxErrChk(DAQmxCreateAIVoltageChan(AI1,"Dev1/ai1","",DAQmx_Val_NRSE ,-10,10,DAQmx_Val_Volts ,NULL))
	DAQmxErrChk(DAQmxCreateTask("DO0",&DO0))
	DAQmxErrChk(DAQmxCreateDOChan(DO0,"Dev1/port0/line0","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO0,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO1",&DO1))
	DAQmxErrChk(DAQmxCreateDOChan(DO1,"Dev1/port0/line1","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO1,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO2",&DO2))
	DAQmxErrChk(DAQmxCreateDOChan(DO2,"Dev1/port0/line2","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO2,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO3",&DO3))
	DAQmxErrChk(DAQmxCreateDOChan(DO3,"Dev1/port0/line3","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO3,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO4",&DO4))
	DAQmxErrChk(DAQmxCreateDOChan(DO4,"Dev1/port0/line4","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO4,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	
	DAQmxErrChk(DAQmxCreateTask("analogGen",&analogGen))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(analogGen,"Dev1/ao0","",-10,10,DAQmx_Val_Volts,""))
	DAQmxErrChk(DAQmxCreateTask("digitalGen",&digitalGen))
	DAQmxErrChk(DAQmxCreateDOChan(digitalGen,"Dev1/port0/line0:7", "", DAQmx_Val_ChanForAllLines))

	cout<<"DAQUB6251 board ready"<<endl;
}
DAQUSB6251::~DAQUSB6251(){
	CheckExists()
	DAQmxErrChk(DAQmxClearTask(AO0))
	DAQmxErrChk(DAQmxClearTask(AO1))
	DAQmxErrChk(DAQmxClearTask(AI0))
	DAQmxErrChk(DAQmxClearTask(AI1))
	DAQmxErrChk(DAQmxClearTask(DO0))
	DAQmxErrChk(DAQmxClearTask(DO1))
	DAQmxErrChk(DAQmxClearTask(DO2))
	DAQmxErrChk(DAQmxClearTask(DO3))
	DAQmxErrChk(DAQmxClearTask(DO4))

	DAQmxErrChk(DAQmxClearTask(analogGen))
	DAQmxErrChk(DAQmxClearTask(digitalGen))
	delete [] analogWaveform;
	delete [] digitalWaveform;
}
