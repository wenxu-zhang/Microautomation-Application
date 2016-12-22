// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, May 12, 2011</lastedit>
// ===================================
#include "Warn.h"
#include "NIDAQ.h"
#include <iostream>
#include "CImg.h"
#include "Timer.h"
#include "Controller.h"
#include "Utils.h"
extern Controller cont;
extern Record logFile;
using namespace std;
using namespace NIDAQmx;
using NIDAQmx::int32;
//using namespace cimg_library;

#ifdef DEBUGNIDAQ
#define DAQmxErrChk(functionCall) GetError(DAQmxFailed(functionCall));
#else
#define DAQmxErrChk(functionCall) functionCall;
#endif

void NIDAQ::GetError(bool b){
	if (b){
	DAQmxGetExtendedErrorInfo(errBuff,2048);
	logFile.write(string("Error: DAQmx: ")+errBuff,true);
	system("pause");
	clickAbort();
	}
}

bool NIDAQ::isValidLine(int line){
	if (digitalOutLines.size()==0) return false;
	if (line<0 || line>=digitalOutLines.size()) return false;
	return true;
}

bool NIDAQ::isValidAnalogOutLine(int line){
	if (analogOutLines.size()==0) return false;
	if (line<0 || line>=analogOutLines.size()) return false;
	return true;
}

bool NIDAQ::isValidAnalogInLine(int line){
	if (analogInLines.size()==0) return false;
	if (line<0 || line>=analogInLines.size()) return false;
	return true;
}

void NIDAQ::prepareAnalogLine(int line){
	CheckExists()
	NIDAQmx::bool32 b;
	DAQmxErrChk(DAQmxGetTaskComplete(analogGen,&b))
	if (!b)
		logFile.write("DAQ Error: Attempt to use analog channel that is in use by another task...stopping the other task",true);
	DAQmxErrChk(DAQmxStopTask(analogGen))
	DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(analogOutLines[line],DAQmx_Val_Task_Commit))
}

void NIDAQ::setVoltage(int line, double voltage){
	CheckExists()
	NIDAQmx::bool32 b;
	DAQmxErrChk(DAQmxGetTaskComplete(analogGen,&b))
	if (!b){
		logFile.write("DAQ Error: Attempt to use analog channel that is in use...waiting until it is done",true);
		return;
		DAQmxErrChk(DAQmxWaitUntilTaskDone(analogGen,-1))
	}
	DAQmxErrChk(DAQmxStopTask(analogGen))
	DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Unreserve))//could already be unreserved so this may be very quick
	DAQmxErrChk(DAQmxTaskControl(analogOutLines[line],DAQmx_Val_Task_Commit))//could already be committed so this may be very quick
	DAQmxErrChk(DAQmxWriteAnalogScalarF64(analogOutLines[line],true,0,voltage,NULL))
}

double NIDAQ::getVoltage(int line){
	CheckExists(0)
	double ret;
	DAQmxErrChk(DAQmxReadAnalogScalarF64(analogInLines[line],0,&ret,NULL))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(analogInLines[line],1))
	return ret;
}

void NIDAQ::prepareDigitalLine(int line){
	prepareDigitalLines(vector<int>(1,line));
}

void NIDAQ::prepareDigitalLines(vector<int> &lines){
	CheckExists()
	if (lines.empty()){
		logFile.write("Error: attempt to prepare digital lines when no lines were specified",true);
		return;
	}
	if (lines!=this->lines){
		DAQmxErrChk(DAQmxClearTask(multiDigitalOutLines))
		string channel=string(dev+"/port0/line")+toString((lines.front()));
		for(vector<int>::iterator i=lines.begin()+1;i!=lines.end();i++){
			channel+=string(", ")+dev+"/port0/line"+toString(*i);
		}
		DAQmxErrChk(DAQmxCreateTask("multiDigitalOutLines",&multiDigitalOutLines))
		DAQmxErrChk(DAQmxCreateDOChan(multiDigitalOutLines,channel.c_str(),"",DAQmx_Val_ChanForAllLines))
	}
	DAQmxErrChk(DAQmxStopTask(digitalGen))
	DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(multiDigitalOutLines,DAQmx_Val_Task_Commit))
	this->lines=lines;
}

