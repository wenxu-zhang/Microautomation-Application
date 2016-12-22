// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "scanfocus.h"

class SpotFocus :
	public ScanFocus
{
public:
	SpotFocus();
	SpotFocus(Focus foc);
	~SpotFocus(void);
	void updateFocus();
	double getFocus(int x,int y);//x,y are not used. Just follows the convention.
private:
	FocusInTemp pos;
};
