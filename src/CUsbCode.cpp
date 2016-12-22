// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, February 17, 2011</lastedit>
// ===================================
//	Filename:	CUsbCode.cpp
//	Birth:		7/16/99
//	Author:		Doug Lovett
//	Ludl Electronic Products Copyright 2000
//	

// Includes
#include "Warn.h"
#include "Definitions.h"
#include <string>
#include <iostream>
#include <objbase.h>
#include <setupapi.h>		// You must Manually add the setupapi.lib to the project library link tab!
#include "devioctl.h"		// This header file is proved by MS, with the DDK
#include "Utils.h"
#include <initguid.h>
#include "Controller.h"
#include "CUsbCode.h"

using namespace std;

extern Controller cont;

// CTL_CODE defines
#define LEPUSB_IOCTL_INDEX         0x0000
#define LEPUSB_IOCTL_VENDOR_INDEX  0x0800


#define IOCTL_LEPUSB_GET_CONFIG_DESCRIPTOR     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   LEPUSB_IOCTL_INDEX,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_LEPUSB_RESET_DEVICE       CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   LEPUSB_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_LEPUSB_RESET_PIPE         CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   LEPUSB_IOCTL_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)


#define IOCTL_LEPUSB_TIME_OUT           CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   LEPUSB_IOCTL_VENDOR_INDEX,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_LEPUSB_GET_READ_LENGTH    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
												   LEPUSB_IOCTL_VENDOR_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_LEPUSB_GET_VERSION_INFO    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
												   LEPUSB_IOCTL_VENDOR_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)


// LEP USB Device GUID => {4D48F140-44E2-11d3-9D64-00E0291DEE58}
DEFINE_GUID(LEP_GUID, 
0x4d48f140, 0x44e2, 0x11d3, 0x9d, 0x64, 0x0, 0xe0, 0x29, 0x1d, 0xee, 0x58);




// CUsbCode Object Functions

CUsbCode::CUsbCode()		// Constructor
{

	FirstTimeOpened = 1;	// flag for first time

}

	
// Scans for all the device that match the GUID passed.
// returns an array of cStrings, containing the device names found, 
// and returns the number of device names found. Returns zero if no
// devices found.
int	CUsbCode::Scan(string *Devices)	
{
	
	// Get handle to the devices manager
	HDEVINFO  hInfo = SetupDiGetClassDevs((LPGUID)&LEP_GUID, NULL, NULL,
	                                     DIGCF_PRESENT |  DIGCF_INTERFACEDEVICE);

    if ((hInfo == INVALID_HANDLE_VALUE) || (hInfo==0))		// check for error
		return(0);
	

	for (DWORD i=0; ; ++i)					// loop till all devices are found
		{
		SP_INTERFACE_DEVICE_DATA Interface_Info;			// declare structure
		Interface_Info.cbSize = sizeof(Interface_Info);		// Enumerate device
		if (!SetupDiEnumInterfaceDevice(hInfo, NULL, (LPGUID) &LEP_GUID,i, &Interface_Info))
			{
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);
			}
	
		DWORD needed;										// get the required lenght
		SetupDiGetInterfaceDeviceDetail(hInfo, &Interface_Info, NULL, 0, &needed, NULL);
		PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(needed);
		if (!detail)
			{  
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);
			}
															// fill the device details
		detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		if (!SetupDiGetInterfaceDeviceDetail(hInfo, &Interface_Info, detail, needed,NULL, NULL))
			{
			free((PVOID) detail);
			SetupDiDestroyDeviceInfoList(hInfo);
			return(i);
			}

		char name[MAX_PATH];
		strncpy(name, detail->DevicePath, sizeof(name));	// copy the device name
		free((PVOID) detail);								// free the mem
		Devices[i] = name;									// save device name
			
	}  // end of for loop
}

// Open the device with the given device name
// Open the device with the given device name
bool CUsbCode::Open(string DeviceName)
{
	// Create the handle to our device
	hUsb = CreateFile(DeviceName.c_str(),
			GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if( hUsb == INVALID_HANDLE_VALUE || !hUsb)	// process error 
		{
		hUsb = 0;
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL );
		// Display the string.
		logFile.write("Ludl: Failed to open Device",true);		
		LocalFree( lpMsgBuf );  // Free the buffer.
		clickAbort(true);
		return(FALSE);
		}
	
	return(TRUE);		// returns True on success
		
}

