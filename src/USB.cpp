#include "Warn.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <USB.h>

bool debug=false;;
using namespace std;

USB::USB(){
	
	//load DLL functions
	m_hmodule = LoadLibrary("Ftd2xx.dll");	
	if(m_hmodule == NULL)
	{
		cout << "Error: Can't Load Ftd2xx.dll";
		system("pause");
		exit(0);
	}


		m_pWrite = (PtrToWrite)GetProcAddress(m_hmodule, "FT_Write");
		int nErr;
	nErr=GetLastError();                    //test dll of err 6: INVALID_HANDLE
	if (m_pWrite == NULL)
	{
		cout <<"Error: Can't Find FT_Write";
		system("pause");
		exit(0);
	}

	m_pRead = (PtrToRead)GetProcAddress(m_hmodule, "FT_Read");
	if (m_pRead == NULL)
	{
		cout <<"Error: Can't Find FT_Read";
		system("pause");
		exit(0);
	}

	m_pOpen = (PtrToOpen)GetProcAddress(m_hmodule, "FT_Open");
	if (m_pOpen == NULL)
	{
		cout <<"Error: Can't Find FT_Open";
		system("pause");
		exit(0);
	}

	m_pOpenEx = (PtrToOpenEx)GetProcAddress(m_hmodule, "FT_OpenEx");
	if (m_pOpenEx == NULL)
	{
		cout <<"Error: Can't Find FT_OpenEx";
		system("pause");
		exit(0);
	}

	m_pListDevices = (PtrToListDevices)GetProcAddress(m_hmodule, "FT_ListDevices");
	if(m_pListDevices == NULL)
	{
		cout <<"Error: Can't Find FT_ListDevices";
		system("pause");
		exit(0);
	}

	m_pClose = (PtrToClose)GetProcAddress(m_hmodule, "FT_Close");
	if (m_pClose == NULL)
	{
		cout <<"Error: Can't Find FT_Close";
		system("pause");
		exit(0);
	}

	m_pResetDevice = (PtrToResetDevice)GetProcAddress(m_hmodule, "FT_ResetDevice");
	if (m_pResetDevice == NULL)
	{
		cout <<"Error: Can't Find FT_ResetDevice";
		system("pause");
		exit(0);
	}

	m_pPurge = (PtrToPurge)GetProcAddress(m_hmodule, "FT_Purge");
	if (m_pPurge == NULL)
	{
		cout <<"Error: Can't Find FT_Purge";
		system("pause");
		exit(0);
	}

	m_pSetTimeouts = (PtrToSetTimeouts)GetProcAddress(m_hmodule, "FT_SetTimeouts");
	if (m_pSetTimeouts == NULL)
	{
		cout <<"Error: Can't Find FT_SetTimeouts";
		system("pause");
		exit(0);
	}

	m_pGetQueueStatus = (PtrToGetQueueStatus)GetProcAddress(m_hmodule, "FT_GetQueueStatus");
	if (m_pGetQueueStatus == NULL)
	{
		cout <<"Error: Can't Find FT_GetQueueStatus";
		system("pause");
		exit(0);
	}

	m_pSetBaudRate = (PtrToSetBaudRate)GetProcAddress(m_hmodule, "FT_SetBaudRate");
	if (m_pSetBaudRate == NULL)
	{
		cout <<"Error: Can't Find FT_SetBaudRate";
		system("pause");
		exit(0);
	}

	m_pSetLatencyTimer = (PtrToSetLatencyTimer)GetProcAddress(m_hmodule, "FT_SetLatencyTimer");
	if (m_pSetLatencyTimer == NULL)
	{
		cout <<"Error: Can't Find FT_SetLatencyTimer";
		system("pause");
		exit(0);
	}

	m_pSetUSBParameters = (PtrToSetUSBParameters)GetProcAddress(m_hmodule, "FT_SetUSBParameters");
	if (m_pSetUSBParameters == NULL)
	{
		cout <<"Error: Can't Find FT_SetUSBParameters";
		system("pause");
		exit(0);
	}
	//done loading dll functions

}

USB::~USB(){
	Close();
	FreeLibrary(m_hmodule);
}

