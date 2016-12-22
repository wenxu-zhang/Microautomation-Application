// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "CMTBCOMContinualEventSink.h"
#include <iostream>

CMTBCOMContinualEventSink::CMTBCOMContinualEventSink(){
		positionSettled=CreateEvent(NULL,false,false,NULL);
	}

void __stdcall CMTBCOMContinualEventSink::OnMTBPositionChanged(short newPosition) 
	{ 
		std::cout<<"Position changed"<<std::endl;
	} 

void __stdcall CMTBCOMContinualEventSink::OnMTBPositionSettled(short newPosition) 
	{ 
		SetEvent(positionSettled);
	} 
	void CMTBCOMContinualEventSink::wait(){
		WaitForSingleObject(positionSettled,INFINITE);
	}