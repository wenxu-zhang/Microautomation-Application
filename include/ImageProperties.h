// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Utils.h"
#include "Controller.h"

class ImageProperties{
public:
	ImageProperties(){}
	ImageProperties(AcquisitionChannel ac, std::string name,std::string comment,int x,int y,double z,double temp,double temp2,std::string time, unsigned int max=0);
	void Update(AcquisitionChannel ac, std::string name,std::string comment,int x,int y,double z,double temp,double temp2,std::string time,unsigned int max=0);
	AcquisitionChannel ac;//do not modify this channel information
	int x;//x position of image (from linear encoder)
	int y;//y position of image (from linear encoder)
	double z;//z focus position of image (from microscope)
	double temp;//temperature during capture
	double temp2;//temperature during capture
	std::string time;//approximate time stamp of acquisition  YYYY:MM:DD HH:MM:SS
	std::string comment;//optional comment
	std::string name;//file name with directory (relative to workingDir) but without extension (extension is left to the desired file format of the camera class usually tiff)
	unsigned int max;//max pixel value
	//bool showScaleBar;
	//bool showOnScreen;
	//handled by camera    unsigned short max;//max intensity value
	//handled by camera    unsigned short min;//min intensity value
	//handled by camera    int bitsPerPixel;//Andor885 is a 14bit camera Hamamatsu BT would be 16bit
};