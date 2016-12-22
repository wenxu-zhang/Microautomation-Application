// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, June 20, 2011</lastedit>
// ===================================
#include "Record.h"
#include "Controller.h"

extern Controller cont;


extern std::string user;

Record::Record(std::string fileName,int lineWidth, Timer* swatch):lineWidth(lineWidth){
	if (!swatch) this->swatch=new Timer(true); 
	else this->swatch=swatch; 
	open(fileName);
}

void Record::clear(){
	close();
	DeleteFile(fileName.c_str());
	open(fileName);
}

void Record::write(std::string comment, bool stdOut,std::string extra,bool dispTime){
	std::string time=toString(*swatch);
	if (!dispTime) time="";
	std::string tab("     ");//not really a tab but it is consistent
	std::string total=comment+tab+extra;
	int index,minwidth;
	if (lineWidth==0){
		recordFile<<time<<"\t"<<comment<<"\t"<<extra<<"\n"<<flush;
	}else if (total.length()+tab.length()+time.length() < lineWidth){
		minwidth=lineWidth-total.length()-tab.length();
		recordFile<<total<<tab<<setw(minwidth)<<time<<endl<<flush;
	}else{
		index=max(total.find_last_of(' ',lineWidth-tab.length()-time.length()),total.find_last_of('\n',lineWidth-tab.length()-time.length()));

		if (index==-1) {
			recordFile<<total<<flush;
		}else{
		recordFile<<total.substr(0,index)<<endl<<tab<<flush;
		total=total.substr(index,total.length()-1);
		while (total.length()+2*tab.length()+time.length() > lineWidth){
			index=total.find_last_of(' ',lineWidth-2*tab.length()-time.length());
			if (index==-1)
				index=lineWidth-2*tab.length()-time.length();
			recordFile<<total.substr(0,index)<<endl<<tab<<flush;
			total=total.substr(index,total.length()-1);
		}
		minwidth=lineWidth-total.length()-tab.length();
		recordFile<<total<<setw(minwidth)<<time<<endl<<flush;	
		}
	}
	if (stdOut){
		cout<<comment<<endl<<flush;
	}
}

void Record::open(std::string fileName){
	this->fileName=fileName;
	std::string dir=fileName.substr(0,fileName.find_last_of('\\',fileName.length()-1));
	long int err=SHCreateDirectoryEx(NULL,dir.c_str(),NULL);
	if (err!=ERROR_SUCCESS && err!=ERROR_FILE_EXISTS && err!=ERROR_ALREADY_EXISTS) {
		cout<<"Record:  error: Could not create directory "<<dir<<endl;
		return;
	}
	close();
	recordFile.open(fileName.c_str(),ios::app);
	if (!recordFile.is_open()) cout<<"Record: error: could not open record file "+fileName<<endl;{
		swatch->resetTimer();
		swatch->startTimer();
		
		write(std::string("File Opened by ")+user+" On : "+Timer::getSysTime(),DEBUGRECORD);
	}
}

void Record::close(){
	if (recordFile.is_open()){
		write("File Closed On : "+Timer::getSysTime(),DEBUGRECORD);
		recordFile<<endl<<endl;
		recordFile.close();
	}
}