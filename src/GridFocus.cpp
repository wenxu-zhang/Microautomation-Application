// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Gridfocus.h"
#include "Controller.h"
#include "XYStage.h"
extern Controller cont;
extern Record logFile;
GridFocus::GridFocus(const Focus& f, int minX,int minY,int maxX,int maxY, int numX, int numY):ScanFocus(f){}
GridFocus::GridFocus(const Focus& f, int minX,int minY,int maxX,int maxY,double maxStdDev):ScanFocus(f){}
GridFocus::GridFocus():ScanFocus(cont.currentFocus){}
double GridFocus::getFocus(int xpos,int ypos){
	double ret;
	NR::splin2(*(this->y),*(this->x),*(this->z),*secondDer,ypos,xpos,ret);
	return ret;
}
void GridFocus::initialFocus(){
	//to be implemented
}
/*
void GridFocus::regionFocus4(double fineRange){
	//scan 2x2 sample surrounding the scan region and build 2x2 bicubic spline matrices;
	int numXsamples=2;
	int numYsamples=2;
	z=new NRMat<DP>(2,2);
	bool deb=false;
	//NRMat<DP> *newZ=new NRMat<DP>(numYsamples,numXsamples);
	x=new NRVec<DP>(2);
	//NRVec<DP> *newX=new NRVec<DP>(numXsamples);
	y=new NRVec<DP>(2);
	//NRVec<DP> *newY=new NRVec<DP>(numYsamples);
	double DOF=focus->getDOF();
	//Focus at 2 steps away from the region to scan through
	stg->moveY(scanMinY-2*yStep);
	(*y)[0]=scanMinY-2*yStep;
	stg->moveX(scanMinX-2*xStep);
	(*x)[0]=scanMinX-2*xStep;
	(*z)[0][0]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,0.03,focus->getPos());
	cout << "measured:\t( " << (*x)[0] << ",\t" << (*y)[0] << " )\tz= " << (*z)[0][0] << endl;
	logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString((*x)[0] )+" Y:"+toString((*y)[0])+ " ZPos:"+toString((*z)[0][0]),DEBUGCONTROLLER);
	
	stg->moveX(scanMaxX+2*xStep);
	(*x)[1]=scanMaxX+2*xStep;
	(*z)[1][0]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,0.03,(*z)[0][0]);
	cout << "measured:\t( " << (*x)[1] << ",\t" << (*y)[0] << " )\tz= " << (*z)[1][0] << endl;
	logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString((*x)[1]  )+" Y:"+toString((*y)[0])+ " ZPos:"+toString((*z)[1][0]),DEBUGCONTROLLER);

	stg->moveY(scanMaxY+2*yStep);
	(*y)[1]=scanMaxY+2*yStep;
	(*z)[1][1]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,0.03,(*z)[1][0]);
	cout << "measured:\t( " << (*x)[1] << ",\t" << (*y)[1] << " )\tz= " << (*z)[1][1] << endl;
	logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString((*x)[1]  )+" Y:"+toString((*y)[1])+ " ZPos:"+toString((*z)[1][1]),DEBUGCONTROLLER);
	
	stg->moveX(scanMinX-2*xStep);
	(*z)[0][1]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,0.03,(*z)[1][1]);
	cout << "measured:\t( " << (*x)[0] << ",\t" << (*y)[1] << " )\tz= " << (*z)[0][1] << endl << endl;
	logFile.write("Temp: "+toString(te.getTemp())+" OutputPower:"+toString(te.getOutputPower())+" X:"+toString((*x)[0] )+" Y:"+toString((*y)[1])+ " ZPos:"+toString((*z)[0][1]),DEBUGCONTROLLER);
	
	secondDer=new NRMat<DP>(numYsamples,numXsamples);
	NR::splie2(*y,*x,*z,*secondDer);

	focusAdjust=true;
	return;//for testing purposes only
}
*/
/*
void GridFocus::regionFocus9(){
	//scan 3x3 sample region and build 2x2 and 3x3 bicubic spline matrices;
	//float exp=.1;
	int numXsamples=3;
	int numYsamples=3;
	z=new NRMat<DP>(2,2);
	bool deb=false;
	NRMat<DP> *newZ=new NRMat<DP>(numYsamples,numXsamples);
	x=new NRVec<DP>(2);
	NRVec<DP> *newX=new NRVec<DP>(numXsamples);
	y=new NRVec<DP>(2);
	NRVec<DP> *newY=new NRVec<DP>(numYsamples);
	double fineRange=20;//microns
	double coarseRange=500;//microns
	double DOF=focus->getDOF();
	double maxZ=focus->getMaxZ();
	double exp=cam->getExposure();
	for(int i=0;i<numYsamples;i++){
		if (i%2==0){
			for(int j=0;j<numXsamples;j++){//moving in the positive X direction
				cout<<"regionFocus step: ("<<j<<","<<i<<")"<<endl;
				if (deb) system("pause");
				if (j==0) {//start of line, move up first
					stg->moveY(scanMinY+round(i*diffY/(numYsamples-1)));
					(*newY)[i]=scanMinY+round(i*diffY/(numYsamples-1));
					if (i==0) {//first position
						stg->moveX(scanMinX);
						(*newX)[j]=scanMinX;
						(*newZ)[i][j]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,exp,autoFocus(focusChannel,ceil(coarseRange/(fineRange/10)), coarseRange, exp,maxZ-(coarseRange/2)));
					}else{
						(*newZ)[i][j]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,exp,(*newZ)[i-1][j]);
					}
				}else {
					stg->moveX(scanMinX+round(j*diffX/(numXsamples-1)));
					(*newX)[j]=scanMinX+round(j*diffX/(numXsamples-1));
					(*newZ)[i][j]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,exp,(*newZ)[i][j-1]);
				}
				
			}
		}else{
			for(int j=numXsamples-1;j>-1;j--){//moving in the negative X direction
				cout<<"regionFocus step: ("<<j<<","<<i<<")"<<endl;			
				if (deb) system("pause");
				if (j==numXsamples-1) {//start of line, move up first
					stg->moveY(scanMinY+round(i*diffY/(numYsamples-1)));
					(*newY)[i]=scanMinY+round(i*diffY/(numYsamples-1));
					(*newZ)[i][j]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,exp,(*newZ)[i-1][j]);
				}else {
					stg->moveX(scanMinX+round(j*diffX/(numXsamples-1)));
					(*newZ)[i][j]=autoFocus(focusChannel,ceil(fineRange/(DOF/2)),fineRange,exp,(*newZ)[i][j+1]);
				}
			}
		}
	}
	(*z)[0][0]=(*newZ)[0][0];
	(*z)[0][1]=(*newZ)[0][2];
	(*z)[1][1]=(*newZ)[2][2];
	(*z)[1][0]=(*newZ)[2][0];
	(*x)[0]=(*newX)[0];
	(*x)[1]=(*newX)[2];
	(*y)[0]=(*newY)[0];
	(*y)[1]=(*newY)[2];
	cout<<"x[0]= "<<(*x)[0] <<endl;
	cout<<"x[1]= "<<(*x)[1] <<endl;
	cout<<"y[0]= "<<(*y)[0] <<endl;
	cout<<"y[1]= "<<(*y)[1] <<endl;
	cout<<"z[0][0]= "<<(*z)[0][0] <<endl;
	cout<<"z[0][1]= "<<(*z)[0][1] <<endl;
	cout<<"z[1][1]= "<<(*z)[1][1] <<endl;
	cout<<"z[1][0]= "<<(*z)[1][0] <<endl;
	

	//calculate std deviation of estimation from 2x2 bicubic spline matrices;
	secondDer=new NRMat<DP>(2,2);
	NR::splie2(*y,*x,*z,*secondDer);
	cout<<"foc("<<(*newX)[1]<<","<<(*newY)[0]<<")="<<(*newZ)[0][1]<<" Predicted="<<getFocus((*newX)[1],(*newY)[0])<<endl;
	cout<<"foc("<<(*newX)[0]<<","<<(*newY)[1]<<")="<<(*newZ)[1][0]<<" Predicted="<<getFocus((*newX)[0],(*newY)[1])<<endl;
	cout<<"foc("<<(*newX)[1]<<","<<(*newY)[1]<<")="<<(*newZ)[1][1]<<" Predicted="<<getFocus((*newX)[1],(*newY)[1])<<endl;
	cout<<"foc("<<(*newX)[2]<<","<<(*newY)[1]<<")="<<(*newZ)[1][2]<<" Predicted="<<getFocus((*newX)[2],(*newY)[1])<<endl;
	cout<<"foc("<<(*newX)[1]<<","<<(*newY)[2]<<")="<<(*newZ)[2][1]<<" Predicted="<<getFocus((*newX)[1],(*newY)[2])<<endl;

	double stdDev=0;
	stdDev+=SQR<double>((*newZ)[0][1]-getFocus((*newX)[1],(*newY)[0]));
	stdDev+=SQR<double>((*newZ)[1][0]-getFocus((*newX)[0],(*newY)[1]));
	stdDev+=SQR<double>((*newZ)[1][1]-getFocus((*newX)[1],(*newY)[1]));
	stdDev+=SQR<double>((*newZ)[1][2]-getFocus((*newX)[2],(*newY)[1]));
	stdDev+=SQR<double>((*newZ)[2][1]-getFocus((*newX)[1],(*newY)[2]));
	stdDev=sqrt(stdDev/(5-1));//unbiased estimator of sigma
	cout<<"inital standard deviation is: "<<stdDev<<endl;
	delete x;
	delete y;
	delete z;
	delete secondDer;
	x=newX;
	y=newY;
	z=newZ;
	secondDer=new NRMat<DP>(numYsamples,numXsamples);
	NR::splie2(*y,*x,*z,*secondDer);
	//improve surface approximation if necessary

		RecordFile << "Region Focused at 9 points.\n";
	
	focusAdjust=true;
	return;//for testing purposes only

	if (stdDev>2*DOF) improveRegionFocus(3*stdDev);
}
*/

