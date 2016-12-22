// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Friday, January 13, 2012</lastedit>
// ===================================
#pragma once
class Chamber{
public:
	bool isX;//is each channel running horizontal (i.e. along the x-axis, true) or vertical (i.e. y-axis, flase), which is given by the direction of stacking of the channels, if channels are stacked along the y-axis then the isX should be true since the flow will be left to right or right to left
	double maxX;//in encoder units X position of the leftmost (largest encoder position) of the first channel of the device (+x of stage will move the objective to the left on the device) 
	double minY;//in encoder units Y position of the bottommost (smallest encoder position) of the first channel of the device (+y of stage will move the objective to the top on the device)
	double channelSpacing;//in mm so we dont have to rely on the stage.....in encoder units for the current XY stage... from center of one channel to center of the next channel
	double channelLength;//in mm so we dont have to rely on the stage......in encoder units for the current XY stage
	double channelWidth;//in mm so we dont have to rely on the stage......in encoder units for the current XY stage
	double channelHeight;//in microns
	int numChannels;
	double minX,maxY,maxZ,focusZ;//max encoder position for this chamber, these values are driven by the other parameters, but will be precalculated for speed
	//int maxX,maxY;//max encoder position for this chamber
	Chamber(double maxX,double minY,double maxZ,double focusZ, double mmChannelSpacing, double mmChannelLength,double mmChannelWidth,double uLChannelVolume,int numChannels,bool isX);
	Chamber();
	double getChannelWidth();//in mm
	double getChannelHeight();//in mm
	double getChannelLength();//in mm
	double getChannelSpacing();//in mm
	double getChannelVolume();//in uL
	double mm2Vol(double mm){return getChannelVolume()*mm/getChannelLength();}//in uL
	double vol2mm(double uL){return getChannelLength()*uL/getChannelVolume();}
	double getVolume(double startFraction, double endFraction){return getChannelVolume()*(endFraction-startFraction);}
	double getCurrentLengthPosition();//use current position to get fractional position of channel, return -1 if outside of the chamber;
	//double getCurrentHeightPosition();
	void move(int channel,double widthFraction=.5,double lengthFraction=.5);//stage must already be inside the chamber region, use a higher level function for stage movements outside this chamber (e.g. Device class move()?)
	bool operator==(const Chamber& other) const;
};