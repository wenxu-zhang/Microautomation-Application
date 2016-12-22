// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "AcquisitionSeries.h"
#include "Camera.h"
#include <functional>
#include <iterator>
#include <algorithm>
using namespace std;
bool AcquisitionSeries::addAcquisitionGroup(AcquisitionGroup* a){
	if(acquisitionGroups.empty() ||
		(a->acquisitionChannels.back().out->cam==acquisitionGroups.back()->acquisitionChannels.back().out->cam &&
		a->acquisitionChannels.back().ap.imageRegion==acquisitionGroups.back()->acquisitionChannels.back().ap.imageRegion &&
		a->acquisitionChannels.back().ap.getGain()==acquisitionGroups.back()->acquisitionChannels.back().ap.getGain() &&
		a->acquisitionChannels.back().ap.bin==acquisitionGroups.back()->acquisitionChannels.back().ap.bin))
	{
			if (!acquisitionGroups.empty())
				_imageNumToChanNum.push_back(-1);//dummy image between groups
			vector<int> imageNumToChanNum(a->imageNumToChanNum);
			_imageNumToChanNum.insert(_imageNumToChanNum.end(), imageNumToChanNum.begin(), imageNumToChanNum.end() );
			for(int i=1;i<std::max(a->numLocalSpotChanges,1);i++){
				transform(imageNumToChanNum.begin(),imageNumToChanNum.end(),imageNumToChanNum.begin(),bind2nd(plus<int>(),imageNumToChanNum.size()));
				_imageNumToChanNum.push_back(-1);
				_imageNumToChanNum.insert(_imageNumToChanNum.end(), imageNumToChanNum.begin(), imageNumToChanNum.end() );
			}
			numLocalSpotChanges+=a->numLocalSpotChanges;
			std::vector<int> v(a->numChans(),acquisitionGroups.size());
			chanNumToGroupNum.insert(chanNumToGroupNum.end(),v.begin(),v.end());
			acquisitionGroups.push_back(a);
			return true;
	}else return false;
}

AcquisitionSeries::~AcquisitionSeries(){
	for(std::vector<AcquisitionGroup*>::iterator i=acquisitionGroups.begin();i!=acquisitionGroups.end();i++){
		delete *i;
	}
}

int AcquisitionSeries::numChannels(){
	return chanNumToGroupNum.size();
}

int AcquisitionSeries::numImages(){
	return _imageNumToChanNum.size();
}

AcquisitionChannel AcquisitionSeries::getChan(int chanNum){
	//chanNum=chanNum-cumNumChans;
	if (chanNum-cumNumChans<0 || chanNum-cumNumChans>numChannels())
		return AcquisitionChannel();
	return acquisitionGroups.at(chanNumToGroupNum.at(chanNum-cumNumChans))->getChan(chanNum);

}