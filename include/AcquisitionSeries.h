// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
//a group of acquisition groups that can be acquired on a single camera with a single acquisistion without having to start a new acquisition
#include <vector>
#include "AcquisitionGroup.h"
class AcquisitionSeries{
public:
	std::vector<AcquisitionGroup*> acquisitionGroups;
	AcquisitionSeries(int cumNumImages=0):numLocalSpotChanges(0),cumNumChans(cumNumImages){}
	bool addAcquisitionGroup(AcquisitionGroup* a);
	~AcquisitionSeries();
	//mapping of camera imageNum to actual image number (i.e. chanNum). 
	//if an acquisition group indicates a new local spot then the chanNum does not start over from zero.  The chanNum keeps increasing through each iteration of that group
	//the size of this vector should indicate the total number of images that must be acquired by the camera (including dummy and pause)
	std::vector<int> _imageNumToChanNum;

	int imageNumToChanNum(int imageNum){
		if (imageNum>=_imageNumToChanNum.size() || imageNum<0) 
			return -1;
		else return _imageNumToChanNum.at(imageNum);//+cumNumChans;
	}
	
	//int numChannels;//num images in this acquisistionSeries not including pauses or dummy images due to delay for mechanical movement (e.g. stage or objective)
	
	//total of all actual images that will be taken previous to this acquisitionSeries. Does not including dummy images (i.e. pauses and mechanical movements)
	//this will help us calculate chanNum
	int cumNumChans;
	
	int numLocalSpotChanges;//number of spot movements specified by all the various acquisition groups.  could be 0 meaning no extra movements for this series
	AcquisitionChannel getChan(int chanNum);
	int numChannels();
	int numImages();
	std::vector<int> chanNumToGroupNum;
};