// Close the device,
int CUsbCode::Close()
{	

	if( hUsb )	{
		if( CloseHandle( hUsb) )			// close the handle to our device
			{
			hUsb=0;
			return(1);
			}
		else
			{
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf, 0, NULL );
			// Display the string.
			logFile.write("Ludl: Failed to close Device",true);		
			LocalFree( lpMsgBuf );  // Free the buffer.
			clickAbort(true);
			hUsb=0;
			}
		}
	return 0;
}

// Write's a string to the usb out port, string must be NULL terminated
// returns number of bytes written
DWORD CUsbCode::Write(unsigned char *pText)
{
	DWORD nBytesWritten;	

	if(hUsb == INVALID_HANDLE_VALUE || !hUsb)		// check for valid handle
		{
			logFile.write("Ludl: USB OUT Device closed. Failed to write to Device",true);		
			clickAbort(true);
			return(0);
		}

	if(WriteFile(hUsb, pText, strlen((char*)pText), &nBytesWritten, NULL))		// write the string
		return nBytesWritten;
	else
		{		// process errors
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL );
		// Display the string.
		Close();
		logFile.write("Ludl: Closing Device! Failed to write Device",true);		
		clickAbort(true);
		LocalFree( lpMsgBuf );  // Free the buffer.
		return(nBytesWritten);
		}
}

// Write's a string to the usb out port, string length must be given
// returns number of bytes written
DWORD CUsbCode::Write(unsigned char *pText, int Length)
{
	DWORD nBytesWritten;	
	DumpRx();
	if(hUsb == INVALID_HANDLE_VALUE || !hUsb)		// check for valid handle
		{
			logFile.write("Ludl: USB OUT Device closed. Failed to write to Device",true);		
			clickAbort(true);
			return(0);
		}

	if(!Length) return(0);

	if(WriteFile(hUsb, pText, Length, &nBytesWritten, NULL))		{// write the string
		if ((int)nBytesWritten!=Length)
			logFile.write("Ludl failed to write the specified number of bytes",true);
		return nBytesWritten;
	}
	else
		{		// process errors
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL );
		// Display the string.
		Close();
		logFile.write("Ludl: Closing Device! Failed to write Device",MB_ICONHAND);		
		LocalFree( lpMsgBuf );  // Free the buffer.
		clickAbort();
		return(nBytesWritten);
		}
}


// Reads data from the USB IN buffer into the memory pointer passed.
// Memory MUST be allocated for at least the size of the request.
// Returns the amount of data read from the IN buffer.
// If there is not enough data in the IN buffer, the readfile() function
// will wait till either; the data becomes available, or the function times out.
// Default timeout is 30 seconds, and can be adjust with the registry or 
// a function call.
DWORD CUsbCode::Read(unsigned char * pText, DWORD nBytes)
{
	DWORD nBytesRead;

	if(hUsb == INVALID_HANDLE_VALUE || !hUsb)
		{
			logFile.write("Ludl: USB In Device closed. Failed to read to Device",true);
			clickAbort();
			return 0;
		}
	Timer t(true);
	if(ReadFile(hUsb, pText, nBytes, &nBytesRead, NULL)){
		double time=t.getTime();
		if (time>1000.0){
			logFile.write(string("Ludl USB took ")+::toString(time/1000.0)+" sec to read "+::toString((int)nBytes)+" bytes",true);
			ReadStatusByte(1);
			ReadStatusByte(2);

		}
		if ((int)nBytesRead!=nBytes)
			logFile.write("Ludl failed to read the specified number of bytes",true);
		return nBytesRead;
	}
	else
		{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL );
		// Display the string.
		Close();
		logFile.write("Ludl: Closing Device! Failed to read Device",MB_ICONHAND);		
		LocalFree( lpMsgBuf );  // Free the buffer.
		clickAbort(true);
		return(nBytesRead);
		}	
	return 0;
}

// GetLen - This function returns the length of data available in the
// USB IN buffer.
DWORD CUsbCode::GetLen()
{
	struct _ReadLenStruct{
    ULONG   DataLength;
    ULONG   FreeLength;
    ULONG   LFLength;
    ULONG   UserLength;
    }		ReadLen;
	unsigned long nBytes;	
	BOOLEAN success;	
		
	if(!hUsb) return(0);

    success = DeviceIoControl(hUsb,			// call the get length function
              IOCTL_LEPUSB_GET_READ_LENGTH,
              NULL, 0, &ReadLen, sizeof(ReadLen),
              &nBytes,
              NULL);
	if( nBytes != sizeof(ReadLen) || !success )	// check of errors
		{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL );
		// Display the string.
		Close();
		logFile.write("LudL: Closing Device! Failed to read device length!",MB_ICONHAND);		
		LocalFree( lpMsgBuf );  // Free the buffer.
		clickAbort(true);
		return(0);
		}
	else
		return(ReadLen.DataLength);		// return the available data length
      
}


