// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, February 24, 2011</lastedit>
// ===================================
#include "Camera.h"
#include "Record.h"
#include <iostream>
using namespace std;
using namespace cimg_library;
extern Record logFile;
#include "Scan.h"
Camera::Camera(Trigger* t,vector<int> triggerLines,int orientation):t(t),triggerLines(triggerLines),orientation(orientation){
	if (t){
		for(vector<int>::iterator i=triggerLines.begin();i!=triggerLines.end();i++){	
			if (!t->isValidLine(*i))
				logFile.write(string("Error: Camera trigger line ")+toString(*i)+" is not valid");
		}
	}
}

HANDLE Camera::getAbortEvent(Scan *s){
	return s->abortEvent;
}

Camera::~Camera(){}
void Camera::trigger(){
#ifdef DEBUGCAMERA
	Timer t2(true);
#endif
	//t->setLineLow(triggerLine); the line should always be low by default
	t->triggerMultipleLines(triggerLines);
#ifdef DEBUGCAMERA
	t2.stopTimer();
	logFile.write(string("Camera Trigger took ")+toString(t2.getTime())+" ms");
#endif
}

void Camera::startLiveView(){
	RECT area;
	SystemParametersInfo(SPI_GETWORKAREA,0,&area,0);
	int upperLeftX=int(area.right*(CONSOLEWIDTH))-1;
	int upperLeftY=0;
	int liveWidth=area.right*(1-CONSOLEWIDTH)+1;
	int liveHeight=area.bottom;
	startLiveView(LiveViewData(upperLeftX,upperLeftY,liveWidth,liveHeight,&scaleFactor,&centerX,&centerY,&zoom,&centerXEff,&centerYEff,-1));

	
}


double Camera::getZoom(std::vector<double>& scaleFactor){
	double ret=1;
	for(std::vector<double>::iterator i=scaleFactor.begin();i!=scaleFactor.end();i++){
		ret=ret*(*i);
	}
	return ret;
}

void Camera::getCenter(LiveViewData& lvd){
	*lvd.centerXEff=0;
	*lvd.centerYEff=0;
	
	int i=0;
	for(vector<double>::iterator lev=lvd.scaleFactor->begin();lev!=lvd.scaleFactor->end();i++,lev++){
		*lvd.centerXEff=*lvd.centerXEff+lvd.centerX->at(i)/getZoom(vector<double>(lvd.scaleFactor->begin(),lvd.scaleFactor->begin()+i));
		*lvd.centerYEff=*lvd.centerYEff+lvd.centerY->at(i)/getZoom(vector<double>(lvd.scaleFactor->begin(),lvd.scaleFactor->begin()+i));
	}
}

//the resulting image will be the zoomed image with one dimension equal to either the display width or the height but not necessarily both
CImg<unsigned short> Camera::getAdjustedImage(CImg<unsigned short>& original, double scale, double centerX, double centerY, int width, int height,double& finalScale, int& beforeX,int& afterX, int& beforeY, int& afterY){
	double minY=0,maxY=0,minX=0,maxX=0;
	if (double(width)/original.width()>double(height)/original.height()){
		//crop height based on scaling and crop width based on size of display
		minY=centerY-1.0/(scale*2);
		maxY=centerY+1.0/(scale*2);
		minX=centerX-(maxY-minY)*width*original.height()/height/original.width()/2;
		maxX=centerX+(maxY-minY)*width*original.height()/height/original.width()/2;
		//minX=centerX-double(width)/original.width()/(scale*2)+.5;
		//maxX=centerX+double(width)/original.width()/(scale*2)+.5;
	}else{
		//crop width based on scaling and crop height based on size of display
		//minY=centerY-double(height)/original.height()/(scale*2)+.5;
		//maxY=centerY+double(height)/original.height()/(scale*2)+.5;		
		minX=centerX-1.0/(scale*2);
		maxX=centerX+1.0/(scale*2);
		minY=centerY-(maxX-minX)*height*original.width()/width/original.height()/2;
		maxY=centerY+(maxX-minX)*height*original.width()/width/original.height()/2;
	}

	minX=minX+.5;
	minY=minY+.5;
	maxX=maxX+.5;
	maxY=maxY+.5;

	minX=(minX*original.width());
	maxX=(maxX*original.width())-1;
	minY=(minY*original.height());
	maxY=(maxY*original.height())-1;
	int iminX=floor(minX),iminY=floor(minY),imaxX=ceil(maxX),imaxY=ceil(maxY);
	if (minX<0) iminX=0;
	if (minY<0) iminY=0;
	if (maxX<0) imaxX=0;
	if (maxY<0) imaxY=0;
	if (minX>original.width()) iminX=original.width()-1;
	if (minY>original.height()) iminY=original.height()-1;
	if (maxX>original.width()) imaxX=original.width()-1;
	if (maxY>original.height()) imaxY=original.height()-1;
	CImg<unsigned short> res=original.get_crop(iminX,iminY,imaxX,imaxY,false);
	
	//scale it NOTE: this resize function actually stretches the image causing distortion.  There is a distortion free way to do this but it has not been implemented in the CImg Library. Would you like to volunteer? It is implimented in ImageJ so you could just copy the code
	double realScale=min(double(height)/(maxY-minY+1),double(width)/(maxX-minX+1));
	finalScale=double(round(res.width()*realScale))/res.width();
	res.resize(round(res.width()*realScale),round(round(res.width()*realScale)*double(res.height())/res.width()),-100,-100);
	//now crop to width and height of display
	beforeX=0;
	beforeY=0;
	afterX=0;
	afterY=0;
	if (res.height()<height){
		//need to zero pad height, no cropping height
		res.crop(round((res.width()-width)/2.0),0,round((res.width()-width)/2.0)+width-1,res.height()-1);
		beforeY=max(0,min(height-res.height(),int(ceil(-minY*finalScale))));
		res=CImg<unsigned short>(res.width(),beforeY,1,1,res.min()).append(res,'y');
		afterY=height-res.height();
		res.append(CImg<unsigned short>(res.width(),afterY,1,1,res.min()),'y');
	}else if (res.width()<width){
		//need to zero pad width, no cropping width
		res.crop(0,round((res.height()-height)/2.0),res.width()-1,round((res.height()-height)/2.0)+height-1);
		beforeX=max(0,min(width-res.width(),int(ceil(-minX*finalScale))));
		res=CImg<unsigned short>(beforeX,res.height(),1,1,res.min()).append(res,'x');
		afterX=width-res.width();
		res.append(CImg<unsigned short>(afterX,res.height(),1,1,res.min()),'x');
	}else{
		//no need for zero padding at all, just crop to display image
		res.crop(round((res.width()-width)/2.0),round((res.height()-height)/2.0),round((res.width()-width)/2.0)+width-1,round((res.height()-height)/2.0)+height-1);
	}

	return res;
}