void NIDAQ::setLineHigh(int line){
	//Timer t(true);
	CheckExists()
	setLine(line,true);
	SetEvent(generationStart);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}

void NIDAQ::setLinesHigh(vector<int>& lines){
	//Timer t(true);
	CheckExists()
	setLines(lines,true);
	SetEvent(generationStart);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}

void NIDAQ::setLineLow(int line){
	//Timer t(true);
	CheckExists()
	setLine(line, false);
	ResetEvent(generationStart);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}

void NIDAQ::setLinesLow(vector<int> &lines){
	//Timer t(true);
	CheckExists()
	setLines(lines, false);
	ResetEvent(generationStart);
	//t.stopTimer();
	//cout<<"setLine took "<<t.getTime()<<" ms"<<endl;
}

void NIDAQ::setLine(int line, bool high){
	setLines(vector<int>(1,line),high);
}

void NIDAQ::setLines(vector<int> &lines, bool high){
	uInt32 val=0;
	NIDAQmx::bool32 b;
	if (lines!=this->lines)
		prepareDigitalLines(lines);
	DAQmxErrChk(DAQmxGetTaskComplete(digitalGen,&b))
	if (!b){
		logFile.write("DAQ Error: Attempt to use digital channel that is in use...waiting until it is done",true);
		DAQmxErrChk(DAQmxWaitUntilTaskDone(digitalGen,-1))
	}
	if (high) val=0xFFFFFFFF;
	DAQmxErrChk(DAQmxStopTask(digitalGen))
	DAQmxErrChk(DAQmxTaskControl(digitalGen,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(multiDigitalOutLines,DAQmx_Val_Task_Commit))
	DAQmxErrChk(DAQmxWriteDigitalScalarU32(multiDigitalOutLines,true,0,val,NULL))
}

void NIDAQ::getWaveform(AcquisitionGroup& channels){
		CheckExists()
	if (channels.acquisitionChannels.size()==0){
		logFile.write("Error: AcquisitionGroup contains no channels",true);
		return;
	}
	channels.sampsPerChan=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		channels.sampsPerChan+=i->ap.exp*rate;
	}
	int moreSamps=2;
	if (channels.sampsPerChan % 2 != 0) moreSamps=3;
	

	//determine how many analog waveforms
	//lookup table to convert trigger line to analog channel index number
	for(int i=0;i<analogOutLines.size();i++){
		channels.conversion.push_back(-1);
	}
	channels.pChannels="";
	channels.index=0;
	for(vector<AcquisitionChannel>::const_iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		if (i->chan->lite().ls->cp->triggerOption==1 || i->chan->lite().ls->cp->triggerOption==2){//make sure analog trigger line is valid
			if (channels.conversion[i->chan->lite().ls->cp->analogTriggerLine]==-1){
				channels.conversion[i->chan->lite().ls->cp->analogTriggerLine]=channels.index;
				channels.pChannels+=dev+"/ao"+toString(i->chan->lite().ls->cp->analogTriggerLine)+", ";
				channels.index++;
			}
		}
		if (i->chan->tirf){
			if (channels.conversion[i->chan->tirf->getTriggerLine()]==-1){
				channels.conversion[i->chan->tirf->getTriggerLine()]=channels.index;
				channels.pChannels+=dev+"/ao"+toString(i->chan->tirf->getTriggerLine())+", ";
				channels.index++;
			}
		}
	}
	//allocate analog waveform and initialize to zero
	if (channels.index==0) {
		channels.index=1;//hack so we have at least one analog trigger
		if (this->isValidAnalogOutLine(cont.tirf.getTriggerLine()))
			channels.pChannels=dev+"/ao"+toString(cont.tirf.getTriggerLine())+", ";
		else
			channels.pChannels=dev+"/ao"+toString((int)(analogOutLines.size()-1))+", ";
	}
	channels.pChannels=channels.pChannels.substr(0,channels.pChannels.length()-2);
	channels.analogWaveform=new float64[channels.index*(channels.sampsPerChan+moreSamps)];
	for(int i=0;i<channels.index*(channels.sampsPerChan+moreSamps);i++){
		channels.analogWaveform[i]=0;//channels.acquisitionChannels[0].chan->TIRFangle.position;
	}
	//make sure light sources are off (voltage might not be zero e.g. galvo-modulated light sources);
	for(set<LightSource*>::iterator light=channels.lights.begin();light!=channels.lights.end();light++){
		if ((*light)->cp->triggerOption>0){
			for(int j=0;j<channels.sampsPerChan+moreSamps;j++){
				int what=j+(channels.sampsPerChan+moreSamps)*channels.conversion[(*light)->cp->analogTriggerLine];
				int what2=channels.conversion[(*light)->cp->analogTriggerLine];
				channels.analogWaveform[j+(channels.sampsPerChan+moreSamps)*channels.conversion[(*light)->cp->analogTriggerLine]]=(*light)->cp->zeroIntensity;//channels.acquisitionChannels[0].chan->TIRFangle.position;
				//float64 what=analogWaveform[j+(sampsPerChan+moreSamps)*conversion[i->chan->lite().ls->analogTriggerLine]];
			}
		}
	}
	//allocate digital waveform array and initialize to zero
	channels.digitalWaveform=new uInt8[channels.sampsPerChan+moreSamps];
	for(int i=0;i<channels.sampsPerChan+moreSamps;i++){
		channels.digitalWaveform[i]=0;
	}
	int32 currentSample=0;
	//trigger all light sources that use a ring buffer so that they advance to the first position (could be a blank position)
		for(set<LightSource*>::iterator light=channels.lights.begin();light!=channels.lights.end();light++){
			if ((*light)->supportsRingBuffer()){
				if ((*light)->cp->triggerOption==0)//digital only
					channels.digitalWaveform[currentSample]|=(1<<(*light)->cp->triggerLine);
				else//no support for other triggering formats right now
					logFile.write("Error: only ring buffer with digital triggering is supported at the moment",true);
			}
		}
	for(vector<AcquisitionChannel>::iterator i=channels.acquisitionChannels.begin();i!=channels.acquisitionChannels.end();i++){
		for(int32 startSample=currentSample;startSample<currentSample+int(i->ap.exp*rate);startSample++){
			//trigger this light source or these light sources as long as they are not ring buffer format
			int k=0;
			for(vector<Light>::iterator lt=i->chan->lites.begin();lt!=i->chan->lites.end();k++,lt++){
				if ((startSample-currentSample) >= int(i->ap.getStart(k)*rate) && (startSample-currentSample)<int(i->ap.getEnd(k)*rate) && !lt->ls->supportsRingBuffer()){
					switch(i->chan->lite().ls->cp->triggerOption){
						case 0: //digital only trigger
							channels.digitalWaveform[startSample]=(1<<i->chan->lite().ls->cp->triggerLine);
							break;
						case 1: //analog only trigger
							channels.analogWaveform[startSample+(channels.sampsPerChan+moreSamps)*channels.conversion[i->chan->lite().ls->cp->analogTriggerLine]]=i->intensity;
							break;
						case 2: //trigger both
							channels.digitalWaveform[startSample]=(1<<i->chan->lite().ls->cp->triggerLine);
							channels.analogWaveform[startSample+(channels.sampsPerChan+moreSamps)*channels.conversion[i->chan->lite().ls->cp->analogTriggerLine]]=i->intensity;
							break;
					}
				}
			}
			
			//hold TIRF angle
			if (i->chan->tirf){
					channels.analogWaveform[startSample+(channels.sampsPerChan+moreSamps)*channels.conversion[i->chan->tirf->getTriggerLine()]]=i->TIRFangle;
			}
		}	
		//trigger all light sources that use a ring buffer with blank positions (e.g. DG) so that they advance to the next position (could be a blank position)
		for(set<LightSource*>::iterator light=channels.lights.begin();light!=channels.lights.end();light++){
			if ((*light)->supportsRingBuffer()){
				if ((*light)->cp->triggerOption==0)//digital only
					channels.digitalWaveform[currentSample+int(i->ap.exp*rate)-2]|=(1<<(*light)->cp->triggerLine);
				else//no support for other triggering formats right now
					logFile.write("Error: only ring buffer with digital triggering is supported at the moment",true);
			}
		}

		//trigger camera triggers
		int k=0;
		for(vector<int>::iterator lines=i->out->cam->triggerLines.begin();lines!=i->out->cam->triggerLines.end();lines++,k++){
			channels.digitalWaveform[currentSample+int(i->ap.getCamDelay(k)*rate)]|=(1<<*lines);
		}
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+1]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+2]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+3]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+4]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+5]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+6]|=(1<<i->out->cam->triggerLine);
		//channels.digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)+7]|=(1<<i->out->cam->triggerLine);
		//cout<<(unsigned int) digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)]<<endl;
		

		//cout<<(unsigned int) digitalWaveform[currentSample+int(i->chan->lite().ls->delay*rate)]<<endl;
		//cout<<(unsigned int) digitalWaveform[currentSample]<<endl;
		//cout<<currentSample<<endl;
		//advance currentSample
		currentSample+=i->ap.exp*rate;
		//visualizeDigitalWaveform(digitalWaveform,this->sampsPerChan+2);
	}
	//start dummy camera aquisition by triggering at second to last sample
	int k=0;
	for(vector<int>::iterator lines=channels.acquisitionChannels.back().out->cam->triggerLines.begin();lines!=channels.acquisitionChannels.back().out->cam->triggerLines.end();lines++,k++){
		channels.digitalWaveform[channels.sampsPerChan]|=(1<<*lines);
	}
