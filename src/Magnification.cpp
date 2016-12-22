// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Magnification.h"
#include "Controller.h"
#include "Utils.h"
#include <limits>
extern Controller cont;

bool Magnification::operator==(Magnification &right){
	if (obj.mag==right.obj.mag && opt.mag==right.opt.mag)
		return true;
	else return false;
}

Magnification::Magnification(Objective& obj,Optovar& opt):opt(opt),obj(obj){};

double Magnification::get() const{return obj.mag*opt.mag;}

void Magnification::set() {opt.set();obj.set();}

void Magnification::wait() {opt.wait();obj.wait();}

std::string Magnification::toString() const{
	string ret;
	if (obj.isOil) ret=::toString(this->obj.mag,0)+"xOil";
	else ret=::toString(this->obj.mag, 0)+"x";
	if (this->opt.mag!=1)
		ret=ret+"_opt"+::toString(this->opt.mag,1)+"x";
	return ret;
}


Objective* Magnification::getObjective(string str){
	string s;
	for(vector<Objective*>::iterator i=cont.axio.objectives.begin();i!=cont.axio.objectives.end();i++){
		if ((*i)->isOil)
			s=::toString((*i)->mag,0)+"xOil";
		else
			s=::toString((*i)->mag,0)+"x";
		if (s==str)
			return *i;
	}
	return NULL;
}


Magnification* Magnification::select(){
	cout<<"Please select a magnification"<<endl;
	int ind=1;
	for(vector<Objective*>::iterator i=cont.axio.objectives.begin();i!=cont.axio.objectives.end();i++){
		if ((*i)->isOil)
			cout<<::toString(ind)<<": "<<::toString((*i)->mag,0)+"xOil"<<endl;
		else
			cout<<::toString(ind)<<": "<<::toString((*i)->mag,0)+"x"<<endl;
		ind++;
	}
	cout<<"V: Also change optovar"<<endl;
	string s=getString();
	if (s=="V"){
		cout<<endl<<"Please select a magnification"<<endl;
		int ind=1;
		vector<string> displays1;
		vector<string> displays2;
		for(vector<Magnification>::const_iterator i=cont.mags.begin();i!=cont.mags.end();i++){
			displays1.push_back(::toString(ind,2)+": "+i->toString());
			displays2.push_back("  ="+::toString(i->get())+"x");
			ind++;
		}
		int max=0;
		for(vector<string>::iterator i=displays1.begin();i!=displays1.end();i++){
			if (i->size()>max)
				max=i->size();
		}
		int j=0;
		for(vector<string>::iterator i=displays1.begin();i!=displays1.end();i++){
			cout<<*i<<string(max-i->size(),' ')<<displays2.at(j)<<endl;
			j++;
		}
		ind=getInt();
		if (ind<1 || ind>cont.mags.size()){
			cout<<"Invalid selection...try again"<<endl;
			return select();
		}
		return &(cont.mags[ind-1]);
	}
	ind=toInt(s);
	int sz=cont.axio.objectives.size();
	if (ind<1 || ind>sz){
		cout<<"Invalid selection...try again"<<endl;
		return select();
	}
	return getMagnification(*(cont.axio.objectives.at(ind-1)),cont.currentChannel().m->opt);
}


Magnification* Magnification::getMagnification(Objective& obj,Optovar& opt){
	for(vector<Magnification>::iterator i=cont.mags.begin();i!=cont.mags.end();i++){
		if (&(i->obj)==&obj && i->opt==opt)
			return &(*i);
	}
	return NULL;
}

int Magnification::getMagnification(string str){
	int ind=0;
	for(vector<Magnification>::iterator i=cont.mags.begin();i!=cont.mags.end();i++){
		if (i->toString()==str)
			return ind;
		ind++;
	}
	return -1;
}
