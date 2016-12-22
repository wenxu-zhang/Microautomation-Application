// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, May 12, 2011</lastedit>
// ===================================
#include "focus.h"
#include "Controller.h"
#include "Record.h"
#include "Magnification.h"
#include "ZStage.h"
#include "Camera.h"
#include <limits>

extern Record logFile,focusLogFile;
extern Controller cont;

Focus::Focus():a(cont.currentFocus.a),z(cont.currentFocus.z),range(cont.currentFocus.range),step(cont.currentFocus.step){
}
Focus::Focus(AcquisitionChannel a,double range, double stepSize):a(a),z(cont.focus),range(range),step(stepSize){}
Focus::Focus(AcquisitionChannel a,double range, int steps):a(a),z(cont.focus),step(range/steps){}
Focus::Focus(AcquisitionChannel a,double range):a(a),z(cont.focus),range(range),step(a.m->obj.getDOF()/4){}//will create focus object using DOF/4 as step size

double Focus::getDefiniteFocus(bool& isInRange, double range,double step,double center){
	int steps=(0.9999+(range/step));//round up
	range=step*steps;
	//we would like focus to potentially find the 'center' position as the focus->  This will allow for this
	double pos=center-((int)steps/2)*step;
	double start=pos;
	z->move(pos);
	z->wait();
	LONGLONG current=0,best=0;
	double bestZ=pos;
#ifdef DEBUGFOCUS
	if (DEBUGFOCUS)
		a.showOnScreen=true;
#endif
	int bestind=0;
	for(int i=0;i<steps;i++){
		current=this->a.out->cam->takePicture(&a,"!Image "+toString(i)+" Z:"+toString(pos));
		if (current>best){
			best=current;
			bestZ=pos;
			bestind=i;
		}
		pos=pos+step;
		z->move(pos);
		z->wait();
	}
	if (bestZ==start || bestZ==start+step*(steps-1)){
		isInRange=false;
		logFile.write("Focus found at edge of range. Likely focus not found or focus not in range",DEBUGFOCUS);
	}
	
#ifdef DEBUGFOCUS
	if (DEBUGFOCUS){
		logFile.write(string("Best index: ")+toString(bestind)+" Out of a total "+toString(steps)+" Best z was "+toString(bestZ)+" start: "+toString(start)+" end: "+toString(pos-step),true);
		system("pause");
		cont.displays.closeAll();
	}
#endif
	return bestZ;
}

double Focus::getContinuousFocus(bool& isInRange,double range, double step, double center){
	isInRange=true;
	int steps=(0.9999+(range/step));//round up
	Timer t2;
range=step*steps;
	//we would like focus to potentially find the 'center' position as the focus->  This will allow for this
	if ((steps%2)==1) z->move(center-range/2);//odd number of steps
	else z->move(center-range/2-step/2);//even number of steps
	double realexp=a.out->cam->startFocusSeries(steps,a.ap);
	double zSpeed=step/realexp;
	logFile.write(string("Focusing zSpeed is ")+toString(zSpeed)+"um/sec",DEBUGFOCUS);
	Timer t;
	z->wait();
	a.on(true);
	if (!z->velocityMove(zSpeed)){
		logFile.write("Focusing: zSpeed too slow...using definite stepwise focusing",DEBUGFOCUS); 
		return getDefiniteFocus(isInRange,range,step,center);
	}	
#ifdef DEBUGFOCUS
	t2.startTimer();
#endif
	a.out->cam->trigger();
	t2.stopTimer();
	t.startTimer();
	double start=z->getVelocityMoveStart();
	a.out->cam->wait();
	t.stopTimer();
	z->stop();
	a.off(true);
	//z->wait();
	double finish=z->getZ();
	Timer cameraDone(true);
	int bestInd=a.out->cam->getBestFocus();
	logFile.write("After camera done it took "+toString(cameraDone.getTime())+" ms to get score");
	if (bestInd==0 || bestInd==(steps-1)){
		isInRange=false;
		logFile.write("Autofocus: warning. focus found at edge of range. Likely focus not found or not in range.",true);
	}
	double bestZ=start+zSpeed*(bestInd*realexp+0.5*realexp);//center-range/2+range*(bestInd+1)/steps-dz/2;//zVals[bestInd];
	double bestZ2=(finish-realexp*zSpeed)-zSpeed*(steps-bestInd-0.5)*realexp;
	double realex=t.getTime()/steps/1000.0;
	double bestZ3=start+zSpeed*(bestInd*realex+0.5*realex);
	double bestZ4=(finish-realex*zSpeed)-zSpeed*(steps-bestInd-0.5)*realex;
	logFile.write("Autofocus: found position "+ toString(bestZ)+"um",DEBUGFOCUS," with a center of "+toString((double)center)+"um and a range of "+toString((double)range)+"um. Best Index: "+toString((int)bestInd)+" Start: "+toString((double)start)+" End: "+toString((double)finish)+" Percent Change Compared to Range: "+toString(100.0*(bestZ-center)/(range/2),true)+"%");
	logFile.write("Autofocus: Speed: "+toString(zSpeed)+" BestZ2: "+toString(bestZ2)+" BestZ3: "+toString(bestZ3)+" BestZ4: "+toString(bestZ4),DEBUGFOCUS);
	logFile.write("Camera Step Time: "+toString((double)realexp,5)+" Software Time: "+toString(realex,5),DEBUGFOCUS);
	logFile.write("Camera took: "+toString(t.getTime(),5),false);
	logFile.write("Trigger took: "+toString(t2.getTime(),5),false);
	return bestZ;
}


