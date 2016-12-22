#pragma once
#include "Controller.h"
//using namespace control;
 
class ProtocolYing_Ja{
public:
	//Controller Class will call this function 
	//if the user wants to run a protocol
	static void runProtocol();

	//all user defined protocols should be stubbed below here and called by the above function.  It should be defined in a separate .cpp file
private:
	//Testing functions
	static void testProtocols(); //This is used to test modules that I wrote for the system.
	static void testScanSpotsPerimeterFocus();
	static void testScanSpots();
	static void testSpotFocus();
	static void testDFS();

	//Actual experimental protocols or utility modules
	static void denature(Solution *s);
	static void ZFocusVsTemperature(int mode=1);
	static void FocusNTimes(int mode=1);
	static void TemperatureScan();
};
