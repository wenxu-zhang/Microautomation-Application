#include "Warn.h"
#include "DAQPCI6733.h"
#include <iostream>
#include "CImg.h"
#include "Timer.h"
#include "Controller.h"
#include "Utils.h"
extern Controller cont;
extern Record logFile;
using namespace std;
using namespace NIDAQmx;
//using namespace cimg_library;

#ifdef DEBUGDAQPCI6733
#define DAQmxErrChk(functionCall) GetError(DAQmxFailed(functionCall));
#else
#define DAQmxErrChk(functionCall) functionCall;
#endif

void DAQPCI6733::GetError(bool b){
	if (b){
	DAQmxGetExtendedErrorInfo(errBuff,2048);
	logFile.write(string("Error: DAQmx: ")+errBuff,true);
	clickAbort();
	}
}

void DAQPCI6733::prepareAnalogLine(int line){
	switch(line){
	case 0:
		DAQmxErrChk(DAQmxStopTask(analogGen))
		DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))
		DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Commit))
		return;
	case 1:
		//don't need to unreserve analogGen because this task doesn't use AO0 and it is on-demand software timed so it doesn't use the sample clock
		DAQmxErrChk(DAQmxTaskControl(AO1,DAQmx_Val_Task_Commit))
		return;
	case 2:
		//don't need to unreserve analogGen because this task doesn't use AO0 and it is on-demand software timed so it doesn't use the sample clock
		DAQmxErrChk(DAQmxTaskControl(AO2,DAQmx_Val_Task_Commit))
		return;
	case 3:
		//don't need to unreserve analogGen because this task doesn't use AO0 and it is on-demand software timed so it doesn't use the sample clock
		DAQmxErrChk(DAQmxTaskControl(AO3,DAQmx_Val_Task_Commit))
		return;
	case 4:
		//don't need to unreserve analogGen because this task doesn't use AO0 and it is on-demand software timed so it doesn't use the sample clock
		DAQmxErrChk(DAQmxTaskControl(AO4,DAQmx_Val_Task_Commit))
		return;
	default:
		logFile.write(string("Error DAQUSB6251: Analog Out Line ")+toString(line)+" is not supported on this device",true);
	}
}

void DAQPCI6733::setVoltage(int line, double voltage){
	CheckExists()
	switch(line){
	case 0:
		DAQmxErrChk(DAQmxStopTask(analogGen))
		DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))//could already be unreserved so this may be very quick
		DAQmxErrChk(DAQmxTaskControl(AO0,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxErrChk(DAQmxWriteAnalogScalarF64(AO0,true,0,voltage,NULL))
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO0,1))
		return;
	case 1:
		DAQmxErrChk(DAQmxTaskControl(AO1,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxWriteAnalogScalarF64(AO1,true,0,voltage,NULL);
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO1,1))
		return;
	case 2:
		DAQmxErrChk(DAQmxTaskControl(AO2,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxWriteAnalogScalarF64(AO2,true,0,voltage,NULL);
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO2,1))
		return;
	case 3:
		DAQmxErrChk(DAQmxTaskControl(AO3,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxWriteAnalogScalarF64(AO3,true,0,voltage,NULL);
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO3,1))
		return;
	case 4:
		DAQmxErrChk(DAQmxTaskControl(AO4,DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
		DAQmxWriteAnalogScalarF64(AO4,true,0,voltage,NULL);
		//DAQmxErrChk(DAQmxWaitUntilTaskDone(AO4,1))
		return;
	default:
		cout<<"Analog Out Line "<<line<<" is not supported on this device"<<endl;
	}
}

double DAQPCI6733::getVoltage(int line){
	CheckExists(0);
	logFile.write("Error: NI DAQ PCI6733 does not support analog input",true);
	return 0;
}


void DAQPCI6733::prepareDigitalLine(int line){
	switch(line){
		case 0:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			//DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))//may not be necessary since startTask is never called for this task 
			DAQmxErrChk(DAQmxTaskControl(DO0,DAQmx_Val_Task_Commit))
			return;
		case 1:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
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


void DAQPCI6733::setLineHigh(int line){
	//Timer t(true);
	CheckExists()
	setLine(line,true);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}
void DAQPCI6733::setLineLow(int line){
	//Timer t(true);
	CheckExists()
	setLine(line, false);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}

void DAQPCI6733::setLine(int line, bool high){
	uInt32 val=0;
	if (high) val=0xFFFFFFFF;
	switch(line){
		case 0:{
			//Timer t(true);
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			DAQmxErrChk(DAQmxTaskControl(DO0,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO0,true,0,val,NULL))
			//	t.stopTimer();
			//cout<<"Trigger took "<<t.getTime()<<" ms"<<endl;
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO0,1))
			return;}
		case 1:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			DAQmxErrChk(DAQmxTaskControl(DO1,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO1,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO1,1))
			return;
		case 2:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
			DAQmxErrChk(DAQmxTaskControl(DO2,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO2,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO2,1))
			return;
		case 3:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			DAQmxErrChk(DAQmxTaskControl(DO3,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO3,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO3,1))
			return;
		case 4:
			DAQmxErrChk(DAQmxStopTask(digitalGen))
			DAQmxErrChk(DAQmxTaskControl(DO4,DAQmx_Val_Task_Commit))
			DAQmxErrChk(DAQmxWriteDigitalScalarU32(DO4,true,0,val,NULL))
			//DAQmxErrChk(DAQmxWaitUntilTaskDone(DO4,1))
				return;
		default:
			cout<<"Digital Out Line "<<line<<" is not supported on this device"<<endl;
			return;
	}
}

