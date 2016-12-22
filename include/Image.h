// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "CImg.h"

class Image{
public:
	Image(CImg<unsigned short>* image,Image Properties* ip);
	~Image();
	CImg<unsigned short>* image;
	ImageProperties* ip;
	//we always will save the file save();
	show();
	saveAs(string fileName);//we can save it as something different
}