// Dumps all the data in the RX buffer
void CUsbCode::DumpRx()
{
	//Timer::wait(2000);
	unsigned char*	pStr;
	int		size;
	if(!hUsb) return;
	if( size=GetLen() )
	{
		pStr = new unsigned char[size];
		Read(pStr,size);
		delete(pStr);
	}

}

int CUsbCode::GetIds(string *Ids)
{
	string	name;
	char	test,buffer[25];
	unsigned char	pStr[1024];
	int		i,cnt=0;

	if(!hUsb) return(0);

	Ids[0] = "0 OFF";
	for(i=1; i<=20; i++)
	{	// get device name

		pStr[0] = i;		// device #
		pStr[1] = 105;		// get info
		pStr[2] = 6;
		pStr[3] = ':'; 
		Write(pStr,4);		// send the get Id sting
		Read(pStr, 6);
		if(pStr[0] == '0' )		// if date/ver returned resend command
		{
			pStr[0] = i;		// device #
			pStr[1] = 105;		// get info
			pStr[2] = 6;
			pStr[3] = ':'; 
			Write(pStr,4);		// send the get Id sting
			Read(pStr, 6);
		}
		pStr[5] = 0;		// add the terminator
		test = pStr[0];
		itoa(i,buffer,10);
		name = buffer;
		name += " " ;
		name += (char*) pStr;
		name += " " ;
		// get version
		pStr[0] = i;		// device #
		pStr[1] = 105;		// get version
		pStr[2] = 6;
		pStr[3] = ':'; 
		Write(pStr,4);		// send the get Id sting
		Read(pStr, 6);
		pStr[5] = 0;		// add the terminator
		sprintf(buffer,"V%G",((double)pStr[4])/10.0);
		name += buffer;
		
		if( test != 'X' )	// only copy device id if it's there.
			Ids[++cnt] = name;
	}
	return(cnt);
}

bool CUsbCode::IsBusy(int Device)
// Returns ture if device is busy
{	
	unsigned char pStr[16];
	if(!hUsb) return(1);
	pStr[0] = Device;	// device #
	pStr[1] = '?';		// busy command
	pStr[2] = ':'; 
	Write(pStr,3);		// send the command
	Read(pStr, 1);
	if(pStr[0] == 'B')
		return(TRUE);
	else if (pStr[0] == 'b')
		return(FALSE);
	else{
		logFile.write("Ludl Error: device "+toString(Device)+" could not determine if device was busy. Return was"+toString(pStr[0]),true);
		return(TRUE);
	}
		
}

bool CUsbCode::Goto(int Device, int Position)
// Move device to position
{	

	unsigned char pStr[16];
	if(!hUsb) return(0);
	pStr[0] = Device;	// device #
	pStr[1] = 84;		// load target postion
	pStr[2] = 3;
	pStr[3] = (0x0000FF&Position)>>0;		
	pStr[4] = (0x00FF00&Position)>>8;		
	pStr[5] = (0xFF0000&Position)>>16;		
	pStr[6] = ':'; 
	Write(pStr,7);		// send the command
	pStr[0] = Device;	// device #
	pStr[1] = 71;		// go command
	pStr[2] = ':'; 
	Write(pStr,3);		// send the command
	return(TRUE);
}