//	channels.digitalWaveform[channels.sampsPerChan]=(1<<channels.acquisitionChannels.back().out->cam->triggerLines.front());
//	analogWaveform[sampsPerChan+1+(sampsPerChan+moreSamps)*conversion[channels.acquisitionChannels.front().chan->lite().ls->analogTriggerLine]]=10;
//if (moreSamps==3)
//	analogWaveform[sampsPerChan+2+(sampsPerChan+moreSamps)*conversion[channels.acquisitionChannels.front().chan->lite().ls->analogTriggerLine]]=10;
	
	//maintain first TIRF angle for next cycle just in case there is only one acquisitionGroup
	if (channels.acquisitionChannels.front().chan->tirf){
		channels.analogWaveform[channels.sampsPerChan+(channels.sampsPerChan+moreSamps)*channels.conversion[channels.acquisitionChannels.front().chan->tirf->getTriggerLine()]]=channels.analogWaveform[(channels.sampsPerChan+moreSamps)*channels.conversion[channels.acquisitionChannels.front().chan->tirf->getTriggerLine()]];
		channels.analogWaveform[channels.sampsPerChan+1+(channels.sampsPerChan+moreSamps)*channels.conversion[channels.acquisitionChannels.front().chan->tirf->getTriggerLine()]]=channels.analogWaveform[(channels.sampsPerChan+moreSamps)*channels.conversion[channels.acquisitionChannels.front().chan->tirf->getTriggerLine()]];
	}
	//make sure sampsPerChan is correct during write
	channels.sampsPerChan+=moreSamps;
