// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include <string>
#include "Utils.h"
#include <vector>
#include "ImageRegion.h"
class AcquisitionParameters{
public:
	//AcquisitionParameters();
	//if asEightBit is true then the data will be scaled resulting in loss of information, but it will only take up half the space
	//if asEightBit is false (default) then the data will be stored as 16bit (not scaled) regardless of the dynamic range of the camera (
	AcquisitionParameters(double exp=.05,int gain=0,int bin=1, ImageRegion imageRegion=ImageRegion(),bool asEightBit=false,std::vector<double>& liteStarts=std::vector<double>(),std::vector<double>& liteEnds=std::vector<double>(),std::vector<double>& camDelays=std::vector<double>());
	AcquisitionParameters(double exp,std::vector<int> gains,int bin=1, ImageRegion imageRegion=ImageRegion(),bool asEightBit=false,std::vector<double>& liteStarts=std::vector<double>(),std::vector<double>& liteEnds=std::vector<double>(),std::vector<double>& camDelays=std::vector<double>());
	
	int bin;//1x1 2x2 3x3 4x4 5x5 etc.
	double exp;//in seconds
	std::vector<double> liteStarts;//when to start exicitation relative to exposure time
	std::vector<double> liteEnds;//when to stop excitation relative to exposure time
	std::vector<double> camDelays;//offset for exposure start time used during frame transfer acquisitions
	//int gain;//linear scale
	std::vector<int> gains;//linear scale
	bool asEightBit;
	ImageRegion imageRegion;
	std::string toString();
	bool operator==(const AcquisitionParameters &a2) const{
		return this->bin==a2.bin && this->gains==a2.gains && this->imageRegion==a2.imageRegion;
	}
	bool operator!=(AcquisitionParameters &right){
		return !(this->operator==(right));
	}
	//gui
	void modify();
	double getStart(int n);
	double getEnd(int n);
	double getCamDelay(int n);
	int getGain(int n=0);
	void setGain(std::vector<int> gains);
	void setGain(int gain);
	std::string getGains();
	//fraction 0-1 representing the amount of overlap between camera exposure times. Cameras must be different 
	//e.g. 1 means start the two acquisitions at the same time 
	//(note: in this case you could have just represented the channel by a single acquisition channel)
	//the default would be 0 where there is no overlap between adjacent acquisitions
	//.25 would mean that after 75% of the first exposure the second exposure will begin
	//we should support negative numbers so that -.25 means there is a delay between the first and second exposures equal to 25% of the first exposure.
	std::vector<double> delays;

private:


};