double Focus::getStepwiseFocus(bool& isInRange,double range,double step,double center){
	isInRange=true;
	int steps=(0.9999+(range/step));//round up
	range=step*steps;
		//we would like focus to potentially find the 'center' position as the focus->  This will allow for this
	double start;
	if ((steps%2)==1) start=(center-range/2+step/2);//odd number of steps
	else start=(center-range/2);//even number of steps
	cont.focus->move(start);
	cont.focus->wait();
	double minTrig=a.out->cam->startStepwiseFocusSeries(steps,a.ap);
	Timer t;
	double current=start;
	a.on();
	//a.out->cam->trigger();
	for(int i=0;i<steps-1;i++){
		t.resetTimer();
		a.out->cam->trigger();
		t.startTimer();
		Timer::wait(1000*a.ap.exp);
		current+=step;
		cont.focus->move(current);
		cont.focus->wait();
		t.waitTotal(1000*minTrig);
	}
	a.out->cam->trigger();
	Timer::wait(1000*a.ap.exp);
	a.off(false);
	int bestInd=a.out->cam->getBestFocus();
	if (bestInd==0 || bestInd==(steps-1)){
		isInRange=false;
		logFile.write("Autofocus: warning. focus found at edge of range. Likely focus not found or not in range.",true);
	}
	double bestZ=start+step*bestInd;
	logFile.write(string("Autofocus found position ")+toString(bestZ)+" Start was "+toString(start)+" End was "+toString(current),true);
	return bestZ;
}

void Focus::adjustIntensity(double percentSaturation){a.out->cam->adjustIntensity(a,percentSaturation);}


double Focus::getFocus(bool&isInRange,double range, double step, double center, int mode){
	switch(mode){
		case 0://continuous focus
			return getContinuousFocus(isInRange,range,step,center);
			break;
		case 1://stepwise focus
			return getStepwiseFocus(isInRange,range,step,center);
			break;
		case 2://slow stepwisefocus
			return getDefiniteFocus(isInRange, range, step,center);
			break;
		default:
			logFile.write("Unsupported focusing mode",true);
			isInRange=false;
			if (center==-1) return cont.focus->getZ();
			else return center;
			break;
	}
}

double Focus::getFocus(bool moveAfterFocus, double center,bool wait,int mode){
	bool b;
	//use mode 0 which is continuous
	return getFocus(moveAfterFocus,center,wait,mode,b);
}

