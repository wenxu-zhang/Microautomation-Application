// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Perimeterfocus.h"
#include "Controller.h"
#include "XYStage.h"
#include <algorithm>
#include <limits>
#include <math.h>
extern Controller cont;
extern Record logFile;

PerimeterFocus::PerimeterFocus(const Focus& f, int minX,int minY,int maxX,int maxY, int numX, int numY):ScanFocus(f){
	this->minX=minX;
	this->maxX=maxX;
	this->minY=minY;
	this->maxY=maxY;
	this->numX=numX;
	this->numY=numY;
	x=new NRVec<DP>(numX);
	y=new NRVec<DP>(numY);
	zx0=new vector<FocusInTemp>(numX);
	zx1=new vector<FocusInTemp>(numX);
	zy0=new vector<FocusInTemp>(numY);
	zy1=new vector<FocusInTemp>(numY);
	initialFocus();
}
PerimeterFocus::PerimeterFocus(const Focus& f, int minX,int minY,int maxX,int maxY,double maxStdDev):ScanFocus(f){
	this->minX=minX;
	this->maxX=maxX;
	this->minY=minY;
	this->maxY=maxY;
	this->numX=numX;
	this->numY=numY;
	x=new NRVec<DP>(numX);
	y=new NRVec<DP>(numY);
	zx0=new vector<FocusInTemp>(numX);
	zx1=new vector<FocusInTemp>(numX);
	zy0=new vector<FocusInTemp>(numY);
	zy1=new vector<FocusInTemp>(numY);
	initialFocus();
}
PerimeterFocus::PerimeterFocus(int numx,int numy):ScanFocus(cont.currentFocus){
	minX=2000000;
	minY=2000000;
	maxX=-2000000;
	maxY=-2000000;
	int c=0;
	cout<<"Please select the 2 corners for the focusing area.\n";
	cont.currentChannel().on();
	cont.currentChannel().out->cam->startLiveView();
	for(int i=0;i<2;i++){
		while(c==0){
			cout<<"After exiting stageControl, press 1 to accept position and 0 to select again.\n";
			cont.stg->stageControl();
			cin>>c;
			std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			if(c==1){
				minX=min(minX,cont.stg->getX());
				maxX=max(maxX,cont.stg->getX());
				minY=min(minY,cont.stg->getY());
				maxY=max(maxY,cont.stg->getY());
				logFile.write("PerimeterFocus: ("+toString(cont.stg->getX())+", "+toString(cont.stg->getY())+") accepted",true);
			}
		}
		c=0;
	}
	cont.currentChannel().out->cam->stopLiveView();
	cont.currentChannel().off();
	//Set numX and numY so that it covers every area along the perimeter.
	if(numx==-1)numx=ceil((maxX-minX)/cont.currentFocus.a.out->cam->getImageWidth(cont.currentFocus.a.ap))+1;
	if(numy==-1)numy=ceil((maxY-minY)/cont.currentFocus.a.out->cam->getImageHeight(cont.currentFocus.a.ap))+1;
	(*this)=PerimeterFocus(cont.currentFocus,minX,minY,maxX,maxY,numx,numy);
}	
void PerimeterFocus::initialFocus(){
	int stepX,stepY;			//step size to take in x and y
	if (numX==1) stepX=0;
	else stepX=(maxX-minX)/(numX-1);
	if (numY==1) stepY=0;
	else stepY=(maxY-minY)/(numY-1);
	cont.stg->move(minX,minY);	//start at (minX,minY)
	for(int i=0;i<numX;i++){	//move in x direction, keeping y at minY
		if(i!=0) cont.stg->incX(stepX);
		cont.stg->wait();
		(*x)[i]=cont.stg->getX();
		(*zx0)[i]=FocusInTemp(f,(*x)[i],cont.stg->getY(),f.getFocus(true,-1,true),cont.te.getTemp());
		logFile.write("PerimeterFocus: x["+toString(i)+"]="+toString((*x)[i])+"\tzx0["+toString(i)+"]="+toString((*zx0)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[numX-1])+"\t"+toString(minY)+"\t"+toString((*zx0)[numX-1].z);
	cont.te.triggerRecord=true;
	(*y)[0]=cont.stg->getY();
	(*zy0)[0]=(*zx0)[numX-1];
	for(int i=1;i<numY;i++){	//From (maxX,minY), move in y direction, keeping x at maxX
		cont.stg->incY(stepY);
		cont.stg->wait();
		(*y)[i]=cont.stg->getY();
		(*zy0)[i]=FocusInTemp(f,(*x)[numX-1],(*y)[i],f.getFocus(true,-1,true),cont.te.getTemp());
		logFile.write("PerimeterFocus: y["+toString(i)+"]="+toString((*y)[i])+"\tzy0["+toString(i)+"]="+toString((*zy0)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[numX-1])+"\t"+toString((*y)[numY-1])+"\t"+toString((*zy0)[numY-1].z);
	cont.te.triggerRecord=true;
	(*zx1)[numX-1]=(*zy0)[numY-1];
	for(int i=numX-2;i>=0;i--){	//From (maxX,maxY) move backwards in x direction, keeping y at maxY
		cont.stg->incX(-stepX);
		cont.stg->wait();
		(*zx1)[i]=FocusInTemp(f,(*x)[i],(*y)[numY-1],f.getFocus(true,-1,true),cont.te.getTemp());
		logFile.write("PerimeterFocus: x["+toString(i)+"]="+toString((*x)[i])+"\tzx1["+toString(i)+"]="+toString((*zx1)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[0])+"\t"+toString((*y)[numY-1])+"\t"+toString((*zy1)[0].z);
	cont.te.triggerRecord=true;
	(*zy1)[numY-1]=(*zx1)[0];
	for(int i=numY-2;i>=1;i--){	//From (minX,maxY) move backwards in y direction, keeping x at minX
		cont.stg->incY(-stepY);
		cont.stg->wait();
		(*zy1)[i]=FocusInTemp(f,(*x)[0],(*y)[i],f.getFocus(true,-1,true),cont.te.getTemp());
		logFile.write("PerimeterFocus: y["+toString(i)+"]="+toString((*y)[i])+"\tzy1["+toString(i)+"]="+toString((*zy1)[i].z)+"\t");
	}
	(*zy1)[0]=(*zx0)[0];
	cont.te.comment=toString((*x)[0])+"\t"+toString((*y)[0])+"\t"+toString((*zy1)[0].z);
	cont.te.triggerRecord=true;
}

void PerimeterFocus::updateFocus(){
	cont.stg->move((*x)[0],(*y)[0]);
	for(int i=0;i<numX;i++){	//move in x direction, keeping y at minY
		if(i!=0) cont.stg->moveX((*x)[i]);
		cont.stg->wait();
		(*zx0)[i].z=f.getFocus(true,(*zx0)[i].getFocusInTemp(),true);
		(*zx0)[i].setZeroTempFocus();
		logFile.write("PerimeterFocus: x["+toString(i)+"]="+toString((*x)[i])+"\tzx0["+toString(i)+"]="+toString((*zx0)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[numX-1])+"\t"+toString(minY)+"\t"+toString((*zx0)[numX-1].z);
	cont.te.triggerRecord=true;
	(*zy0)[0]=(*zx0)[numX-1];
	for(int i=1;i<numY;i++){	//From (maxX,minY), move in y direction, keeping x at maxX
		cont.stg->moveY((*y)[i]);
		cont.stg->wait();
		(*zy0)[i].z=f.getFocus(true,(*zy0)[i].getFocusInTemp(),true);
		(*zy0)[i].setZeroTempFocus();
		logFile.write("PerimeterFocus: y["+toString(i)+"]="+toString((*y)[i])+"\tzy0["+toString(i)+"]="+toString((*zy0)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[numX-1])+"\t"+toString((*y)[numY-1])+"\t"+toString((*zy0)[numY-1].z);
	cont.te.triggerRecord=true;
	(*zx1)[numX-1]=(*zy0)[numY-1];
	for(int i=numX-2;i>=0;i--){	//From (maxX,maxY) move backwards in x direction, keeping y at maxY
		cont.stg->moveX((*x)[i]);
		cont.stg->wait();
		(*zx1)[i].z=f.getFocus(true,(*zx1)[i].getFocusInTemp(),true);
		(*zx1)[i].setZeroTempFocus();
		logFile.write("PerimeterFocus: x["+toString(i)+"]="+toString((*x)[i])+"\tzx1["+toString(i)+"]="+toString((*zx1)[i].z)+"\t");
	}
	cont.te.comment=toString((*x)[0])+"\t"+toString((*y)[numY-1])+"\t"+toString((*zy1)[0].z);
	cont.te.triggerRecord=true;
	(*zy1)[numY-1]=(*zx1)[0];
	for(int i=numY-2;i>=1;i--){	//From (minX,maxY) move backwards in y direction, keeping x at minX
		cont.stg->moveY((*y)[i]);
		cont.stg->wait();
		(*zy1)[i].z=f.getFocus(true,(*zy1)[i].getFocusInTemp(),true);
		(*zy1)[i].setZeroTempFocus();
		logFile.write("PerimeterFocus: y["+toString(i)+"]="+toString((*y)[i])+"\tzy1["+toString(i)+"]="+toString((*zy1)[i].z)+"\t");
	}
	(*zy1)[0]=(*zy1)[0];
	cont.te.comment=toString((*x)[0])+"\t"+toString((*y)[0])+"\t"+toString((*zy1)[0].z);
	cont.te.triggerRecord=true;
}

double PerimeterFocus::getFocus(int xpos,int ypos){
	secondDerX0=new NRVec<DP>(numX);
	secondDerX1=new NRVec<DP>(numX);
	secondDerY0=new NRVec<DP>(numY);
	secondDerY1=new NRVec<DP>(numY);
	double zposx1,zposy1,zposx0,zposy0,zposx,zposy;
	//zx0l...zy1l are z positions at x and y after correcting for temperature by FocusInTemp
	NRVec<DP> *zx0l=new NRVec<DP>(numX);
	NRVec<DP> *zx1l=new NRVec<DP>(numX);
	NRVec<DP> *zy0l=new NRVec<DP>(numY);
	NRVec<DP> *zy1l=new NRVec<DP>(numY);
	int j=0;
	for(vector<FocusInTemp>::iterator i=zx0->begin();i!=zx0->end();i++,j++){
		zx0l[j]=(*i).z;
	}
	j=0;
	for(vector<FocusInTemp>::iterator i=zx1->begin();i!=zx1->end();i++,j++){
		zx1l[j]=(*i).z;
	}
	j=0;
	for(vector<FocusInTemp>::iterator i=zy0->begin();i!=zy0->end();i++,j++){
		zy0l[j]=(*i).z;
	}
	j=0;
	for(vector<FocusInTemp>::iterator i=zy1->begin();i!=zy1->end();i++,j++){
		zy1l[j]=(*i).z;
	}
/*	if (numX==1) {
		zposx=zx0->front().z;
	}else{ 
		NR::spline(*x,*zx0l,1.0e30,1.0e30,*secondDerX0);
		NR::splint(*x,*zx0l,*secondDerX0,xpos,zposx0);
		NR::spline(*x,*zx1l,1.0e30,1.0e30,*secondDerX1);
		NR::splint(*x,*zx1l,*secondDerX1,xpos,zposx1);
		zposx=zposx0+(zposx1-zposx0)*(ypos-(int)(*y)[0])/((int)(*y)[numY-1]-(int)(*y)[0]);
	}
	if (numY==1) {
		zposy=zy0->front().z;
	}else{ 
		NR::spline(*y,*zy0l,1.0e30,1.0e30,*secondDerY0);
		NR::splint(*y,*zy0l,*secondDerY0,ypos,zposy0);
		NR::spline(*y,*zy1l,1.0e30,1.0e30,*secondDerY1);
		NR::splint(*y,*zy1l,*secondDerY1,ypos,zposy1);
		zposy=zposy0+(zposy1-zposy0)*(xpos-(int)(*x)[0])/((int)(*x)[numX-1]-(int)(*x)[0]);
	}
	logFile.write("PerimeterFocus: ("+toString(xpos)+","+toString(ypos)+") zposx="+toString(zposx)+" zposy="+toString(zposy)
		+"\n(*y)[0]="+toString((*y)[0])+" (*y)[numY-1]="+toString((*y)[numY-1])+" (*x)[0]="+toString((*x)[0])+" (*x)[numX-1]="+toString((*x)[numX-1]));
	if (numX==1) return zposy;
	else if (numY==1) return zposx;
	else return (zposx+zposy)/2;
*/
	if (numX==1 && numY==1)
		return zx0->front().z;
	else if (numX==1){
		NR::spline(*y,*zy0l,1.0e30,1.0e30,*secondDerY0);
		NR::splint(*y,*zy0l,*secondDerY0,ypos,zposy0);
		return zposy0;
	}else if (numY==1){
		NR::spline(*x,*zx0l,1.0e30,1.0e30,*secondDerX0);
		NR::splint(*x,*zx0l,*secondDerX0,xpos,zposx0);
		return zposx0;		
	}else{
		NRVec<DP> *xx=new NRVec<DP>(2);
		NRVec<DP> *yy=new NRVec<DP>(2);
		NRMat<DP> *zx=new NRMat<DP>(numX,2);
		NRMat<DP> *zy=new NRMat<DP>(2,numY);
		NRMat<DP> *secondDerX=new NRMat<DP>(numX,2);
		NRMat<DP> *secondDerY=new NRMat<DP>(2,numY);
		for(int i=0;i<zx0l->size();i++){
			(*zx)[i][0]=(*zx0l)[i];
			(*zx)[i][1]=(*zx1l)[i];
		}
		(*xx)[0]=(*x)[0];
		(*xx)[1]=(*x)[numX-1];
		NR::splie2(*xx,*y,*zx,*secondDerX);
		NR::splin2(*xx,*y,*zx,*secondDerX,xpos,ypos,zposx);
		for(int i=0;i<zy0l->size();i++){
			(*zy)[0][i]=(*zy0l)[i];
			(*zy)[1][i]=(*zy1l)[i];
		}
		(*yy)[0]=(*y)[0];
		(*yy)[1]=(*y)[numY-1];
		NR::splie2(*x,*yy,*zy,*secondDerY);
		NR::splin2(*x,*yy,*zy,*secondDerY,xpos,ypos,zposy);
		return (zposx+zposy)/2;
	}
}