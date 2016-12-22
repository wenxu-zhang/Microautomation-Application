// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Controller.h"
#include "ImageProperties.h"
extern Controller cont;
ImageProperties::ImageProperties(AcquisitionChannel ac, string name,string comment,int x,int y,double z,double temp,double temp2,string time,unsigned int max)
:ac(ac),name(name),x(x),y(y),temp(temp),temp2(temp2),time(time),comment(comment),max(max)
{}
void ImageProperties::Update(AcquisitionChannel ac, string name,string comment,int x,int y,double z,double temp,double temp2,string time,unsigned int max){
	this->ac=ac;
	this->name=name;
	this->comment=comment;
	this->x=x;
	this->y=y;
	this->z=z;
	this->temp=temp;
	this->temp2=temp2;
	this->time=time;
	this->max=max;
}
	