// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, July 10, 2012</lastedit>
// ===================================
#include "RS232.h"
#include "Record.h"
using namespace std;
extern Record logFile;
RS232::RS232(int comnum,int baud,bool flowControl,int readTimeout,int stopbits):comnum(comnum),baud(baud){
	hCommAccess=CreateMutex(NULL, false, NULL);
	if (opencom(flowControl,readTimeout,stopbits))
		isPresent=true;
	else
		isPresent=false;
}
RS232::~RS232(){
	if (isPresent)
		closecom();
}
void RS232::closecom(void)
    {
		if (isPresent){
	        CloseHandle(hCom);
			isPresent=false;
		}
    }

int RS232::opencom(bool flowControl, int readTimeout,int stopbits)
    {
		string sCom="COM"+toString(comnum);
		hCom = CreateFileA(sCom.c_str(),GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
        if(hCom==INVALID_HANDLE_VALUE){
			logFile.write(string("Could not open com port ")+sCom+". Error: "+getCOMError());
			return 0;
		}
		DCB dcb;
		 GetCommState(hCom,&dcb);
		switch (baud){
		case 110:
			dcb.BaudRate=CBR_110;
			break;
		case 300:
			dcb.BaudRate=CBR_300;
			break;
		case 600:
			dcb.BaudRate=CBR_600;
			break;
		case 1200:
			dcb.BaudRate=CBR_1200;
			break;
		case 2400:
			dcb.BaudRate=CBR_2400;
			break;
		case 4800:
			dcb.BaudRate=CBR_4800;
			break;
		case 9600:
			dcb.BaudRate=CBR_9600;
		case 14400:
			dcb.BaudRate=CBR_14400;
			break;
		case 19200:
			dcb.BaudRate=CBR_19200;
			break;
		case 38400:
			dcb.BaudRate=CBR_38400;
			break;
		case 57600:
			dcb.BaudRate=CBR_57600;
			break;
		case 115200:
			dcb.BaudRate=CBR_115200;
			break;
		case 128000:
			dcb.BaudRate=CBR_128000;
			break;
		case 256000:
			dcb.BaudRate=CBR_256000;
			break;
		default:
			logFile.write("Unknown baud rate. Defaulting to 9600",true);
			dcb.BaudRate=CBR_9600;
			break;
		}
        dcb.ByteSize=8;
        dcb.Parity=NOPARITY;
		switch(stopbits){
			case 1:
				dcb.StopBits=ONESTOPBIT;
				break;
			case 2:
				dcb.StopBits=TWOSTOPBITS;
				break;
			default:
				logFile.write("Unknown stop bits. Defaulting to 1",true);
				dcb.StopBits=ONESTOPBIT;
				break;
		}
		if (flowControl){
	        dcb.fRtsControl=RTS_CONTROL_ENABLE;//This is needed for Power meter, but not for TE modules? OMG bad documentation
			dcb.fDtrControl=DTR_CONTROL_ENABLE;
		}else{
			dcb.fRtsControl=RTS_CONTROL_DISABLE;
			dcb.fDtrControl=DTR_CONTROL_DISABLE;
		}
		SetupComm(hCom,4096,4096);
        SetCommState(hCom, &dcb);
		COMMTIMEOUTS commto;
        commto.ReadIntervalTimeout=readTimeout;
        commto.ReadTotalTimeoutMultiplier=readTimeout;
        commto.ReadTotalTimeoutConstant=readTimeout;
        commto.WriteTotalTimeoutMultiplier=1000;
        commto.WriteTotalTimeoutConstant=1000; 
        SetCommTimeouts(hCom,&commto);
        SetCommMask(hCom,EV_TXEMPTY);
        DWORD cmask=EV_TXEMPTY;
        PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
        return 1;
 }

string RS232::read(int num)
{
	WaitForSingleObject(hCommAccess, INFINITE);
	DWORD dwBytesRead=0;
	Timer t(true);
	bool b=ReadFile(hCom,strbuf,num,&dwBytesRead,NULL);
	double time=t.getTime();
	if (!b) 
		logFile.write("RS232: could not read from com port "+toString(this->comnum)+". Error: "+getCOMError(),true);
	else if (num!=dwBytesRead)
		logFile.write("RS232 Error: failed to read the specified number of bytes",true);
	strbuf[dwBytesRead]='\0';
	string ret(strbuf);
	ReleaseMutex(hCommAccess);
    return ret;
}

int RS232::getNumReadBytes(){
	DWORD errors;
	COMSTAT stat;
	ClearCommError(hCom,&errors,&stat);
	return stat.cbInQue;
}

string RS232::read(string end)
{
	WaitForSingleObject(hCommAccess, INFINITE);
	DWORD dwBytesRead;
	bool b;
	int i=0;
	while(true){
		b=ReadFile(hCom,strbuf+i,1,&dwBytesRead,NULL);
		if (!b) break;
		if (dwBytesRead!=1){
			logFile.write("Error: RS232 failed to read up to end character",true);
			break;
		}
		strbuf[i+1]='\0';
		if (string(strbuf+i+1-end.length())==end){
			//i=i+end.length();dont return end string
			break;
			
		}
		i=i++;
	}
	if (!b)
		logFile.write("RS232: could not read from com port "+toString(this->comnum)+". Error: "+getCOMError(),true);
	strbuf[i]='\0';
	string ret(strbuf);
	ReleaseMutex(hCommAccess);
    return ret;
}

void RS232::write(string s){
	WaitForSingleObject(hCommAccess, INFINITE);
	DWORD nwritten;
	if (!WriteFile(hCom,s.c_str(),s.length(),&nwritten,NULL))
		logFile.write("RS232: could not write to com port "+toString(this->comnum)+". Error: "+getCOMError(),true);
	else if (nwritten!=s.length())
		logFile.write("RS232 error: did not write all characters to com port",true);
	ReleaseMutex(hCommAccess);
}

void RS232::dumpRx(){
	PurgeComm(hCom,PURGE_RXCLEAR);
}