/*
void GridFocus::improveRegionFocus(double range){
	float exp=.1;
	double zEst;
	double stdDev=0;
	double DOF=focus->getDOF();
	//determine whether to increase x sampling or y sampling
	if (diffX/x->size()>diffY/y->size()){
		//increasing x sample rate
		int numYsamples=y->size();
		int numXsamples=x->size()*2-1;
		NRMat<DP> *newZ=new NRMat<DP>(numYsamples,numXsamples);
		NRVec<DP> *newX=new NRVec<DP>(numXsamples);
		//construct new sampling matrix from old matrix
		//also compute std deviation of new pts in the process
		for(int i=0;i<numYsamples;i++){
			if(i%2==0){//increasing x direction
				for(int j=0;j<x->size()-1;j++){
					if (j==0){
						(*newZ)[i][j]=(*z)[i][j];
						(*newX)[j]=(*x)[j];
						stg->moveY((*y)[i]);
						if (i==0) stg->moveX(round(((*x)[j+1]+(*x)[j])/2));
					}else stg->moveX(round(((*x)[j+1]+(*x)[j])/2));
					(*newZ)[i][2*j+2]=(*z)[i][j+1];
					zEst=getFocus(round(((*x)[j+1]+(*x)[j])/2),(*y)[i]);
					(*newZ)[i][2*j+1]=autoFocus(ceil(range/(DOF/2)),range,exp,false,zEst);
					stdDev+=SQR((*newZ)[i][2*j+1]-zEst);
					(*newX)[2*j+2]=(*x)[j+1];
					(*newX)[2*j+1]=round(((*x)[j+1]+(*x)[j])/2);
				}
			}else{//decreasing x direction
				for(int j=x->size()-1;j>0;j--){
					if (j==x->size()-1){
						(*newZ)[i][2*j]=(*z)[i][j];
						(*newX)[2*j]=(*x)[j];
						stg->moveY((*y)[i]);
					}else stg->moveX(round(((*x)[j-1]+(*x)[j])/2));
					(*newZ)[i][2*j-2]=(*z)[i][j-1];
					zEst=getFocus(round(((*x)[j-1]+(*x)[j])/2),(*y)[i]);
					(*newZ)[i][2*j-1]=autoFocus(ceil(range/(DOF/2)),range,exp,false,zEst);
					stdDev+=SQR((*newZ)[i][2*j-1]-zEst);
					(*newX)[2*j-2]=(*x)[j-1];
					(*newX)[2*j-1]=round(((*x)[j-1]+(*x)[j])/2);
				}
			}
		}
		stdDev=stdDev/((x->size()-1)*numYsamples-1);//unbiased estimator
		delete x;
		delete z;
		delete secondDer;
		x=newX;
		z=newZ;
		secondDer=new NRMat<DP>(numYsamples,numXsamples);
		NR::splie2(*y,*x,*z,*secondDer);
	}else{
		//increasing y sample rate
		int numYsamples=y->size()*2-1;
		int numXsamples=x->size();
		NRMat<DP> *newZ=new NRMat<DP>(numYsamples,numXsamples);
		NRVec<DP> *newY=new NRVec<DP>(numYsamples);
		//construct new sampling matrix from old matrix
		//also compute std deviation of new pts in the process
		for(int i=0;i<y->size()-1;i++){
			if(i%2==0){//increasing x direction
				for(int j=0;j<numXsamples;j++){
					if (i==0){
						(*newZ)[i][j]=(*z)[i][j];
						(*newY)[i]=(*y)[i];
						if (j==0) stg->move((*x)[j],round(((*y)[i]+(*y)[i+1])/2));
					}else if (j==0) stg->moveY(round(((*y)[i]+(*y)[i+1])/2));
					else stg->moveX((*x)[j]);
					(*newZ)[2*i+2][j]=(*z)[i+1][j];
					zEst=getFocus((*x)[j],round(((*y)[i+1]+(*y)[i])/2));
					(*newZ)[2*i+1][j]=autoFocus(ceil(range/(DOF/2)),range,exp,false,zEst);
					stdDev+=SQR((*newZ)[2*i+1][j]-zEst);
					(*newY)[2*i+2]=(*x)[i+1];
					(*newY)[2*i+1]=round(((*y)[i+1]+(*y)[i])/2);
				}
			}else{//decreasing x direction
				for(int j=numXsamples-1;j>-1;j--){
					if (j==numXsamples-1){
						stg->moveY(round(((*y)[i]+(*y)[i+1])/2));
					}else stg->moveX((*x)[j]);
					(*newZ)[2*i+2][j]=(*z)[i+1][j];
					zEst=getFocus((*x)[j],round(((*y)[i+1]+(*y)[i])/2));
					(*newZ)[2*i+1][j]=autoFocus(ceil(range/(DOF/2)),range,exp,false,zEst);
					stdDev+=SQR((*newZ)[2*i+1][j]-zEst);
					(*newY)[2*i+2]=(*y)[i+1];
					(*newY)[2*i+1]=round(((*y)[i+1]+(*y)[i])/2);
				}
			}
		}
		stdDev=stdDev/((y->size()-1)*numXsamples-1);//unbiased estimator
		delete y;
		delete z;
		delete secondDer;
		y=newY;
		z=newZ;
		secondDer=new NRMat<DP>(numYsamples,numXsamples);
		NR::splie2(*y,*x,*z,*secondDer);
	}
	cout<<"standard deviation before improvement: "<<stdDev<<endl;
	//improve surface approximation if necessary
//////////////////////////////////////////////////////////
	//should check to see if stdDev is improved
	
		RecordFile << "Region focus improved.\n";
	
	if (stdDev>2*DOF) improveRegionFocus(3*stdDev);
	focusAdjust=true;
}
*/