//	for(int i=0;i<2*channels.sampsPerChan;i++){
//		NIDAQmx::float64 f=channels.analogWaveform[i];//channels.acquisitionChannels[0].chan->TIRFangle.position;
//	}
	//visualizeDigitalWaveform(channels);
	//visualizeAnalogWaveform(channels);
}

void NIDAQ::setWaveform(AcquisitionGroup& channels){
	CheckExists()
	
	DAQmxErrChk(DAQmxClearTask(analogGen))
	DAQmxErrChk(DAQmxCreateTask("analogGen",&analogGen))
	DAQmxErrChk(DAQmxCreateAOVoltageChan(analogGen,channels.pChannels.c_str(),"",-10,10,DAQmx_Val_Volts,""))
	DAQmxErrChk(DAQmxStopTask(digitalGen))
	string sampleClock="ao/SampleClock";
	DAQmxErrChk(DAQmxCfgSampClkTiming(digitalGen,sampleClock.c_str(),rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,channels.sampsPerChan))
	DAQmxErrChk(DAQmxCfgSampClkTiming(analogGen,"",rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,channels.sampsPerChan))
}

void NIDAQ::visualizeDigitalWaveform(AcquisitionGroup& g){
	CImg<NIDAQmx::uInt8> image(g.digitalWaveform,g.sampsPerChan,1,1,1,false);
	CImg<NIDAQmx::uInt8> chan0=(image&0x01) << 7;
	CImg<NIDAQmx::uInt8> chan1=(image&0x02) << 6;
	CImg<NIDAQmx::uInt8> chan2=(image&0x04) << 5;
	CImg<NIDAQmx::uInt8> chan3=(image&0x08) << 4;
	CImg<NIDAQmx::uInt8> chan4=(image&0x10) << 3;
	CImg<NIDAQmx::uInt8> chan5=(image&0x20) << 2;
	CImg<NIDAQmx::uInt8> chan6=(image&0x40) << 1;
	CImg<NIDAQmx::uInt8> chan7=(image&0x80) << 0;
	for(int i=1;i<10;i++){
		chan0=chan0.append(chan0,'y','c');
		chan1=chan1.append(chan1,'y','c');
		chan2=chan2.append(chan2,'y','c');
		chan3=chan3.append(chan3,'y','c');
		chan4=chan4.append(chan4,'y','c');
		chan5=chan5.append(chan5,'y','c');
		chan6=chan6.append(chan6,'y','c');
		chan7=chan7.append(chan7,'y','c');
	}
	vector<CImg<NIDAQmx::uInt8>> lineImages;
	lineImages.push_back(chan0);
	lineImages.push_back(chan1);
	lineImages.push_back(chan2);
	lineImages.push_back(chan3);
	lineImages.push_back(chan4);
	lineImages.push_back(chan5);
	lineImages.push_back(chan6);
	lineImages.push_back(chan7);
	for(int j=0;j<4;j++){
	for(int i=0;i<chan1.width();i=i+1920){
		//CImgDisplay disp(min(1920,chan1.width()-i),512,string(string("digital chan")+toString(j)+" "+"lines "+toString(i)+"-"+toString(min(i+1919,chan1.width()-1))).c_str());
		CImgDisplay disp=CImgDisplay(min(1920,chan1.width()-i),lineImages.front().height(),string(string("digital chan")+toString(j)+" "+"lines "+toString(i)+"-"+toString(min(i+1919,chan1.width()-1))).c_str());	
		lineImages.at(j).get_columns(i,min(i+1919,chan1.width()-1)).display(disp,true);
		//system("pause");
	}
	}
}

