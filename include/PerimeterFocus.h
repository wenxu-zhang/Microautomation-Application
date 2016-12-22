// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Scanfocus.h"

class PerimeterFocus:public ScanFocus{
public:
	PerimeterFocus(int numX=-1,int numY=-1);
	PerimeterFocus(const Focus& f, int minX,int minY,int maxX,int maxY, int numX, int numY);
	PerimeterFocus(const Focus& f,int minX,int minY,int maxX,int maxY,double maxStdDev);
	void initialFocus();
	void updateFocus();
	double getFocus(int xpos,int ypos);
private:
	int minX,minY,maxX,maxY,numX,numY;
	//z is split into zx and zy to save memory space, so that there are no memory allocated for points in the middle
	//convention is to store the 1st row corresponding to the smaller x/y position, and 2nd row the larger
	vector<FocusInTemp> *zx0;	//a size(x)-by-2 matrix that stores the z position along the x direction
	vector<FocusInTemp> *zx1;	//a size(x)-by-2 matrix that stores the z position along the x direction
	vector<FocusInTemp> *zy0;	//a size(y)-by-2 matrix that stores the z position along the y direction
	vector<FocusInTemp> *zy1;	//a size(y)-by-2 matrix that stores the z position along the y direction
	//NRVec<DP> *zx0;	//a size(x)-by-2 matrix that stores the z position along the x direction
	//NRVec<DP> *zx1;	//a size(x)-by-2 matrix that stores the z position along the x direction
	//NRVec<DP> *zy0;	//a size(y)-by-2 matrix that stores the z position along the y direction
	//NRVec<DP> *zy1;	//a size(y)-by-2 matrix that stores the z position along the y direction
	NRVec<DP> *x;	//x coordinate for surface approx
	NRVec<DP> *y;	//y coordinate for surface approx
	NRVec<DP> *secondDerX0;
	NRVec<DP> *secondDerX1;
	NRVec<DP> *secondDerY0;
	NRVec<DP> *secondDerY1;
};