void GridFocus::updateFocus(){
	float exp=.1;
	//scan subset of points and determine stdDev
	double stdDev=0;
	double prevZ;
	//1
	cont.stg->move((*x)[0],(*y)[0]);
	prevZ=(*z)[0][0];
	(*z)[0][0]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[0][0]-prevZ);
	//2
	cont.stg->moveX((*x)[(x->size()-1)/2]);
	prevZ=(*z)[0][(x->size()-1)/2];
	(*z)[0][(x->size()-1)/2]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[0][(x->size()-1)/2]-prevZ);
	//3
	cont.stg->moveX((*x)[(x->size()-1)]);
	prevZ=(*z)[0][(x->size()-1)];
	(*z)[0][(x->size()-1)]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[0][(x->size()-1)]-prevZ);
	//4
	cont.stg->moveY((*y)[(y->size()-1)/2]);
	prevZ=(*z)[(y->size()-1)/2][(x->size()-1)];
	(*z)[(y->size()-1)/2][(x->size()-1)]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[(y->size()-1)/2][(x->size()-1)]-prevZ);
	//5
	cont.stg->moveX((*x)[(x->size()-1)/2]);
	prevZ=(*z)[(y->size()-1)/2][(x->size()-1)/2];
	(*z)[(y->size()-1)/2][(x->size()-1)/2]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[(y->size()-1)/2][(x->size()-1)/2]-prevZ);
	//6
	cont.stg->moveX((*x)[0]);
	prevZ=(*z)[(y->size()-1)/2][0];
	(*z)[(y->size()-1)/2][0]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[(y->size()-1)/2][0]-prevZ);
	//7
	cont.stg->moveY((*y)[y->size()-1]);
	prevZ=(*z)[(y->size()-1)][0];
	(*z)[(y->size()-1)][0]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[(y->size()-1)][0]-prevZ);
	//8
	cont.stg->moveX((*x)[(x->size()-1)/2]);
	prevZ=(*z)[(y->size()-1)][(x->size()-1)/2];
	(*z)[(y->size()-1)][(x->size()-1)/2]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[(y->size()-1)][(x->size()-1)/2]-prevZ);
	//9
	cont.stg->moveX((*x)[(x->size()-1)]);
	prevZ=(*z)[y->size()-1][(x->size()-1)];
	(*z)[y->size()-1][(x->size()-1)]=f.getFocus(false,prevZ);
	stdDev+=SQR((*z)[y->size()-1][(x->size()-1)]-prevZ);
	
	stdDev=stdDev/(9-1);//unbiased estimator
	//if stdDev is too large then rescan every point
	if (stdDev>4*f.step){
		cout<<"updating region focus"<<endl;
		for(int i=0;i<y->size();i++){
			if(i%2==0){
				for(int j=0;j<x->size();j++){//increasing x position
					if(j==0){
						cont.stg->moveY((*y)[i]);
						if (i==0) cont.stg->moveX((*x)[j]);
					}else cont.stg->moveX((*x)[j]);
					(*z)[i][j]=f.getFocus(false,(*z)[i][j]);
				}
			}else{
				for(int j=x->size()-1;j>-1;j--){//decreasing x position
					if(j==0){
						cont.stg->moveY((*y)[i]);
					}else cont.stg->moveX((*x)[j]);
					(*z)[i][j]=f.getFocus(false,(*z)[i][j]);
				}
			}
		}
	}else{
		cout<<"do not need to update region focus"<<endl;
	}
	NR::splie2(*y,*x,*z,*secondDer);

	logFile.write("Grid Region focus updated.\n");
	
}