void NIDAQ::visualizeAnalogWaveform(AcquisitionGroup& g){
	vector<CImg<NIDAQmx::float64>> lineImages;
	vector<int> chan;
	int i=0;
	for(vector<int>::iterator v=g.conversion.begin();v!=g.conversion.end();v++,i++){
		if (*v==-1) continue;
		lineImages.push_back(CImg<NIDAQmx::float64>(g.analogWaveform+g.conversion.at(i)*g.sampsPerChan,g.sampsPerChan,1));
		chan.push_back(i);
	}
	for(int i=1;i<10;i++){
		for (vector<CImg<NIDAQmx::float64>>::iterator i=lineImages.begin();i!=lineImages.end();i++){
			*i=i->append(*i,'y','c');
		}
	}
	for(int j=0;j<lineImages.size();j++){
		for(int i=0;i<lineImages.front().width();i=i+1920){
		CImgDisplay disp(min(1920,lineImages.front().width()-i),lineImages.front().height(),string(string("analog chan")+toString(chan.at(j))+" "+"lines "+toString(i)+"-"+toString(min(i+1919,lineImages.front().width()-1))).c_str());
		lineImages.at(j).get_columns(i,min(i+1919,lineImages.front().width()-1)).display(disp,true);
		//system("pause");
	}
	}
}

