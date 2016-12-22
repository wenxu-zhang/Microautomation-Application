// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, July 10, 2012</lastedit>
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
#include "CUsbCodeVCOM.h"

using namespace std;
extern int LUDLCOM;

// CUsbCode Object Functions

DWORD CUsbCodeVCOM::GetLen(){
	return comm.getNumReadBytes();
}

CUsbCodeVCOM::CUsbCodeVCOM():comm(LUDLCOM,	9600,true,10000,2)	// Constructor
{
	this->DumpRx();
	FirstTimeOpened = 1;	// flag for first time

}

	
// Scans for all the device that match the GUID passed.
// returns an array of cStrings, containing the device names found, 
// and returns the number of device names found. Returns zero if no
// devices found.
int	CUsbCodeVCOM::Scan(string *Devices)	
{
	
	Devices[0]="VIRTUALCOM";
	return 1;
}

// Open the device with the given device name
// Open the device with the given device name
bool CUsbCodeVCOM::Open(string DeviceName)
{
	return comm.isPresent;
}

// Close the device,
int CUsbCodeVCOM::Close()
{	
	comm.closecom();
	return 1;
}

// Write's a string to the usb out port, string must be NULL terminated
// returns number of bytes written
DWORD CUsbCodeVCOM::Write(unsigned char *pText)
{
	string s((char*)pText);
	comm.write(s);
	return s.length();
}

// Write's a string to the usb out port, string length must be given
// returns number of bytes written
DWORD CUsbCodeVCOM::Write(unsigned char *pText, int Length)
{
	if (Length>511) return 0;
	memcpy(msg,pText,Length+1);
	msg[Length]=0;
	return Write((unsigned char *)msg);
}


// Reads data from the USB IN buffer into the memory pointer passed.
// Memory MUST be allocated for at least the size of the request.
// Returns the amount of data read from the IN buffer.
// If there is not enough data in the IN buffer, the readfile() function
// will wait till either; the data becomes available, or the function times out.
// Default timeout is 30 seconds, and can be adjust with the registry or 
// a function call.
DWORD CUsbCodeVCOM::Read(unsigned char * pText, DWORD nBytes)
{
	string s=comm.read(nBytes);
	memcpy(pText,s.c_str(),nBytes);
	pText[nBytes]=0;
	return nBytes;
}

// Dumps all the data in the RX buffer
void CUsbCodeVCOM::DumpRx()
{
	comm.dumpRx();
}

int CUsbCodeVCOM::GetIds(string *Ids)
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

bool CUsbCodeVCOM::IsBusy(int Device)
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
	else
		return(FALSE);
}

bool CUsbCodeVCOM::Goto(int Device, int Position)
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

int CUsbCodeVCOM::ReadStatusByte(int Device)
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

int CUsbCodeVCOM::GetPosition(int Device)
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

void CUsbCodeVCOM::Stop(int Device)
{
	unsigned char pStr[16];
	if(!hUsb) return;
	pStr[0] = Device;	// device #
	pStr[1] = 66;		// stop motor position
	pStr[2] = ':'; 
	Write(pStr,3);		// send the command
}

void CUsbCodeVCOM::SetPosition(int Device, int Position)
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

int CUsbCodeVCOM::GetVelocity(int Device)
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

int CUsbCodeVCOM::GetAccel(int Device)
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

void CUsbCodeVCOM::SetVelocity(int Device, int Speed)
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



void CUsbCodeVCOM::SetAccel(int Device, int Accel)
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


bool CUsbCodeVCOM::IsPresent()
{
	if(hUsb == INVALID_HANDLE_VALUE || !hUsb)	return(0);
	else										return(1);
		
}
