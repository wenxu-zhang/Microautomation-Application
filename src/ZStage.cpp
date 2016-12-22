// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "ZStage.h"
#include <iostream>
#include <limits>
#include "Chamber.h"
//#include "Controller.h"
using namespace std;
#define MAXZFOCUS  9500
extern Chamber currentChamber;
//extern Controller cont;
double ZStage::getMaxZ(){
	return MAXZFOCUS;
}
void ZStage::focusControl(){
	char c;	
	double z;
	while(true){
		cout<<"Welcome to Focus Control\n";
		cout<<"Please select a task"<<endl;
		cout<<"1: Get focus position"<<endl;
		cout<<"2: Move focus to given position"<<endl;
		cout<<"3: Set maximum Z (currently at "<<getMaxZ()<<").\n";
		if (currentChamber.maxZ!=0 && currentChamber.focusZ!=0) 
			cout<<"4: Move to default chamber focus ("<<currentChamber.focusZ<<")"<<endl;
		cout<<"e: Exit Focus Control"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':
				cout<<"Focus is currently at "<<getZ()<<".\n";
				break;
			case '2':
				cout<<"Please enter z position to move focus to:\n";
				cin>>z;
				move(z);
				break;
			case '3':
				cout<<"Please enter the maximum Z:\n";
				cin>>z;
				currentChamber.maxZ=z;
				setMaxZ(z);
				break;
			
			case 'e':
				return;
				break;
			case '4':
				if (currentChamber.maxZ!=0 && currentChamber.focusZ!=0){
					setMaxZ(currentChamber.maxZ);
					move(currentChamber.focusZ);
					break;
				}
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
		}
	}
	
}
