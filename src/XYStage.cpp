// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, April 11, 2011</lastedit>
// ===================================
#include "XYStage.h"
#include <iostream>
#include <limits>
#include "Controller.h"
#include "ZStage.h"
#include "Chamber.h"
using namespace std;
extern Controller cont;
extern Chamber currentChamber;
void XYStage::stageControl(){
	char c;	
	int x,y;
	double xMicron,yMicron;
	int chamber, channel, section;
	if (cont.currentChannel().chan && cont.currentChannel().m->obj.isOil)
		setSpeed(OILVELOCITY);
	
	while(true){
		cout<<"Welcome to Stage Control\n";
		cout<<"Please select a task"<<endl;
		cout<<"s: Enable servo mode"<<endl;
		cout<<"0: Disable servo mode"<<endl;
		cout<<"1: Get stage position"<<endl;
		cout<<"2: Move stage to given position"<<endl;
		cout<<"3: Move stage in x direction (Steps)"<<endl;
		cout<<"4: Move stage in y direction (Steps)"<<endl;
		cout<<"5: Fine stage control: using arrow keys"<<endl;
		cout<<"6: FOV up"<<endl;
		cout<<"7: FOV down"<<endl;
		cout<<"8: FOV left"<<endl;
		cout<<"9: FOV right"<<endl;
		cout<<"X: Move stage in x direction (microns)"<<endl;
		cout<<"Y: Move stage in y direction (microns)"<<endl;
		cout<<"C: Chamber Move (go to specified region of a chip)"<<endl;
		cout<<"Z: Zero this position"<<endl;
		cout<<"H: Home Stage"<<endl;
		cout<<"T: Test Stage Homing"<<endl;
		cout<<"e: Exit Stage Control"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case 's':
				enableServo();
				break;
			case '0':
				disableServo();
				break;
			case '1':
				cout<<"Stage is currently at ("<<getX()<<", "<<getY()<<").\n";
				break;
			case '2':
				cout<<"Please enter x position to move stage to:\n";
				cin>>x;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Please enter y position to move stage to:\n";
				cin>>y;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				move(x,y);
				break;
			case '3':
				cout<<"Please enter the distance to move in x direction:\n";
				cin>>x;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				incX(x);
				break;
			case '4':
				cout<<"Please enter the distance to move in y direction:\n";
				cin>>y;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				incY(y);
				break;
			case '5':
				fineControl();
				break;
			case '6':
				setYstep(cont.currentChannel().getFOVHeight()/getStepSize());
				stepUp();
				break;
			case '7':
				setYstep(cont.currentChannel().getFOVHeight()/getStepSize());
				stepDown();
				break;
			case '8':
				setXstep(cont.currentChannel().getFOVWidth()/getStepSize());
				stepLeft();
				break;
			case '9':
				setXstep(cont.currentChannel().getFOVWidth()/getStepSize());
				stepRight();
				break;
			case 'X':
				cout<<"Please enter the distance (microns) to move in x direction:\n";
				cin>>xMicron;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				incX(xMicron/this->getStepSize());
				break;
			case 'Y':
				cout<<"Please enter the distance (microns) to move in y direction:\n";
				cin>>yMicron;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				incY(yMicron/this->getStepSize());
				break;
			case 'C':{
				cout<<"Please enter the desired channel number (1-"<<currentChamber.numChannels<<")";
				int chan=getInt();
				cout<<"Please enter the desired fractional position along the length of the channel (0-1)"<<endl;
				double len=getDouble();
				cout<<"Please enter the desired fractional position along the width of the channel (0-1)"<<endl;
				double wid=getDouble();
				if (cont.axio.getCurrentObjective()->isOil)
					cont.stg->setSpeed(OILVELOCITY);
					currentChamber.move(chan,wid,len);
					cont.stg->setSpeed(MAXVELOCITY);
				break;
				}
			case 'Z':
				setPosition(0,0);
				break;
			case 'H':
				homeStage();
				break;
			case 'T':{
				cont.focus->move(0);
				cont.focus->wait();
				int x,y;
				home(x,y);
				setPosition(0,0);
				move(0,0);
				wait();
				Record ludl("E:\\LudlHomeTestNoZero.txt");
				for(int i=1;i<101;i++){
					home(x,y);
					ludl.write(toString(i)+") Position (x y) is: \t"+toString(this->getX())+"\t"+toString(this->getY()));
					move(0,0);
					wait();
				}
				Record ludl2("E:\\LudlHomeTestZero.txt");
				for(int i=1;i<101;i++){
					home(x,y);
					setPosition(0,0);
					ludl2.write(toString(i)+") Position (x y) is: \t"+toString(this->getX())+"\t"+toString(this->getY()));
					move(0,0);
					wait();
				}
					 }break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
	setSpeed(MAXVELOCITY);
}


//-1 is no chamber 0 is left 1 is right
int XYStage::getChamber(int currentX, int currentY){
	if (currentX>=-chamberWidth/2 && currentX<=chamberWidth/2 &&
		currentY>=-chamberHeight/2 && currentY<=chamberHeight/2)
		//in left chamber
		return 0;
	else if 
		(currentX>=-chamberWidth/2-chamberOffset && currentX<=chamberWidth/2-chamberOffset &&
		currentY>=-chamberHeight/2 && currentY<=chamberHeight/2)
		//in right chamber
		return 1;
	else{
		//not in any chamber
		return -1;	
	}
}

void XYStage::enableLimits(bool isLeftChamber){
	double stepSize=getStepSize();
	if (isLeftChamber){
		enableLimits(-chamberWidth/2-LIMITCUSHION/stepSize,-chamberHeight/2-LIMITCUSHION/stepSize,chamberWidth/2+LIMITCUSHION/stepSize,chamberHeight/2+LIMITCUSHION/stepSize);
	}else{
		enableLimits(-chamberWidth/2-chamberOffset-LIMITCUSHION/stepSize,-chamberHeight/2-LIMITCUSHION/stepSize,chamberWidth/2-chamberOffset+LIMITCUSHION/stepSize,chamberHeight/2+LIMITCUSHION/stepSize);
	}
}

void XYStage::move(int x,int y){
#ifdef XYLIMITS
	double z;
	int currentX=getX();
	int currentY=getY();
	int currentChamber=getChamber(currentX,currentY);
	int newChamber=getChamber(x,y);
	if (newChamber==-1){//cannot move here
		logFile.write("Error: XYStage move must be to a position within a chamber",true);
	}else if (newChamber==currentChamber){//no need to move z
		moveX(x);moveY(y);	
	}else{//must protect objective during move
		disableLimits();
		z=cont.focus->getZ();
		cont.focus->move(0);
		cont.focus->wait();
		moveX(x);moveY(y);
		wait();
		cont.focus->move(z);
		if (newChamber==0)
			enableLimits(true);
		else
			enableLimits(false);
	}
#else
	moveX(x);moveY(y);
#endif
}


bool XYStage::homeStage(bool setZero, bool moveBack){
	if (!cont.focus->doesExist()){
		logFile.write("Cannot home stage when Z drive is not present",true);
		return false;
	}
	double z=cont.focus->getZ();
	cont.focus->move(0);
	cont.focus->wait();
	int	startX=this->getX();
	int startY=this->getY();
	cout<<"Ensure nothing is blocking stage movement to limit switches"<<endl;
	system("pause");
	int x,y;
	if (home(x,y))
		logFile.write("Stage Homing found position ("+toString(x)+","+toString(y)+")",true);
	else{
		logFile.write("Stage Homing failed. Objective will remain in load position",true);
		return false;
	}
	if (setZero) {
		logFile.write("Setting home position to (0,0)",true);
		move(x,y);
		wait();
		setPosition(0,0);//stage center position should be (0,0)
	}

	if (moveBack)
		if (setZero){
			moveX(startX-x);
			moveY(startY-y);
		}else{
			moveX(startX);
			moveY(startY);
		}
	wait(30000);
	cont.focus->move(z);
	cont.focus->wait();
	return true;
}


//this moves the stage to the start of a section of the device
	//chamber: 0 is left 1 is right
	//channel: 0 is left 1 is middle 2 is right
	//section: 0 is bottom 1 is middle 2 is top 3 is current
void XYStage::chamberMove(int chamber,int channel,int section){
#ifdef XYLIMITS
	int x=0, y=0;
	switch(channel){
			case 0:
				x=channelOffset;
				break;
			case 1:
				x=0;
				break;
			case 2:
				x=-channelOffset;
				break;
			case 3:
				x=cont.stg->getX();

			default:
				logFile.write(string("Error: unsupported channel number ")+toString(channel),true);
				return;
	}
	switch (section){
			case 0:
				y=-chamberHeight/2;
				break;
			case 1:
				y=-chamberHeight/6;
				break;
			case 2:
				y=chamberHeight/6;
				break;
			default:
				logFile.write(string("Error: unsupported section number ")+toString(section),true);
				return;

	}
	switch (chamber){
			case 0://left
				break;
			case 1://right
				x-=chamberOffset;
				break;
			default:
				logFile.write(string("Error: unsupported chamber number ")+toString(chamber),true);
				return;
	}
	move(x,y);

#else
	logFile.write(string("Error: channel move not supported when XYLimits are not enabled"),true);
#endif
}