int CUsbCode::ReadStatusByte(int Device)
// returns the Status byte
{
	unsigned char pStr[16];
	if(!hUsb) return(0);
	pStr[0] = Device;	// device #
	pStr[1] = 126;		// status byte command
	pStr[2] = 1;	
	pStr[3] = ':'; 
	Write(pStr,4);		// send the command
	Read(pStr, 1);
	unsigned int result=(unsigned int) pStr[0];
	unsigned int out=result&0x00000001;
	cout<<"motor running?: "<<out<<endl;
	out=result&0x00000002>>1;
	cout<<"servo ON?: "<<out<<endl;
	out=result&0x00000004>>2;
	cout<<"motor phases are ON?: "<<out<<endl;
	out=result&0x00000008>>3;
	cout<<"joystick ON?: "<<out<<endl;
	out=result&0x00000010>>4;
	cout<<"motor ramping?: "<<out<<endl;
	out=result&0x00000020>>5;
	cout<<"motor ramping UP?: "<<out<<endl;
	out=result&0x00000040>>6;
	cout<<"CW end limit switch?: "<<out<<endl;
	out=result&0x00000080>>7;
	cout<<"CCW end limit switch?: "<<out<<endl;
	pStr[0] = Device;	// device #
	pStr[1] = 128;		// status byte command
	pStr[2] = 1;	
	pStr[3] = ':'; 
	Write(pStr,4);		// send the command
	Read(pStr, 1);
	result=(int) pStr[0];
	out=result&0x00000001;
	cout<<"motor stalled?: "<<out<<endl;
	out=result&0x00000002>>1;
	cout<<"servo aligning at idle?: "<<out<<endl;
	return((int)pStr[0]);
}

int CUsbCode::GetPosition(int Device)
// returns the current position of the axis
{
	int	 pos=0;
	unsigned char pStr[16];
	if(!hUsb) return(0);
	pStr[0] = Device;	// device #
	pStr[1] = 97;		// read motor position
	pStr[2] = 3;	
	pStr[3] = ':'; 
	Write(pStr,4);		// send the command
	Read(pStr, 3);
	pos =  0x000000FF&((unsigned int)pStr[0]);
	pos |= 0x0000FF00&((unsigned int)pStr[1]<<8);
	pos |= 0x00FF0000&((unsigned int)pStr[2]<<16);
	if(pos & 0x00800000) pos |= 0xFF000000;	// make it negative
	return(pos);
}

void CUsbCode::Stop(int Device)
{
	unsigned char pStr[16];
	if(!hUsb) return;
	pStr[0] = Device;	// device #
	pStr[1] = 66;		// stop motor position
	pStr[2] = ':'; 
	Write(pStr,3);		// send the command
}

void CUsbCode::SetPosition(int Device, int Position)
{
	unsigned char pStr[16];
	if(!hUsb) return;
	pStr[0] = Device;	// device #
	pStr[1] = 65;		// set motor positionload target postion
	pStr[2] = 3;
	pStr[3] = (0x0000FF&Position)>>0;		
	pStr[4] = (0x00FF00&Position)>>8;		
	pStr[5] = (0xFF0000&Position)>>16;		
	pStr[6] = ':'; 
	Write(pStr,7);		// send the command

}

int CUsbCode::GetVelocity(int Device)
{
	int	 speed;
	unsigned char pStr[16];
	if(!hUsb) return(0);
	pStr[0] = Device;	// device #
	pStr[1] = 115;		// get speed
	pStr[2] = 2;	
	pStr[3] = ':'; 
	Write(pStr,4);		// send the command
	Read(pStr, 2);
	speed =  0x000000FF&((unsigned int)pStr[0]);
	speed |= 0x0000FF00&((unsigned int)pStr[1]<<8);
	return( 5529600/(65536-speed));
}

int CUsbCode::GetAccel(int Device)
{
	unsigned char pStr[16];
	if(!hUsb) return(0);
	pStr[0] = Device;	// device #
	pStr[1] = 113;		// get accel
	pStr[2] = 1;	
	pStr[3] = ':'; 
	Write(pStr,4);		// send the command
	Read(pStr, 1);
	return( 0x000000FF&(unsigned int)pStr[0]);
}

void CUsbCode::SetVelocity(int Device, int Speed)
{	
	unsigned char pStr[16];
	if(!hUsb) return;
	Speed = 65536-(5529600/Speed);
	pStr[0] = Device;	// device #
	pStr[1] = 83;		// set motor positionload target postion
	pStr[2] = 2;
	pStr[3] = (0x0000FF&Speed)>>0;		
	pStr[4] = (0x00FF00&Speed)>>8;		
	pStr[5] = ':'; 
	Write(pStr,6);		// send the command

}



void CUsbCode::SetAccel(int Device, int Accel)
{
	unsigned char pStr[16];
	if(!hUsb) return;
	pStr[0] = Device;	// device #
	pStr[1] = 81;		// set accel
	pStr[2] = 1;
	pStr[3] = (unsigned char)Accel;
	pStr[4] = ':'; 
	Write(pStr,5);		// send the command
	
}


bool CUsbCode::IsPresent()
{
	if(hUsb == INVALID_HANDLE_VALUE || !hUsb)	return(0);
	else										return(1);
		
}