CImg<unsigned short> Camera::getFinalImage(LiveViewArg& lva, CImg<unsigned short>& copyImageDisp,double zoom, double mag){
	CImg<unsigned short> res=copyImageDisp;
	
	//hack because normalization in CImg doesnt seem to work.
	res=res-res.min();
	res=res*(255.0/res.max());
	
	/*unsigned short myMin=copyImageDisp.min();
	//pad image with min value to fit display window
	if (copyImageDisp.width()<lva.a->liveDisp.width()){
		//need to expand the width and fill with the min value from the image
		res=CImg<unsigned short>(floor((lva.a->liveDisp.width()-copyImageDisp.width())/2.0),lva.a->liveDisp.height(),1,1,res.min()).append(res,'x');
		res.append(CImg<unsigned short>(ceil((lva.a->liveDisp.width()-copyImageDisp.width())/2.0),lva.a->liveDisp.height(),1,1,res.min()),'x');
	}else{
		//need to expand the height and fill with the min value from the image
		res=CImg<unsigned short>(lva.a->liveDisp.width(),floor((lva.a->liveDisp.height()-copyImageDisp.height())/2.0),1,1,res.min()).append(res,'y');
		res.append(CImg<unsigned short>(lva.a->liveDisp.width(),ceil((lva.a->liveDisp.height()-copyImageDisp.height())/2.0),1,1,res.min()),'y');
	}
	*/
	//add scale bar and max pixel value
	int numPixels;//=round(.3*lva.a->getImageWidth()/a->getPixelSize());
	//double dist=numPixels*lva.a->getPixelSize()/zoom;
	double dist=lva.lvd.width*.3*lva.a->getPixelSize()/zoom/mag;
	dist=round(dist/pow(10.0,floor(log10(dist))))*pow(10.0,floor(log10(dist)));
	numPixels=dist*zoom*mag/lva.a->getPixelSize();

	string sacle,maxIntensity;
	maxIntensity="Max Intensity is "+toString((int) copyImageDisp.max())+" out of a possible "+toString(int(pow(2.0,lva.a->getBitDepth()))-1);
	string scale;
	if (dist<1)
		scale=toString(dist*1000,0)+" nm";
	else
		scale=toString(dist,0)+" um";
	unsigned short fg=res.max();
	unsigned short bg=res.min();
	if (fg==bg){
		fg=255;
		bg=0;
	}
	res.draw_rectangle(10,10,10+numPixels,10+numPixels/(8.0*sqrt((float)2)),&fg);
	res.draw_text(10+numPixels/2,10+numPixels/(8*sqrt((float)2))+1,scale.c_str(),&fg,&bg,1,20);
	res.draw_text( 20+numPixels,15,maxIntensity.c_str(),&fg,&bg,1,20);
	return res;
}

/*std::string Camera::toString(int n){
	if (n>=name.size() || n==-1)
		n=1;
	return name.at(n);

}
*/
/*
HANDLE Camera::startSeries(int numImages,int seriesNum,Scan* s){return 0;}//exposure times determined by external triggers. Will use a temporary folder to save files to.  The folder will be deleted before each acquistion  
double Camera::getTemp(){return 0;}
void Camera::waitTemp(){}
//virtual void saveSeries(int num, string fileName);//This will save the appropriate file to the corresponding file name as a tiff
double Camera::startFocusSeries(int num,AcquisitionParameters ap){return 0;}//needs trigger to start
int Camera::getBestFocus(){return 0;}//return index to best image
LONGLONG Camera::takePicture(AcquisitionChannel* ac,std::string fileName){return 0;}
void Camera::takeAccumulation(AcquisitionChannel *ac,int num,std::string fileName){}
void Camera::startLiveView(){}
void Camera::saveLiveImage(std::string fileName){}
void Camera::stopLiveView(){}
double Camera::getImageWidth(AcquisitionParameters ap){return 0;}//return image width in microns
double Camera::getImageHeight(AcquisitionParameters ap){return 0;}//return image height in microns
void Camera::wait(){}
bool Camera::validate(AcquisitionParameters ap){return true;}//verify these acquisition parameters can be achieved with this camera
bool Camera::isTriggerable(){if (t) return true; else return false;}
void Camera::adjustIntensity(AcquisitionChannel &ac, double percentSaturation){}
*/