// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Trigger.h"
#include "Controller.h"
using namespace std;
extern Controller cont;

void Trigger::triggerControl(){
		char c;	
		int line;
		double volt;
	double z;
	while(true){
		cout<<"Welcome to Trigger Control\n";
		cout<<"Please select a task"<<endl;
		cout<<"1: Set Line High"<<endl;
		cout<<"2: Set Line Low"<<endl;
		cout<<"3: Set Voltage"<<endl;
		cout<<"4: Get Voltage"<<endl;
		cout<<"5: Start Non FT Kinetic with 2 images"<<endl;
		cout<<"e: Exit Focus Control"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':
				cout<<"Select digital line to set high"<<endl;
				cin>>line;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				this->setLineHigh(line);
				break;
			case '2':
				cout<<"Select digital line to set low"<<endl;
				cin>>line;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				this->setLineLow(line);
				break;
			case '3':
				cout<<"Select analog line to set voltage";
				cin>>line;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Enter desired voltage (-10V to 10V)"<<endl;
				cin>>volt;
				this->setVoltage(line,volt);
				break;
			case '4':
				cout<<"Select analog line to get voltage";
				cin>>line;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cout<<"Voltage is "<<getVoltage(line)<<endl;
				break;
			case '5':
				cont.currentChannel().out->cam->startStepwiseFocusSeries(2,AcquisitionParameters(.5,4));
				break;
			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}

}