// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include <string>
/*
This structure defines the part of the detector array to use regardless of detector dimensions or size 
examples: (minX,minY) -> (maxX,maxY)
			(0,0)   -> (1,1)    would be a full frame
			(.5,0)  -> (1,.5)	would be quadrant 1 (i.e. Upper Right)
			(0,0)   -> (.5,.5)  would be quadrant 2 (i.e. Upper Left)
			(0,.5)  -> (.5,1)   would be quadrant 3 (i.e. Lower Left)
			(.5,.5) -> (1,1)	would be quadrant 4 (i.e. Lower Right)
			(.25,.25)->(.75,.75)would be the middle quadrant
*/

class ImageRegion{
public:
	bool operator==(const ImageRegion &right) const {
		return minX==right.minX && minY==right.minY && maxX==right.maxX && maxY==right.maxY;
	}
	bool operator!=(ImageRegion &right){
		return (!(this->operator==(right)));
	}
	ImageRegion(int imageRegion=0);
	ImageRegion(double minX,double minY,double maxX,double maxY, std::string name="Custom"):minX(minX),minY(minY),maxX(maxX),maxY(maxY),name(name){}
	static const int FULL=0;//full image
	static const int Q1=1;//quadrant 1
	static const int Q2=2;//quadrant 2
	static const int Q3=3;//quadrant 3
	static const int Q4=4;//quadrant 4
	static const int CENTER=5;//center quadrant (useful for autofocusing)
	std::string name;
	double minX;//normalized min X pixel number (between 0 and 1)
	double minY;//normalized min Y pixel number
	double maxX;//normalized max X pixel number
	double maxY;//normalized max Y pixel number

	//gui
	static ImageRegion select();
};
