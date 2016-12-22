// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Objective.h"

bool Objective::cleanOil=false;

Objective::Objective(double mag,double dof, double wd,bool isOil, Changer* c, int position, std::string name)
:s(Selection(c,position,name)),mag(mag),dof(dof),wd(wd),isOil(isOil){
		if (isOil) needsOil=true;
		else needsOil=false;
}

double Objective::getDOF() const{
	return dof;
}	

double Objective::getWorkingDist() const{
	return wd;
}

void Objective::set(){
	if (s.c->get()==s.position && !needsOil) return;
	if (cleanOil) {
		std::cout<<"Please clean oil from objective and specimen"<<std::endl;
		system("pause");
		cleanOil=false;
	}
	s.set();
	if (isOil) {
		needsOil=true;
		wait();
	}
		
}

void Objective::wait(){
if (needsOil) {
	std::cout<<"Please add oil to objective"<<std::endl;
			system("pause");
			cleanOil=true;
			needsOil=false;
		}
		s.c->wait();

}

std::string Objective::toString(){
	return s.name;
}