void NIDAQ::prepareWaveform(AcquisitionGroup& channels){
	CheckExists()
	int32 sampsWritten;
	DAQmxErrChk(DAQmxStopTask(digitalGen))
	DAQmxErrChk(DAQmxStopTask(analogGen))
	for(vector<TaskHandle>::iterator a=analogOutLines.begin();a!=analogOutLines.end();a++){
		DAQmxErrChk(DAQmxTaskControl(*a,DAQmx_Val_Task_Unreserve))
	}
//	DAQmxErrChk(DAQmxTaskControl(analogOutLineReserved,DAQmx_Val_Task_Unreserve))
	DAQmxErrChk(DAQmxTaskControl(multiDigitalOutLines,DAQmx_Val_Task_Unreserve))
	//for(int i=0;i<2*channels.sampsPerChan;i++){
	//	NIDAQmx::float64 f=channels.analogWaveform[i];//channels.acquisitionChannels[0].chan->TIRFangle.position;
	//}
	DAQmxErrChk(DAQmxWriteDigitalU8(digitalGen,channels.sampsPerChan,true,-1,DAQmx_Val_GroupByChannel,channels.digitalWaveform,&sampsWritten,NULL))
	DAQmxErrChk(DAQmxWriteAnalogF64(analogGen,channels.sampsPerChan,false,-1,DAQmx_Val_GroupByChannel,channels.analogWaveform,&sampsWritten,NULL))
	//DAQmxErrChk(DAQmxStartTask(digitalGen))	autostarting
	DAQmxErrChk(DAQmxTaskControl(analogGen,DAQmx_Val_Task_Commit))
}

bool NIDAQ::generateWaveform(HANDLE abortEvent){
	CheckExists(true)
	//this->visualizeDigitalWaveform(digitalWaveform,sampsPerChan);
	if (triggerStartEvent && !abortEvent) 
		WaitForSingleObject(triggerStartEvent,INFINITE);
	else if (triggerStartEvent && abortEvent){
		HANDLE h[]={abortEvent,triggerStartEvent};//give preference to abortEvent
		DWORD eventNum=WaitForMultipleObjects(2,h,false,INFINITE);
		if (eventNum==WAIT_OBJECT_0)
			return false;
	}else if (WAIT_OBJECT_0==WaitForSingleObject(abortEvent,0))
		return false;
	DAQmxErrChk(DAQmxStartTask(analogGen))
	SetEvent(generationStart);
	DAQmxErrChk(DAQmxWaitUntilTaskDone(digitalGen,-1))
	DAQmxErrChk(DAQmxWaitUntilTaskDone(analogGen,-1))
	ResetEvent(generationStart);
	return true;
}

void NIDAQ::waitGenerationStart(){
	WaitForSingleObject(generationStart,INFINITE);
}

