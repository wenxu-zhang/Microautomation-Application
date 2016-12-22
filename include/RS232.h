// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, July 10, 2012</lastedit>
// ===================================
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "time.h"
#include "Utils.h"
#include <string>

class RS232{
public:
	RS232(int comnum,int baud,bool flowControl=true,int readTimeout=1000,int stopbits=1);
	~RS232();
	bool isPresent;
	std::string read(int num);

	std::string read(std::string endSeq);
	void closecom(void);
	void write(std::string s);
	void dumpRx();
	int getNumReadBytes();
	int opencom(bool flowControl=true,int readTimeout=1000,int stopbits=1);
private:
	char strbuf[256];
	int comnum;
	int baud;
	HANDLE hCommAccess,hCom;
	
	
};