double Focus::getFocus(bool moveAfterFocus,double center,bool wait, int mode, bool& isInRange){
	double ret;
	double best;
	bool liveView=false;
	if (this->a.out->cam->isLiveView()){
		liveView=true;
		this->a.out->cam->stopLiveView();
	}
	//error checking
	if (range<=0){
		logFile.write("Called getFocus with range less than or equal to zero. Returning immediately", DEBUGFOCUS);
		if (liveView){
			a.out->cam->startLiveView();
			cont.currentChannel().on();
		}
		return cont.focus->getZ();
	}
	if (range>COARSERANGE){
		logFile.write("Range is too large. Adjusting to maximum "+::toString(COARSERANGE),DEBUGFOCUS);
		range=COARSERANGE;
		return getFocus(moveAfterFocus,center,wait,mode,isInRange);
	}
	if (range<a.m->obj.getDOF()){
		logFile.write("Range is too small. Adjusting to minimum "+::toString(2*a.m->obj.getDOF()),DEBUGFOCUS);
		range=2.0*a.m->obj.getDOF();
		return getFocus(moveAfterFocus,center,wait,mode,isInRange);
	}
	if (step>a.m->obj.getDOF()*COARSEDOFMULTIPLES){
		logFile.write("Step is too large. Adjusting to maximum "+::toString(a.m->obj.getDOF()*COARSEDOFMULTIPLES),DEBUGFOCUS);
		step=a.m->obj.getDOF()*COARSEDOFMULTIPLES;
		return getFocus(moveAfterFocus,center,wait,mode,isInRange);
	}
	if (step<a.m->obj.getDOF()/10.0){
		logFile.write("Step is too small.  Adjusting to minimum "+::toString(a.m->obj.getDOF()/10.0),DEBUGFOCUS);
		step=a.m->obj.getDOF()/10.0;
		return getFocus(moveAfterFocus,center,wait,mode,isInRange);
	}
	z->wait();
	if (center==-1){
		center=z->getZ();
		if (center==0){
			z->wait();
			center=z->getZ();
		}
	}
	/*if (center<z->getMaxZ()-a.m->obj.getWorkingDist()/4-COARSERANGE/2){
		logFile.write("Center is below search region. Performing initial focus.",DEBUGFOCUS);
		return getInitialFocus(moveAfterFocus,wait);
	}*/
	//end error checking

	if ((range*step+2.0*square(a.m->obj.getDOF()*COARSEDOFMULTIPLES))<(a.m->obj.getDOF()*COARSEDOFMULTIPLES*range)){
		//we need to do a coarse and then a fine autofocusing
		double stepTemp=a.m->obj.getDOF()*COARSEDOFMULTIPLES;
		logFile.write("Performing Coarse Focusing",DEBUGFOCUS);
		logFile.write("LowerBound: "+toString(center-range/2)+" Upper Bound: "+toString(center+range/2)+" step "+toString(stepTemp)+"um",DEBUGFOCUS);
		bool b1;
		double centerTemp=getFocus(b1,range,stepTemp,center,mode);
		logFile.write("Performing Fine Focusing",DEBUGFOCUS);
		logFile.write("LowerBound: "+toString(centerTemp-(3*a.m->obj.getDOF()*COARSEDOFMULTIPLES)/2)+" Upper Bound: "+toString(centerTemp+(3*a.m->obj.getDOF()*COARSEDOFMULTIPLES)/2)+" step "+toString(step)+"um",DEBUGFOCUS);
		ret=getFocus(isInRange,3*a.m->obj.getDOF()*COARSEDOFMULTIPLES,step,centerTemp,mode);
		if (!b1) isInRange=false;
	}else {
		logFile.write("Performing ONLY Fine Focusing",DEBUGFOCUS);
	//fine autofocusing
		ret=getFocus(isInRange,range,step,center,mode);
	}
	if (moveAfterFocus) {
		z->move(ret);
	}
	if (wait) z->wait();
	//if (isInRange) cont.displays.closeAll();
	//else{
	//	system("pause");
	//	cont.displays.closeAll();
	//}
	if (liveView){
		a.out->cam->startLiveView();
		cont.currentChannel().on();
	}
	return ret;
}




double Focus::getInitialFocus(bool moveAfterFocus,bool wait){
	double tempRange=range;
	range=COARSERANGE;
	cout<<z->getMaxZ()-a.m->obj.getWorkingDist()/4-COARSERANGE/2<<endl;
	double ret=getFocus(moveAfterFocus,z->getMaxZ()-a.m->obj.getWorkingDist()/4-COARSERANGE/2,wait);		
	range=tempRange;
	return ret;
}


void Focus::modify(){
	char cTemp;
	double d;
	while(true){
		cout<<"Please confirm focus settings"<<endl;
		if (this->a!=cont.currentChannel())
			cout<<"0: Use current acquisition channel"<<cont.currentChannel().toString()<<endl;
		cout<<"1: Change acquisition channel ("<<a.toString()<<")"<<endl;
		cout<<"2: Change range               ("<<range<<"um)"<<endl;
		cout<<"3: Change step size           ("<<step<<"um)"<<endl;
		cout<<"a: accept current settings"<<endl;
		cin>>cTemp;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(cTemp){
			case '0':
				this->a=cont.currentChannel();
				break;
			case '1':{
				Magnification* m=a.m;
				a.modify();
				if (m!=a.m)
					step=a.m->obj.getDOF()/2;
				break;}
			case '2':
				cout<<"Enter desired range in microns"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				range=d;
				break;
			case '3':
				cout<<"Enter desired step size in microns"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				step=d;
				break;
			case 'a':
				return;
				break;
		}
	}

}


