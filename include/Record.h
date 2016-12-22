// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, June 20, 2011</lastedit>
// ===================================
#pragma once
#include "Timer.h"
#include "Utils.h"
#include <fstream>
#include <shlobj.h>
#include "Definitions.h"

class Record{
public:
	int lineWidth;
	std::ofstream recordFile;
	Timer* swatch;
	std::string fileName;
	void setFileName(std::string newFileName){
		if (!(newFileName == fileName)){
			close();
			open(fileName);
		}
	}

	Record(std::string fileName,int lineWidth=LINEWIDTH, Timer* swatch=NULL);
	~Record(){recordFile.close();}
	void write(std::string data, bool stdOut=false, std::string extra="",bool dispTime=true);
	void clear();

	void close();
	void open(std::string fileName);
};
