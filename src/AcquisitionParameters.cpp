// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#define NOMINMAX
#pragma once
#include "AcquisitionParameters.h"
#include <stdio.h>
#include <limits>
#include "Record.h"

using namespace std;

extern Record logFile;

//AcquisitionParameters::AcquisitionParameters():exp(.05),gain(3),bin(1),imageRegion(ImageRegion::FULL),asEightBit(false){}


AcquisitionParameters::AcquisitionParameters(double exp,vector<int> gains,int bin, ImageRegion imageRegion,bool asEightBit,std::vector<double>& liteStarts,std::vector<double>& liteEnds,vector<double>& camDelays):exp(exp),bin(bin),imageRegion(imageRegion),asEightBit(asEightBit),liteStarts(liteStarts),liteEnds(liteEnds),camDelays(camDelays){
	if (gains.empty()){
		logFile.write("gains cannot be empty",true);
		gains.push_back(0);
	}else{
		this->gains=gains;
	}
	if (liteStarts.size()!=liteEnds.size()){
		logFile.write("Error in Acquisition Parameters: Both a start and end time for a light is required");
		liteStarts.clear();
		liteEnds.clear();
		return;
	}
	int k=0;
	for(vector<double>::iterator i=liteStarts.begin();i!=liteStarts.end();i++,k++){
		if (*i>=exp){
			logFile.write("Error in Acquisition Parameters: start time for light must be less than exposure time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
		if (liteEnds.at(k)>=exp){
			logFile.write("Error in Acquisition Parameters: end time for light must be less than exposure time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
		if (liteEnds.at(k)<=*i){
			logFile.write("Error in Acquisition Parameters: start time for light must be less than end time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
	}
	for(vector<double>::iterator i=camDelays.begin();i!=camDelays.end();i++){
		if (*i>=exp){
			logFile.write("Error in Acqusition Parameters: camera delays must be less than the exposure time",true);
			camDelays.clear();
		}
	}
}


AcquisitionParameters::AcquisitionParameters(double exp,int gain,int bin, ImageRegion imageRegion,bool asEightBit,std::vector<double>& liteStarts,std::vector<double>& liteEnds,vector<double>& camDelays):exp(exp),bin(bin),gains(1,gain),imageRegion(imageRegion),asEightBit(asEightBit),liteStarts(liteStarts),liteEnds(liteEnds),camDelays(camDelays){
	if (liteStarts.size()!=liteEnds.size()){
		logFile.write("Error in Acquisition Parameters: Both a start and end time for a light is required");
		liteStarts.clear();
		liteEnds.clear();
		return;
	}
	int k=0;
	for(vector<double>::iterator i=liteStarts.begin();i!=liteStarts.end();i++,k++){
		if (*i>=exp){
			logFile.write("Error in Acquisition Parameters: start time for light must be less than exposure time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
		if (liteEnds.at(k)>=exp){
			logFile.write("Error in Acquisition Parameters: end time for light must be less than exposure time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
		if (liteEnds.at(k)<=*i){
			logFile.write("Error in Acquisition Parameters: start time for light must be less than end time",true);
			liteStarts.clear();
			liteEnds.clear();
			return;
		}
	}
	for(vector<double>::iterator i=camDelays.begin();i!=camDelays.end();i++){
		if (*i>=exp){
			logFile.write("Error in Acqusition Parameters: camera delays must be less than the exposure time",true);
			camDelays.clear();
		}
	}
}

double AcquisitionParameters::getStart(int n){
	if (n<0 || n>=liteStarts.size()){
		return 0;
	}else{
		return liteStarts.at(n);
	}
}

double AcquisitionParameters::getEnd(int n){
	if (n<0 || n>=liteStarts.size()){
		return exp;
	}else{
		return liteEnds.at(n);
	}
}

double AcquisitionParameters::getCamDelay(int n){
	if (n<0 || n>=camDelays.size()){
		return 0;
	}else{
		return camDelays.at(n);
	}
}

int AcquisitionParameters::getGain(int n){
	if (n<0 || n>=gains.size()){
		return gains.at(0);
	}else{
		return gains.at(n);
	}
}

void AcquisitionParameters::setGain(int gain){
	gains.clear();
	gains.push_back(gain);
}

void AcquisitionParameters::setGain(vector<int> gains){
	if (gains.size()==0){
		this->gains.clear();
		gains.push_back(0);
	}else{
		this->gains=gains;
	}
}

string AcquisitionParameters::getGains(){
	if (gains.empty())
		return ::toString(0);
	string ret=::toString(gains.at(0));
	for(vector<int>::iterator i=gains.begin()+1;i!=gains.end();i++){
		ret+=","+::toString(*i);
	}
	return ret;
}

std::string AcquisitionParameters::toString(){
	return std::string("bin: ")+::toString(bin)+"exp: "+::toString(exp)+"gain: "+getGains()+"image region: "+imageRegion.name;
}

void AcquisitionParameters::modify(){
	char c;
	while(true){
		cout<<"1: Change exposure     ("<<exp<<")"<<endl;
		cout<<"2: Change gain         ("<<getGains()<<")"<<endl;
		cout<<"3: Change bin          ("<<bin<<")"<<endl;
		cout<<"4: Change ImageRegion  ("<<imageRegion.name<<")"<<endl;
		cout<<"e: exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':{
				double d=0;
				cout<<"Enter exposure time in seconds"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				exp=d;
				break;}
			case '2':{
				int i=0;
				cout<<"Enter gain (integer)"<<endl;
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				gains.clear();
				gains.push_back(i);
				break;}
			case '3':{
				int i=0;
				cout<<"Enter binning (integer)"<<endl;
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				bin=i;
				break;}
			case '4':
				imageRegion=ImageRegion::select();//modify();
				break;
			case 'e':
				return;
				break;
		}
	}
}
