// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Sunday, February 27, 2011</created>
// <lastedit>Thursday, June 16, 2011</lastedit>
// ===================================
//All major class definitions will go here
#pragma once
#define NOMINMAX

//#define AXIOVERT
#define OBSERVER

//window sizes
#define CONSOLEWIDTH 0.5 //fraction of total screen width used for console window
#define CONSOLEHEIGHT 0.5//fraction of total screen height used for console window

#define ALLOWMISSINGCOMPONENTS

#ifdef ALLOWMISSINGCOMPONENTS
#define CheckExists(retVal) if (!isPresent) {logFile.write("Attempt to use device that is not present"); return retVal;}
#else
#define CheckExists(retVal) //don't perform check the program will not execute
#endif

#define CONSOLETITLE string("Microautomation Application")

#ifdef AXIOVERT
#define DEFAULTWORKINGDIR "D:\\Program_Working_Directory\\"
#endif
#ifdef OBSERVER
#define DEFAULTWORKINGDIR "E:\\Program_Working_Directory\\"

#endif

//AxioObserver
#define DEBUGDEFINITEFOCUS true
#define DEFINITEFOCUSTIMEOUT 10000//timeout in ms for definite focus initialization and focusing

//Controller
#define DEBUGCONTROLLER true
#define DEBUGLASER true
#define DEBUGDAQPCI6733 true
#define DEBUGDAQUSB6251 true

//Pump
#define DEBUGPUMP true

//Setup specific defaults
#ifdef OBSERVER
#endif
#ifdef AXIOVERT
#define PUMPCOM 3
#define SYRINGE 1-1
#define VALVE1 6-1//?
#define VALVE2 8-1
#define REAGENTTUBING 150//150uL is default tubing volume from reagent solution to valve
#define WASHTUBING 314//314uL is default tubing volume from wash solution to valve
#define DAISYVALVE 9 //valve position on valve 1 that connects to valve 2 common port
#define DAISYTUBING 50//tubing volume in microliters between valve 1 position 9 and valve 2 common port
#define SYRINGE2CHAMBERTUBING 485//tubing volume between syringe pump and chamber exit?
#define VALVE2CHAMBERTUBING 562//tubing volume betweeen valve1 common port and chamber entrance
#endif

//XYStage
//#define XYLIMITS
#define LIMITCUSHION 500.0 //extra space around each chamber where the limits are defined (in microns)
#define STAGECHAMBEROFFSET 70.0 //70mm between chambers
#define STAGECHAMBERWIDTH  14.0 //25mm width but need to be careful with 100x
#define STAGECHAMBERHEIGHT 25.0 //40mm height but need to be careful with the 100x
#define STAGECHANNELOFFSET 6.0 //7mm between channels
#define XOFFSET 0//945878//948456//954234//counter steps from XlimitSwitch to center of left chamber
#define YOFFSET 0//512295//counter steps from YlimitSwitch to center of left chamber
#define ACCELERATION 1 //1 is max 10 is default
#define MAXVELOCITY 200000 //max velocity is 2764800 but that would will stall the motors.  this seems to be as fast as we can go
#define OILVELOCITY  50000 //velocity to move when using an oil objective so the oil follows the objective
#define JOYSTICKSLOW 80000
#define JOYSTICKFAST 200000
//#define JOYSTICKDEFLECTION 120 //MAC6000 only
//#define JOYSTICKACCELERATION 1000000  //MAC6000 only
#define HOMINGVELOCITYCOARSE 100000  
#define HOMINGVELOCITYFINE 50000
#define MOTORRESOLUTION 20000 //steps per revolution ..default is 10000 
#define STAGETIMEOUT 20000//in milliseconds

//Camera
#define DEBUGANDORCAM false
#define DEBUGANDORFOCUS false
#define SAVEFILETIMEOUT 10000000 //wait 30 seconds for each image to be spooled to disk
#define ANDORCAMTEMPDIR std::string(std::string(DEFAULTWORKINGDIR)+"AndorCamTemp\\")
#define OVEREXPOSED 0.9  //images with maximum intensity above 90% of theoretical maximum will be considered dangerous to the camera
#define UNDEREXPOSED 0.25 //images with maximum intensity only 25% of theoretical maximum will be considered underexposed

//Scan
#define DEBUGSCAN false

//Focus
#define DEBUGFOCUS false
#define COARSERANGE 500		//500uM search range
#define COARSEDOFMULTIPLES 4    //.5 would be equal to fine focusing.  This is used for coarse focusing step size

//Record
#define LINEWIDTH 100 //width of each line of text in recording file
#define DEBUGRECORD false

//DG
#define DGTIMEOUT 1000 //ms timeout for dg communication

//DAQ board
#define DEBUGNIDAQ true

//TEModules
#define TE36DERGAIN 0//.23);
#define TE36INTGAIN .5//2.3);
#define	TE36PROPBAND 6//6);  this is zero to 100% not -100% to 100% like in the program
#define TE36HEATMULT 1.0
#define TE36COOLMULT 1.0

#define TE24DERGAIN .05//.23);
#define TE24INTGAIN .5//2.3);
#define	TE24PROPBAND 10//6);  this is zero to 100% not -100% to 100% like in the program
#define TE24HEATMULT 1.0