NIDAQ::NIDAQ(string dev="Dev1"):
dev(dev),
rate(2000.0),
isPresent(true),
error(0)
{
	generationStart=CreateEvent(NULL,TRUE,FALSE,NULL);//after generation the event will be reset manually
	//analogWaveform[0]=0;
	//analogWaveform[1]=0;
	//digitalWaveform[0]=0;
	//digitalWaveform[1]=0;
	errBuff[0]='\0';
	error=DAQmxSuccess;
	TaskHandle task;
	string taskName,channel;
	//digital lines
	while(true){
		taskName=string("DO")+toString(int(digitalOutLines.size()));
		DAQmxErrChk(DAQmxCreateTask(taskName.c_str(),&task))
		channel=dev+"/port0/line"+toString(int(digitalOutLines.size()));
		error=DAQmxCreateDOChan(task,channel.c_str(),"",DAQmx_Val_ChanForAllLines);
		if (DAQmxFailed(error)){
			DAQmxErrChk(DAQmxClearTask(task))
			break;
		}
		digitalOutLines.push_back(task);
		if (digitalOutLines.size()==8)//we are writing to device using uInt8 so we can only support 8 lines
			break;
	}
	if (digitalOutLines.size()==0){
		logFile.write(string("Error: NIDAQ ")+dev+" NOT PRESENT",true);
		isPresent=false;
		return;
	}
	//analog out lines
	while(true){
		taskName=string("AO")+toString(int(analogOutLines.size()));
		DAQmxErrChk(DAQmxCreateTask(taskName.c_str(),&task))
		channel=dev+"/ao"+toString(int(analogOutLines.size()));
		error=DAQmxCreateAOVoltageChan(task,channel.c_str(),"",-10,10,DAQmx_Val_Volts ,"");
		if (DAQmxFailed(error)){
			DAQmxErrChk(DAQmxClearTask(task))
			break;
		}
		analogOutLines.push_back(task);
	}
//	analogOutLineReserved=analogOutLines.back();
//	analogOutLines.pop_back();
	//analog in lines
	while(true){
		taskName=string("AI")+toString(int(analogInLines.size()));
		DAQmxErrChk(DAQmxCreateTask(taskName.c_str(),&task))
		channel=dev+"/ai"+toString(int(analogInLines.size()));
		error=DAQmxCreateAIVoltageChan(task,channel.c_str(),"",DAQmx_Val_NRSE,-10,10,DAQmx_Val_Volts,"");
		if (DAQmxFailed(error)){
			DAQmxErrChk(DAQmxClearTask(task))
			break;
		}
		analogInLines.push_back(task);
	}
	channel=dev+"/ao0:"+toString(int(analogOutLines.size()-1));
	DAQmxErrChk(DAQmxCreateTask("analogGen",&analogGen))
	channel=dev+"/port0/line0:"+toString(int(digitalOutLines.size()-1));
	DAQmxErrChk(DAQmxCreateTask("digitalGen",&digitalGen))
	DAQmxErrChk(DAQmxCreateDOChan(digitalGen,channel.c_str(), "", DAQmx_Val_ChanForAllLines))
	DAQmxErrChk(DAQmxCreateTask("multiDigitalOutLines",&multiDigitalOutLines))
	lines.push_back(-1);
	prepareDigitalLines(std::vector<int>(1,0));
	logFile.write(string("NIDAQ ")+dev+" ready",true);
}

NIDAQ::~NIDAQ(){
	CheckExists()
	//clear digital out lines
	for(vector<TaskHandle>::const_iterator d=digitalOutLines.begin();d!=digitalOutLines.end();d++){
		DAQmxErrChk(DAQmxClearTask(*d))
	}
	//clear analog out lines
	for(vector<TaskHandle>::const_iterator ao=analogOutLines.begin();ao!=analogOutLines.end();ao++){
		DAQmxErrChk(DAQmxClearTask(*ao))
	}
	//clear analog in lines
	for(vector<TaskHandle>::const_iterator ai=analogInLines.begin();ai!=analogInLines.end();ai++){
		DAQmxErrChk(DAQmxClearTask(*ai))
	}
	DAQmxErrChk(DAQmxClearTask(analogGen))
	DAQmxErrChk(DAQmxClearTask(digitalGen))
}
