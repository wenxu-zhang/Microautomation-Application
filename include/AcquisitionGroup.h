// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
//a group of channels that can be taken in quick succession without the need for a dummy acquisition
#include <vector>
#include "AcquisitionChannel.h"
namespace NIDAQmx{
#include "NIDAQmx.h"
}
class LightSource;
#include <set>
class AcquisitionGroup{
public:
	std::vector<AcquisitionChannel> acquisitionChannels;
	AcquisitionGroup(int cumNumChans=0,int numLocalSpotChanges=0);
	~AcquisitionGroup();
	bool addAcquisitionChannel(AcquisitionChannel a);
	std::set<LightSource*> lights;
	static AcquisitionGroup getAcquisitionGroup();
	std::string pChannels;
	NIDAQmx::int32 sampsPerChan;
	NIDAQmx::float64* analogWaveform;	
	NIDAQmx::uInt8* digitalWaveform;
	int index;
	std::vector<int> conversion;
	int numLocalSpotChanges;//this group will be require this many move2NextSpotLocal() calls during acquisition. default will be 0 meaning acquire without calling move2NextLocalSpot(). 1 means first call move2NextSpotLocal() then acquire acquisition Group, 2 means first call move2NextSpotLocal() then acquire Acquisition Group then call move2NextSpotLocal() again and then acquire Acquisition Group one more time.  etc.
	int numChans();
	int cumNumChans;
	AcquisitionChannel getChan(int chanNum);
	std::vector<int> chanNumToImageNum;
	std::vector<int> imageNumToChanNum;
};