/*
double Controller::autoFocus( int channel, int steps,double range, float exp, double center, int gain){
	//cam->debug=true;
	cam->waitIdle();
	cam->focusNum=steps;
	ResetEvent(cam->newImage);
	if (center==-1){
		center=focus->getPos();
	}
	double dz=range/steps;
	//double* zVals=new double [steps];
	int oldBin=cam->getBin();
	float oldExp=cam->getExposure();
	int oldGain=cam->getGain();
	if (gain==-1) gain=cam->getGain();
	if (exp==-1) exp=cam->getExposure();
	cam->setGain(gain);
	cam->setExposure(exp);
	cam->setBin(1);
	cam->setImageRegion(0,25,25);
	cam->bin=oldBin;
	cam->exp=oldExp;
	cam->gain=oldGain;
	//double start,end;
	double begin,finish,test;
	unsigned threadID;
	HANDLE thread=(HANDLE) _beginthreadex(NULL,0,
		Camera::FocusThread,
		this->cam,
		0,
		&threadID);
	//we would like focus to potentially find the same position.  this will allow for this
	if ((steps%2)==1) focus->move(center-range/2);//odd number of steps
	else focus->move(center-range/2-dz/2);//even number of steps
	Timer t;
	float realexp=cam->startFTKinetics(steps,exp,-1,gain);
	double zspeed=dz/realexp;//required speed in micrometers/second
	prepareChannel(channel);
	focus->go(zspeed);
	filt->triggerCam();
	t.startTimer();
	begin=focus->getStartPosition();
	while(!cam->isIdle());//WaitForSingleObject(thread,INFINITE);	
	focus->stop();
	t.stopTimer();
	finish=focus->getPos();
	if (finish<begin){
		cout<<"microscope reports negative move"<<endl;
		finish=focus->getPos();
	}
	lightOff();
	//cout<<"Total Time: "<<t.getTime()<<"  SPEED: "<<zspeed<<"um/sec  DISTANCE: "<<finish-begin<<"  START: "<<begin<<endl;//" test focus position: "<<test<<endl;
	//cout<<"Exposure time: "<<realexp<<endl;
	WaitForSingleObject(thread,INFINITE);
	DWORD bestInd;
	GetExitCodeThread(thread,&bestInd);
	cam->setImageRegion(0,100,100);
//	Thread::removeThread(th);
	
	if (bestInd==0 || bestInd==(steps-1)){
		logFile.write("Autofocus: warning. focus found at edge of range. Likely focus not found or not in range.",DEBUGCONTROLLER);
	}
	double bestZ=begin+zspeed*(bestInd*realexp+0.5*realexp);//center-range/2+range*(bestInd+1)/steps-dz/2;//zVals[bestInd];
	double bestZ2=(finish-realexp*zspeed)-zspeed*(steps-bestInd-0.5)*realexp;
	double realex=t.getTime()/steps/1000.0;
	double bestZ3=begin+zspeed*(bestInd*realex+0.5*realex);
	double bestZ4=(finish-realex*zspeed)-zspeed*(steps-bestInd-0.5)*realex;
	logFile.write("Autofocus: found position "+ toString(bestZ)+"um",DEBUGCONTROLLER," with a center of "+toString((double)center)+"um and a range of "+toString((double)range)+"um. Best Index: "+toString((int)bestInd)+" Start: "+toString((double)begin)+" End: "+toString((double)finish)+" Percent Change Compared to Range: "+toString(100.0*(bestZ-center)/(range/2),true)+"%");
	logFile.write("Autofocus: BestZ2: "+toString(bestZ2)+" BestZ3: "+toString(bestZ3)+" BestZ4: "+toString(bestZ4),DEBUGCONTROLLER);
	logFile.write("Camera Step Time: "+toString((double)realexp,false,5)+" Sofware Time: "+toString(realex,false,5),DEBUGCONTROLLER);
	return bestZ;
}
*/
