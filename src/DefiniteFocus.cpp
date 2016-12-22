// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#ifdef OBSERVER
#include "DefiniteFocus.h"
extern Controller cont;
extern Record logFile;
extern int protocolIndex;
DefiniteFocus::DefiniteFocus(bool isPresent):ScanFocus(cont.currentFocus),DFS(&(cont.df)),isPresent(isPresent)
{
}

DefiniteFocus::~DefiniteFocus(void)
{
	CheckExists()
	DFS->stopDefiniteFocus();
}
void DefiniteFocus::initialFocus(){
}
void DefiniteFocus::updateFocus(bool wait){
	CheckExists()
	DFS->getDefiniteFocus(protocolIndex,wait);
}

void DefiniteFocus::wait(){
	CheckExists()
	DFS->wait();
}

double DefiniteFocus::getFocus(int x,int y){
	CheckExists(cont.focus->getZ())
	if(!DFS->isOn()){
		logFile.write("Error: Definite Focus is not stabilizing. Focus may be incorrect.");
	}
	return cont.focus->getZ();
}
#endif
