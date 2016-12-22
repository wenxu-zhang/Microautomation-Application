// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Scanfocus.h"
#include "Controller.h"
#include "XYStage.h"
extern Controller cont;

ScanFocus::ScanFocus(const Focus& f):f(f){}
const double FocusInTemp::slope=-0.4919;
FocusInTemp::FocusInTemp(){
	f=cont.currentFocus;
	x=cont.stg->getX();
	y=cont.stg->getY();
	z=cont.focus->getZ();
	setZeroTempFocus(z,cont.te.getTemp());
}
FocusInTemp::FocusInTemp(Focus f,int x,int y,double z,double T):x(x),y(y),z(z),f(f){
	setZeroTempFocus(z,T);
}
void FocusInTemp::setZeroTempFocus(){
	cont.stg->move(x,y);
	cont.stg->wait();
	zeroTempFocus=f.getFocus(true)-slope*cont.te.getTemp();
}
void FocusInTemp::setZeroTempFocus(double zfocus, double iniTemp){
	zeroTempFocus=zfocus-slope*iniTemp;
}
double FocusInTemp::getFocusInTemp(double temp){
	z=slope*temp+zeroTempFocus;
	return z;
}
double FocusInTemp::getFocusInTemp(){
	double temp;
	temp=cont.te.getTemp();
	return getFocusInTemp(temp);
}
int FocusInTemp::getX(){
	return x;
}
int FocusInTemp::getY(){
	return y;
}