void DAQPCI6733::setWaveform(AcquisitionGroup channels){
	CheckExists()
	delete analogWaveform;
	delete digitalWaveform;
	sampsPerChan=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		sampsPerChan+=i->ap.exp*rate;
	}
	DAQmxErrChk(DAQmxCfgSampClkTiming(digitalGen,"/Dev2/ao/SampleClock",rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sampsPerChan+2))
	DAQmxErrChk(DAQmxCfgSampClkTiming(analogGen,"",rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,sampsPerChan+2))
	//allocate analog waveform and initialize to starting voltage
	analogWaveform=new float64[numAnalog*(sampsPerChan+2)];
	for(int i=0;i<numAnalog*sampsPerChan+2;i++){
		analogWaveform[i]=channels.acquisitionChannels[0].chan->TIRFangle.position;
	}
	//allocate digital waveform array and initialize to zero
	digitalWaveform=new uInt8[sampsPerChan+2];
	for(int i=0;i<sampsPerChan+2;i++){
		digitalWaveform[i]=0;
	}
	if (channels.acquisitionChannels.front().out->cam->triggerLine>=numDigital || channels.acquisitionChannels.front().out->cam->triggerLine<0){
		logFile.write(string("Error: PCI 6733 camera trigger line ")+toString(channels.acquisitionChannels.front().out->cam->triggerLine)+" is not supported by this device",true);
		clickAbort();
	}
	int32 currentSample=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		//trigger light source leaving one sample low at end for ringbuffer (DG) suppport
		switch(i->chan->lite.ls->triggerOption){
			case 0: //digital only trigger
				if (i->chan->lite.ls->triggerLine>=numDigital || i->chan->lite.ls->triggerLine<0){
					logFile.write(string("Error: PCI 6733 light source digital trigger line ")+toString(i->chan->lite.ls->triggerLine)+" is not supported by this device",true);
					clickAbort();
				}
				for(int32 startSample=currentSample;startSample<currentSample+i->ap.exp*rate-1;startSample++){
					digitalWaveform[startSample]=(1<<i->chan->lite.ls->triggerLine);
				}
				break;
			case 1: //analog only trigger
				if (i->chan->lite.ls->analogTriggerLine>=numAnalog || i->chan->lite.ls->analogTriggerLine<0){
					logFile.write(string("Error: PCI 6733 light source analog trigger line ")+toString(i->chan->lite.ls->analogTriggerLine)+" is not supported by this device",true);
					clickAbort();
				}
				for(int32 startSample=currentSample;startSample<currentSample+i->ap.exp*rate-1;startSample++){
					analogWaveform[startSample+(sampsPerChan+2)*i->chan->lite.ls->analogTriggerLine]=i->intensity;
				}
				break;
			case 2: //trigger both
				if (i->chan->lite.ls->analogTriggerLine>=numAnalog || i->chan->lite.ls->analogTriggerLine<0){
					logFile.write(string("Error: PCI 6733 light source analog trigger line ")+toString(i->chan->lite.ls->analogTriggerLine)+" is not supported by this device",true);
					clickAbort();
				}
				if (i->chan->lite.ls->triggerLine>=numDigital || i->chan->lite.ls->triggerLine<0){
					logFile.write(string("Error: PCI 6733 light source digital trigger line ")+toString(i->chan->lite.ls->triggerLine)+" is not supported by this device",true);
					clickAbort();
				}
				for(int32 startSample=currentSample;startSample<currentSample+i->ap.exp*rate-1;startSample++){
					digitalWaveform[startSample]=(1<<i->chan->lite.ls->triggerLine);
					analogWaveform[startSample+(sampsPerChan+2)*i->chan->lite.ls->analogTriggerLine]=i->intensity;
				}
				break;
		}
	
		//wait for light
		currentSample+=i->chan->lite.ls->delay*rate;
		//trigger cam
		digitalWaveform[currentSample]+=(1<<i->out->cam->triggerLine);
		//hold TIRF angle
		if (cont.g.isPresent){
			if (cont.g.GALVOOUT>=numAnalog || cont.g.GALVOOUT<0){
				logFile.write(string("Error: PCI 6733 galvanometer trigger line ")+toString(cont.g.GALVOOUT)+" is not supported by this device",true);
				clickAbort();
			}
			double position=i->chan->TIRFangle.position;
			for(int32 startSample=currentSample;currentSample<startSample+i->ap.exp*rate+i->chan->lite.ls->delay*rate && currentSample<sampsPerChan;currentSample++){
				analogWaveform[currentSample+(sampsPerChan+2)*cont.g.GALVOOUT]=position;
			}
		}
	}
	//start dummy camera aquisition and advance ring buffer
	digitalWaveform[sampsPerChan]+=(1<<channels.acquisitionChannels.back().chan->lite.ls->triggerLine);
	digitalWaveform[sampsPerChan]+=(1<<channels.acquisitionChannels.back().out->cam->triggerLine);
	
	//maintain TIRF angle for next cycle
	if (cont.g.isPresent){
		analogWaveform[sampsPerChan+(sampsPerChan+2)*cont.g.GALVOOUT]=analogWaveform[sampsPerChan-1+(sampsPerChan+2)*cont.g.GALVOOUT];
		analogWaveform[sampsPerChan+1+(sampsPerChan+2)*cont.g.GALVOOUT]=analogWaveform[sampsPerChan+(sampsPerChan+2)*cont.g.GALVOOUT];
	}
}

