// FileName: CUsbCode.h	
// Ludl Electronic Products - Copyright 2002
// Defines the CUsbCode Class

//typedef unsigned char uchar;         
#include <Windows.h>
class CUsbCode
{
	char	msg[512];
	HANDLE	hUsb;
	int		FirstTimeOpened;
public:
	bool IsPresent();
	void SetAccel(int Device, int Accel);
	void SetVelocity(int Device, int Speed);
	int GetAccel(int Device);
	int GetVelocity(int Device);
	void SetPosition(int Device, int Position);
	void Stop(int Device);
	int GetPosition(int Device);
	int ReadStatusByte(int Device);
	bool Goto(int Device, int Position);
	bool IsBusy(int Device);
	CUsbCode();
	void	DumpRx();
	int	GetIds(std::string *Ids);
	int	Scan(std::string *Devices);
	bool Open(std::string DeviceName);
	int		Close();
	DWORD	Write(unsigned char *);
	DWORD	Write(unsigned char *, int);
	DWORD	Read(unsigned char * pText, DWORD nBytes);
	DWORD	GetLen();
};
 



//  - - - - - - - e o f - - - - - - - - //