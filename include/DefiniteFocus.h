// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#ifdef OBSERVER
#pragma once
#include "ScanFocus.h"
#include "Scan.h"

class DefiniteFocus :
	public ScanFocus
{
public:
	DefiniteFocus(bool isPresent=true);
	~DefiniteFocus(void);
	void initialFocus();//compute approximation for scanning
	void updateFocus(bool wait=false);
	//void improveFocus();not necessary?
	double getFocus(int x,int y);//get approximated focus
	void wait();
	bool isPresent;
private:
	//Scan *scan;
	ObserverDefiniteFocus* DFS;
};
#endif