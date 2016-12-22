// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Friday, January 13, 2012</lastedit>
// ===================================
#include "Chamber.h"
#include "Controller.h"
#include "XYStage.h"
extern Controller cont;
extern Record logFile;


Chamber::Chamber(double maxX,double minY,double maxZ, double focusZ, double mmChannelSpacing, double mmChannelLength,double mmChannelWidth,double uLChannelVolume,int numChannels,bool isX):
maxX(maxX),minY(minY),maxZ(maxZ),focusZ(focusZ),channelSpacing(mmChannelSpacing),channelLength(mmChannelLength),channelWidth(mmChannelWidth),channelHeight(uLChannelVolume/mmChannelWidth/mmChannelLength),numChannels(numChannels), isX(isX){
	if (isX){
		minX=maxX-channelLength*1000.0/cont.stg->getStepSize();//this will be the right most position of any channel
		maxY=minY+(numChannels-1)*channelSpacing*1000.0/cont.stg->getStepSize()+channelWidth*1000.0/cont.stg->getStepSize();//this will be the bottom most position of any channel
	}else{
		maxY=minY+channelLength*1000.0/cont.stg->getStepSize();//this will be the right most position of any channel
		minX=maxX-(numChannels-1)*channelSpacing*1000.0/cont.stg->getStepSize()-channelWidth*1000.0/cont.stg->getStepSize();//this will be the bottom most position of any channel
	}
		//maxX=;//this should be the same as zeroX so is not needed
		//maxY=;//this should be the same as zeroY so is not needed


}

Chamber::Chamber():maxX(-1),maxY(-1),maxZ(-1),focusZ(0),channelSpacing(0),channelLength(0),channelWidth(0),channelHeight(0),numChannels(0),minX(0),minY(0),isX(true){
}

bool Chamber::operator==(const Chamber& other) const{
	return minX==other.minX && minY==other.minY && maxZ==other.maxZ && focusZ==other.focusZ && channelSpacing==other.channelSpacing && channelLength==other.channelLength && channelWidth==other.channelWidth && channelHeight==other.channelHeight && numChannels==other.numChannels && maxX==other.maxX && maxY==other.maxY && isX==other.isX;
}

double Chamber::getChannelWidth(){return channelWidth;}//in mm
double Chamber::getChannelHeight(){return channelHeight;}//in mm
double Chamber::getChannelLength(){return channelLength;}//in mm
double Chamber::getChannelSpacing(){return channelSpacing;}//in mm
double Chamber::getChannelVolume(){
	double v=getChannelWidth()*getChannelHeight()*getChannelLength();
	return v;
}

double Chamber::getCurrentLengthPosition(){
	
	if (isX){
		int x=cont.stg->getX();
		if (x<minX || x>maxX){
			logFile.write("Error: current length position outside Chamber boundaries",true);
			return .5;
		}
		return (maxX-x)*(cont.stg->getStepSize()/1000.0)/channelLength;
	}else{
		int y=cont.stg->getY();
		if (y<minY || y>maxY){
			logFile.write("Error: current length position outside Chamber boundaries",true);
			return .5;
		}
		return (maxY-y)*(cont.stg->getStepSize()/1000.0)/channelLength;
	}
}
	
void Chamber::move(int channel,double widthFraction,double lengthFraction){
	if (!cont.stg->getIsPresent()){
		logFile.write("Chamber Error: Attempt to move stage when stage is not present",true); 
		return;
	}
	
	
	if (!cont.stg->isHomed()){
		logFile.write("Error: Chamber move is not supported when XY Stage has not been homed",true);
		return;
	}
	int currX=cont.stg->getX();
	int currY=cont.stg->getY();
	if (currX<minX || currX>maxX || currY<minY || currY>maxY){
		logFile.write("Error: Chamber move not supported when the xy stage is not already within the chamber limits (too dangerous), try device move if it is supported",true);
		return;
	}
	if (channel<1 || channel>numChannels){
		logFile.write("Error: this channel number is not supported by this chamber",true);
		return;
	}
	if (widthFraction<0.0 || widthFraction>1.0){
		logFile.write("Error: widthFraction must be between 0 and 1",true);
		return;
	}
	if (lengthFraction<0.0 || lengthFraction>1.0){
		logFile.write("Error: lengthFraction must be between 0 and 1",true);
	}
	int x,y;
	int	currentObj=0;
	Objective* o=cont.axio.getCurrentObjective();
	if (o->isOil)
		cont.stg->setSpeed(OILVELOCITY);
	if (isX){
		cont.stg->move(round(maxX-channelLength*(1000.0/cont.stg->getStepSize())*lengthFraction),round(minY+channelWidth*(1000.0/cont.stg->getStepSize())*widthFraction+(channel-1)*channelSpacing*(1000.0/cont.stg->getStepSize())));
	}else{
		cont.stg->move(round(maxX-channelWidth*(1000.0/cont.stg->getStepSize())*widthFraction-(channel-1)*channelSpacing*(1000.0/cont.stg->getStepSize())),round(minY+channelLength*(1000.0/cont.stg->getStepSize())*lengthFraction));
	}
	cont.stg->setSpeed(MAXVELOCITY);
}