void DAQPCI6733::prepareWaveform(){
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
	//DAQmxErrChk(DAQmxStartTask(digitalGen))	autostarting
	DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Commit))
}

void DAQPCI6733::generateWaveform(){
	CheckExists()
	DAQmxErrChk(DAQmxStartTask(analogGen))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(digitalGen,-1))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(analogGen,-1))
}


DAQPCI6733::DAQPCI6733(){
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
		cout<<"DAQPCI6733 board NOT PRESENT"<<endl;
		return;
	}
	error=DAQmxCreateAOVoltageChan(AO0,"Dev2/ao0", NULL,-10,10,DAQmx_Val_Volts ,NULL);
	if (DAQmxFailed(error)){
		isPresent=false;
		DAQmxClearTask(AO0);
		cout<<"DAQPCI6733 board NOT PRESENT"<<endl;
		return;
	}
	DAQmxErrChk(DAQmxCreateTask("AO1",&AO1))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AO1,"Dev2/ao1","",-10,10,DAQmx_Val_Volts ,""))
	DAQmxErrChk(DAQmxCreateTask("AO2",&AO2))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AO2,"Dev2/ao2","",0,5,DAQmx_Val_Volts ,""))
	DAQmxErrChk(DAQmxCreateTask("AO3",&AO3))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AO3,"Dev2/ao3","",0,5,DAQmx_Val_Volts ,""))
	DAQmxErrChk(DAQmxCreateTask("AO4",&AO4))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(AO4,"Dev2/ao4","",0,5,DAQmx_Val_Volts ,""))

	DAQmxErrChk(DAQmxCreateTask("DO0",&DO0))
	DAQmxErrChk(DAQmxCreateDOChan(DO0,"Dev2/port0/line0","",DAQmx_Val_ChanForAllLines))
	DAQmxErrChk(DAQmxStartTask(DO0))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO0,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO1",&DO1))
	DAQmxErrChk(DAQmxCreateDOChan(DO1,"Dev2/port0/line1","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO1,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO2",&DO2))
	DAQmxErrChk(DAQmxCreateDOChan(DO2,"Dev2/port0/line2","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO2,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO3",&DO3))
	DAQmxErrChk(DAQmxCreateDOChan(DO3,"Dev2/port0/line3","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO3,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	DAQmxErrChk(DAQmxCreateTask("DO4",&DO4))
	DAQmxErrChk(DAQmxCreateDOChan(DO4,"Dev2/port0/line4","",DAQmx_Val_ChanForAllLines))
	//DAQmxErrChk(DAQmxCfgSampClkTiming(DO4,"/Dev1/100kHzTimebase",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2))
	
	numAnalog=5;
	string analogChannels=string("Dev2/ao0:")+toString(numAnalog-1);
	DAQmxErrChk(DAQmxCreateTask("analogGen",&analogGen))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(analogGen,analogChannels.c_str(),"",-10,10,DAQmx_Val_Volts,""))
	numDigital=5;
	string digitalChannels=string("Dev2/port0/line0:")+toString(numDigital-1);
	DAQmxErrChk(DAQmxCreateTask("digitalGen",&digitalGen))
	DAQmxErrChk(DAQmxCreateDOChan(digitalGen,digitalChannels.c_str(), "", DAQmx_Val_ChanForAllLines))

	cout<<"DAQPCI6733 board ready"<<endl;
}
DAQPCI6733::~DAQPCI6733(){
	CheckExists()
	DAQmxErrChk(DAQmxClearTask(AO0))
	DAQmxErrChk(DAQmxClearTask(AO1))
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
