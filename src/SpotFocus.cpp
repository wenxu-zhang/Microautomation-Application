// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, June 16, 2011</lastedit>
// ===================================
#include "SpotFocus.h"
#include "Controller.h"
#include <limits>
#include "XYStage.h"
extern Controller cont;
extern Record logFile;

SpotFocus::SpotFocus():ScanFocus(cont.currentFocus){}
SpotFocus::SpotFocus(Focus foc):ScanFocus(foc){
	f.modify();
	int c=0;
	cout<<"Please move to the desired position for focusing.\n";
	f.a.on();
	f.a.out->cam->startLiveView();
	while(c==0){
		cout<<"After exiting stageControl, press 1 to accept position and 0 to select again.\n";
		cont.stg->stageControl();
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if(c==1)break;
	}
	f.a.out->cam->stopLiveView();
	pos=FocusInTemp(f,cont.stg->getX(),cont.stg->getY(),f.getFocus(true,-1,true),cont.te.getTemp());
	f.a.off();
	logFile.write("SpotFocus: ("+toString(pos.getX())+", "+toString(pos.getY())+") accepted",true);
}

SpotFocus::~SpotFocus(void)
{
}
double SpotFocus::getFocus(int x,int y){
	return pos.z;
}//get approximated focus
void SpotFocus::updateFocus(){
	cont.stg->move(pos.getX(),pos.getY());
	cont.stg->wait();
	double temp=cont.te.getTemp();
	pos.z=f.getFocus(true,pos.getFocusInTemp(temp),true);
	pos.setZeroTempFocus(pos.z,temp);
}
