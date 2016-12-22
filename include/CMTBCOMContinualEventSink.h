// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
//#include "Controller.h"
#include <atlbase.h>
#include <atlcom.h>
EXTERN_C const CLSID CLSID_MTBCOMContinualEventSink;
#import "MTBApi.tlb" named_guids
using namespace MTBApi;
class ATL_NO_VTABLE CMTBCOMContinualEventSink : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CMTBCOMContinualEventSink, &CLSID_MTBCOMContinualEventSink>, 
//  public IDispatchImpl<IMTBCOMEventSink, &IID_IMTBCOMEventSink, &LIBID_MTBClientUsingCOMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, 
  public IDispEventImpl</*nID*/ 1, CMTBCOMContinualEventSink, &MTBApi::DIID_IMTBContinualEvents, &MTBApi::LIBID_MTBApi, /*wMajor*/ 1, /*wMinor*/ 0> 
{ 
private : 
	HANDLE positionSettled;
public : 
	
/*
BEGIN_COM_MAP(CMTBCOMContinualEventSink) 
   //COM_INTERFACE_ENTRY(IMTBCOMEventSink) 
   COM_INTERFACE_ENTRY(IDispatch) 
END_COM_MAP() 
DECLARE_PROTECT_FINAL_CONSTRUCT() 
HRESULT FinalConstruct() { 
	return S_OK; 
} 
void FinalRelease() 
   { 
   } 
*/
public : 
	CMTBCOMContinualEventSink();
	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBPositionChanged(short newPosition);

	// Event handler for event: MTBPositionChangedEvent 
	void __stdcall OnMTBPositionSettled(short newPosition);
	void wait();


BEGIN_SINK_MAP(CMTBCOMContinualEventSink) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBContinualEvents, /*dispid =*/ 0x1, OnMTBPositionChanged) 
	SINK_ENTRY_EX( 1, MTBApi::DIID_IMTBContinualEvents, /*dispid =*/ 0x3, OnMTBPositionSettled) 
END_SINK_MAP()   
}; 