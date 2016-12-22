// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Scanfocus.h"

class GridFocus:public ScanFocus {
public:
	GridFocus(const Focus& f, int minX,int minY,int maxX,int maxY, int numX, int numY);
	GridFocus(const Focus& f, int minX,int minY,int maxX,int maxY, double umStep);
	GridFocus();
	//GridFocus(int minX,int minY,int maxX,int maxY, double maxStdDev);
	void initialFocus();
	//void improveFocus();
	void updateFocus();
	double getFocus(int xpos,int ypos);

private:


	int minX,minY,maxX,maxY, numX,numY;
	NRMat<DP> *z;
	NRVec<DP> *y;	//y coordinate for surface approx
	NRVec<DP> *x;	//x coordinate for surface approx
	NRMat<DP> *secondDer;
};