bool USB::Open(LPCSTR serialNo){
	unsigned char txbuf[25], rxbuf[25];
	FT_STATUS status;
	DWORD ret_bytes;
	m_ftHandle=0;
	if (!m_pOpenEx){
		cout<< "FT_OpenEx is not valid!"; 
		return false;
		system("pause");
		exit(0);
	}
	
	if (!(*m_pOpenEx)((PVOID)serialNo, (DWORD)FT_OPEN_BY_SERIAL_NUMBER, &m_ftHandle)){
		//status=ResetDevice();
		//if (status) cout<<"first reset didn't work"<<endl;
		status=Purge(FT_PURGE_RX || FT_PURGE_TX);
		if (status) cout<<"purge didn't work"<<endl;		
		status=ResetDevice();
		//status=status|ResetDevice();
		//status=status|ResetDevice();
		//status=status|ResetDevice();
		if (status) cout<<"second reset didn't work"<<endl;
		status=Purge(FT_PURGE_RX || FT_PURGE_TX);
		if (status) cout<<"purge didn't work"<<endl;
		
		status=SetLatencyTimer(2);//2ms polling of device receive buffer
		if (status) cout<<"set latency timer didn't work"<<endl;
		status=SetBaudRate(128000);
		if (status) cout<<"Set Baud Rate didn't work"<<endl;
		status=SetUSBParameters(64,0);
		if (status) cout<<"Set USB params didn't work"<<endl;
		status=SetTimeouts(100, 100);//extend timeout while board finishes reset
		if (status) cout<<"Set timeouts didn't work"<<endl;
		Sleep(150);
		Purge(FT_PURGE_RX);//status=Read(rxbuf, 2, &ret_bytes);	
		SetTimeouts(200, 200);
		//test for presence of board
		txbuf[0] = 0xEE;
		status=Write(txbuf, 1, &ret_bytes);
		if (status || ret_bytes<1){
			cout<<"Did not write ONLINE command to USB"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		status=Read(rxbuf, 2, &ret_bytes);
		if (status){
			cout<<"error in USB communications"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		
		if(ret_bytes==1){
			cout<<"what happened here?"<<endl;
			return false;
			Close();
			system("pause");
			exit(0);
		}
		if(ret_bytes==0){
			cout<<"first attempt at connecting to filter wheel failed with return bytes: "<<ret_bytes<<endl;
			status=Read(rxbuf, 2, &ret_bytes);
			if (status || ret_bytes<2){
				cout<<"second attempt failed as well: something is definitely wrong"<<endl;
				Close();
				return false;
				system("pause");
				exit(0);
			}
		}
		if(rxbuf[0] != 0xEE){
			cout<<"the device sent something else back?  buffer not purged? received: "<<hex<<(unsigned int)rxbuf[0]<<dec<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		if(rxbuf[1]!=0x0D){//carriage return
			cout<<"device did not send carriage return...what is going on"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		txbuf[0] = 0xCC;//just a status command to test communications		
		status=Write(txbuf, 1, &ret_bytes);
		if (status || ret_bytes==0) {
			cout<<"USB communcication error. issued command echo"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		status=Read(rxbuf, 12, &ret_bytes);//12 bytes not 14 since we dont have the neutral density option	
		if (status) {
			cout<<"USB communication error. could not read from device"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		if (ret_bytes==1){
			cout<<"how is this possible?"<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		if(ret_bytes==0){//if no response maybe windows was busy... read it again
			cout<<"windows was busy? timeout should not have occurred"<<endl;
			status=Read(rxbuf, 2, &ret_bytes);
			if (status || ret_bytes<2){
				cout<<"could not get response back from USB"<<endl;
				Close();
				return false;
				system("pause");
				exit(0);
			}
		}
		if(rxbuf[0] != 0xCC){			
			cout<<"USB returned wrong echo command..this is not likely: "<<hex<<(unsigned int)rxbuf[0]<<dec<<endl;
			Close();
			return false;
			system("pause");
			exit(0);
		}
		
	}else{//open return something other than 0
		//cout<< "Could Not Open USB"<<endl;
		return false;
		system("pause");
		exit(0);
	}
	return true;
	//SetTimeouts(60000, 60000);//nothing should take longer than 100ms to complete so this is good
}

void USB::Close(){
	if (!m_pClose)
	{
		cout<<"FT_Close is not valid!"; 
		system("pause");
		exit(0);
	}
	
	(*m_pClose)(m_ftHandle);

}

FT_STATUS USB::Read(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytesRead)
{
	if (!m_pRead)
	{
		cout<<"FT_Read is not valid!"; 
		system("pause");
		exit(0);
	}
	FT_STATUS status;
	unsigned long i=0;
	unsigned char* start=(unsigned char*) lpvBuffer;
	while(true){
		status=(*m_pRead)(m_ftHandle, start, dwBuffSize-i, lpdwBytesRead);
		i=i+*lpdwBytesRead;
		if (i==dwBuffSize){
			*lpdwBytesRead=i;
			return status;
		}
		start=start+*lpdwBytesRead;
	}
}	

FT_STATUS USB::Write(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytes)
{
	if (!m_pWrite)
	{
		cout<<"FT_Write is not valid!"; 
		system("pause");
		exit(0)		;
	}
	
	return (*m_pWrite)(m_ftHandle, lpvBuffer, dwBuffSize, lpdwBytes);
}	

FT_STATUS USB::ResetDevice()
{
	if (!m_pResetDevice)
	{
		cout<<"FT_ResetDevice is not valid!"; 
		system("pause");
		exit(0);
	}
	
	return (*m_pResetDevice)(m_ftHandle);
}

FT_STATUS USB::Purge(ULONG dwMask)
{
	if (!m_pPurge)
	{
		cout<<"FT_Purge is not valid!"; 
		system("pause");
		exit(0);
	}

	return (*m_pPurge)(m_ftHandle, dwMask);
}	

FT_STATUS USB::SetTimeouts(ULONG dwReadTimeout, ULONG dwWriteTimeout)
{
	if (!m_pSetTimeouts)
	{
		cout<<"FT_SetTimeouts is not valid!"; 
		system("pause");
		exit(0);
	}

	return (*m_pSetTimeouts)(m_ftHandle, dwReadTimeout, dwWriteTimeout);
}	

FT_STATUS USB::SetBaudRate(ULONG nRate)
{
	if (!m_pSetBaudRate)
	{
		cout<<"FT_SetBaudRate is not valid!"; 
		system("pause");
		exit(0);
	}

	return (*m_pSetBaudRate)(m_ftHandle, nRate);
}

FT_STATUS USB::SetLatencyTimer(UCHAR nTimer)
{
	if (!m_pSetLatencyTimer)
	{
		cout<<"FT_SetLatencyTimer is not valid!"; 
		system("pause");
		exit(0);
	}

	return (*m_pSetLatencyTimer)(m_ftHandle, nTimer);
}

FT_STATUS USB::SetUSBParameters(DWORD nInTransferSize, DWORD nOutTransferSize)
{
	if (!m_pSetUSBParameters)
	{
		cout<<"FT_SetUSBParameters is not valid!"; 
		system("pause");
		exit(0);
	}

	return (*m_pSetUSBParameters)(m_ftHandle, nInTransferSize, nOutTransferSize);
}