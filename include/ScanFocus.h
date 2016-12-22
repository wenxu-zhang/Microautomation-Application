// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include <vector>
//#include "AcquisitionSeries.h"
//#include "ImageProperties.h"
#include "focus.h"
#include "nr.h"

class ScanFocus{
protected:
	ScanFocus(const Focus& f);
public:
	//member variables
	Focus f;

	//real methods
	
	~ScanFocus(){}
	//virtual methods
	virtual void initialFocus(){}//compute approximation for scanning
	virtual void updateFocus(bool wait=false){}
	//virtual void improveFocus();not necessary?
	virtual double getFocus(int x,int y){return 0;}//get approximated focus
	virtual void wait(){}//wait til focusing is complete
};

class FocusInTemp{	//This class records previous focus parameters and finds an approximate focus under temperature change
public:
	FocusInTemp();
	FocusInTemp(Focus f,int x,int y,double z, double T);
	~FocusInTemp(){}
	double getFocusInTemp(double temp);
	double getFocusInTemp();
	int getX();
	int getY();
	void setZeroTempFocus();
	void setZeroTempFocus(double zfocus, double iniTemp);
	double z;
	Focus f;
private:
	int x,y;
	static const double slope;
	double zeroTempFocus;
};
