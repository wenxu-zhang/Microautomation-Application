// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, May 02, 2011</lastedit>
// ===================================
#pragma once
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;



class ScopeComm{

public:
	HANDLE hCom,hCommAccess;
DWORD maskSent;
DWORD maskRecvd;

ScopeComm(){
	hCom=0;
	hCommAccess=CreateMutex(NULL,false,NULL);
}

~ScopeComm(){
	CloseHandle(hCommAccess);
}

int writeAndRead(string t,char* rx, int numBytes){
	WaitForSingleObject(hCommAccess,INFINITE);
	PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
	DWORD numBytesSent;
	DWORD cmask;
	WriteFile(hCom,t.c_str(),t.length(),&numBytesSent,NULL);
	//WaitCommEvent(hCom,&cmask,NULL);
	DWORD numBytesRead;
	//rx=new char[numBytes+1];
	//WaitCommEvent(hCom,&cmask,NULL);
	int i=0;
	while(i<numBytes){
	ReadFile(hCom,rx+i,1,&numBytesRead,NULL);
		if (*(rx+i)=='\r' || numBytesRead==0) break;
		i++;
	}
	SetEvent(hCommAccess);
	return i;

}

int read(char* rx, int numBytes){
	WaitForSingleObject(hCommAccess,INFINITE);
	DWORD numBytesRead;
	//rx=new char[numBytes+1];
	DWORD cmask;
	//WaitCommEvent(hCom,&cmask,NULL);
	int i=0;
	while(i<numBytes){
	ReadFile(hCom,rx+i,1,&numBytesRead,NULL);
		if (*(rx+i)=='\r' || numBytesRead==0) break;
		i++;
	}
	//SetCommMask(hCom,EV_TXEMPTY | EV_RXCHAR);
	SetEvent(hCommAccess);
	return i;
}

void write(string t){
	WaitForSingleObject(hCommAccess,INFINITE);
	PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
	DWORD numBytes=t.length();
	DWORD numBytesSent;
	DWORD cmask;
	WriteFile(hCom,t.c_str(),numBytes,&numBytesSent,NULL);
	//WaitCommEvent(hCom,&cmask,NULL);
	SetEvent(hCommAccess);
}

int opencom(char *str)
    {
		COMMTIMEOUTS commto;
		DCB dcb;
      // Open Serial Port here
        hCom = CreateFile(str,GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if(hCom==INVALID_HANDLE_VALUE){
			cout<<"Cannot open Com port: "<<str<<endl;
			return 0;
		}
        GetCommState(hCom,&dcb);
        dcb.BaudRate=CBR_9600;//9600;
        dcb.ByteSize=8;
        dcb.Parity=NOPARITY;
		dcb.fParity=FALSE;
		dcb.fOutxCtsFlow=FALSE;
		dcb.fOutxDsrFlow=FALSE;
		dcb.fDtrControl=DTR_CONTROL_ENABLE;
		dcb.fDsrSensitivity=FALSE;
		dcb.fTXContinueOnXoff=TRUE;
		dcb.fOutX=FALSE;
		dcb.fInX=FALSE;
		dcb.fNull=FALSE;
		dcb.fAbortOnError=FALSE;
        dcb.StopBits=ONESTOPBIT;
		dcb.fOutxCtsFlow=TRUE;
        dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;
		if (!SetupComm(hCom,4096,4096)){
			cout<<"Could not setup the serial port"<<endl;
			CloseHandle(hCom);
			return 0;
		}
		if (!SetCommState(hCom, &dcb)){
			cout<<"Could not set comm state"<<endl;
			CloseHandle(hCom);
			return 0;
		}
        commto.ReadIntervalTimeout=0;//time between bytes @19200 BAUD this should be 2.4ms between bytes
        commto.ReadTotalTimeoutMultiplier=200;//time for each byte rcvd
        commto.ReadTotalTimeoutConstant=0;//total time for each read
        commto.WriteTotalTimeoutMultiplier=200;//time for each byte sent
        commto.WriteTotalTimeoutConstant=0;//total time for each write
		if (!SetCommTimeouts(hCom,&commto)){
			cout<<"Could not set comm timeouts"<<endl;
			CloseHandle(hCom);
			return 0;
		}
        SetCommMask(hCom,EV_TXEMPTY);
		maskSent=EV_TXEMPTY;
		maskRecvd=EV_RXCHAR;
        PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
		//write("HPTM96\r");
		//put scope into 19200 baud state
		
		write("HPTM19\r");
		EscapeCommFunction(hCom,CLRRTS);
        CloseHandle(hCom);
		 hCom = CreateFile(str,GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if(hCom==INVALID_HANDLE_VALUE){
			cout<<"Cannot open Com port: "<<str<<endl;
			return 0;
		}
        GetCommState(hCom,&dcb);
        dcb.BaudRate=CBR_19200;//19200;
        dcb.ByteSize=8;
        dcb.Parity=NOPARITY;
		dcb.fParity=FALSE;
		dcb.fOutxCtsFlow=FALSE;
		dcb.fOutxDsrFlow=FALSE;
		dcb.fDtrControl=DTR_CONTROL_ENABLE;
		dcb.fDsrSensitivity=FALSE;
		dcb.fTXContinueOnXoff=TRUE;
		dcb.fOutX=FALSE;
		dcb.fInX=FALSE;
		dcb.fNull=FALSE;
		dcb.fAbortOnError=FALSE;
        dcb.StopBits=ONESTOPBIT;
		dcb.fOutxCtsFlow=TRUE;
        dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;
		if (!SetupComm(hCom,4096,4096)){
			cout<<"Could not setup the serial port"<<endl;
			CloseHandle(hCom);
			return 0;
		}
		if (!SetCommState(hCom, &dcb)){
			cout<<"Could not set comm state"<<endl;
			CloseHandle(hCom);
			return 0;
		}
        commto.ReadIntervalTimeout=30;//time between bytes @19200 BAUD this should be 2.4ms between bytes
        commto.ReadTotalTimeoutMultiplier=0;//time for each byte rcvd
        commto.ReadTotalTimeoutConstant=1000;//total time for each read
        commto.WriteTotalTimeoutMultiplier=10;//time for each byte sent
        commto.WriteTotalTimeoutConstant=0;//total time for each write
		if (!SetCommTimeouts(hCom,&commto)){
			cout<<"Could not set comm timeouts"<<endl;
			CloseHandle(hCom);
			return 0;
		}
		SetCommMask(hCom,EV_TXEMPTY | EV_RXCHAR);
       // SetCommMask(hCom,EV_TXEMPTY);
		maskSent=EV_TXEMPTY;
		maskRecvd=EV_RXCHAR;
        PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
		
		dcb.BaudRate=CBR_19200;
		if (!SetCommState(hCom, &dcb)){
			cout<<"Could not set comm state in 19200 mode"<<endl;
			CloseHandle(hCom);
			return 0;
		}
		write("HPTM19\r");
		//Sleep(100);
		//test scope
		write("FPZFs\r");
		char rx[256];
		DWORD numRead;
		DWORD cmask;
		numRead=read(rx,4);
		//WaitCommEvent(hCom,&cmask,NULL);
		//ReadFile(hCom,rx,4,&numRead,NULL);
		if (numRead!=4) {
			//cout << "numRead=" << numRead << endl;
			return 0;
		}
        return 1;
    }

void closecom()
    {
		//return scope to 9200 baud
		write("HPTM96\r");
        // Close serial port here
		EscapeCommFunction(hCom,CLRRTS);
        CloseHandle(hCom);
    }
};