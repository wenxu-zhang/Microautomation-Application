// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Optovar.h"

Optovar::Optovar(double mag,Changer* c, int position, std::string name):s(Selection(c,position,name)),mag(mag){}

void Optovar::set() {
	s.set();
}

void Optovar::wait() {
	s.wait();
}
bool Optovar::operator==(Optovar right){
	return s==right.s && mag==right.mag;
}