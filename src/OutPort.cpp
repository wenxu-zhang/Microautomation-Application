// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "OutPort.h"
#include "Controller.h"
#include "Utils.h"
#include <limits>

extern Controller cont;

OutPort::OutPort(Selection& s,Camera* cam):s(s),cam(cam){}

void OutPort::set(){s.set();}

void OutPort::wait(){s.wait();}

std::string OutPort::toString() const{
	return s.name;
}

OutPort* OutPort::select(){
	cout<<"Please select an Out Port"<<endl;
	int ind=0;
	for(vector<OutPort>::const_iterator i=cont.outPorts.begin();i!=cont.outPorts.end();i++){
		cout<<::toString(ind,2)<<": "<<i->toString()<<endl;
		ind++;
	}
	cin>>ind;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (ind<0 || ind>=cont.outPorts.size()) return select();
	return &(cont.outPorts[ind]);
}