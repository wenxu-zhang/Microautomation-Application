#include <windows.h>
#include "ftd2xx.h"
class USB{
public:
	USB();//load DLL
	~USB();
	bool Open(LPCSTR serialNo);//open USB device by serial number
	void Close();//close USB connection
 	FT_STATUS Read(LPVOID, DWORD, LPDWORD);//read from device
	FT_STATUS Write(LPVOID, DWORD, LPDWORD);//write to device
	
	//these functions were not needed...YET!!
	//FT_STATUS GetQueueStatus(LPDWORD);
	//FT_STATUS ListDevices(PVOID, PVOID, DWORD);
	FT_STATUS Purge(ULONG);

private:
	FT_STATUS ResetDevice();
	
	FT_STATUS SetTimeouts(ULONG, ULONG);
	FT_STATUS SetBaudRate(ULONG);
	FT_STATUS SetLatencyTimer(UCHAR);
	FT_STATUS SetUSBParameters(DWORD, DWORD);
	HMODULE m_hmodule;
	FT_HANDLE m_ftHandle;
	typedef FT_STATUS (WINAPI *PtrToOpen)(PVOID, FT_HANDLE *); 
	PtrToOpen m_pOpen; 
	typedef FT_STATUS (WINAPI *PtrToOpenEx)(PVOID, DWORD, FT_HANDLE *); 
	PtrToOpenEx m_pOpenEx;
	typedef FT_STATUS (WINAPI *PtrToListDevices)(PVOID, PVOID, DWORD);
	PtrToListDevices m_pListDevices; 
	typedef FT_STATUS (WINAPI *PtrToClose)(FT_HANDLE);
	PtrToClose m_pClose;
	typedef FT_STATUS (WINAPI *PtrToRead)(FT_HANDLE, LPVOID, DWORD, LPDWORD);
	PtrToRead m_pRead;
	typedef FT_STATUS (WINAPI *PtrToWrite)(FT_HANDLE, LPVOID, DWORD, LPDWORD);
	PtrToWrite m_pWrite;
	typedef FT_STATUS (WINAPI *PtrToResetDevice)(FT_HANDLE);
	PtrToResetDevice m_pResetDevice;
	typedef FT_STATUS (WINAPI *PtrToPurge)(FT_HANDLE, ULONG);
	PtrToPurge m_pPurge;
	typedef FT_STATUS (WINAPI *PtrToSetTimeouts)(FT_HANDLE, ULONG, ULONG);
	PtrToSetTimeouts m_pSetTimeouts;
	typedef FT_STATUS (WINAPI *PtrToGetQueueStatus)(FT_HANDLE, LPDWORD);
	PtrToGetQueueStatus m_pGetQueueStatus;
	typedef FT_STATUS (WINAPI *PtrToSetBaudRate)(FT_HANDLE, ULONG);
	PtrToSetBaudRate m_pSetBaudRate;
	typedef FT_STATUS (WINAPI *PtrToSetLatencyTimer)(FT_HANDLE, UCHAR);
	PtrToSetLatencyTimer m_pSetLatencyTimer;
	typedef FT_STATUS (WINAPI *PtrToSetUSBParameters)(FT_HANDLE, DWORD, DWORD);
	PtrToSetUSBParameters m_pSetUSBParameters;

};