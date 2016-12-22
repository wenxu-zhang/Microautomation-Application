// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, December 13, 2011</lastedit>
// ===================================
#include "Utils.h"
#include "Controller.h"
#include "Record.h"
#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <process.h>
#include <cmath>
#include <algorithm>
#include "nr.h"
using namespace std;
//extern HANDLE hInput;
extern HWND console;
//extern HANDLE inputWaitEvents[];
extern Record logFile;
extern bool internalAbort;
extern HANDLE abortEvent;
double square(double x){
	return x*x;
}

template <class Type> Type* copyArray(Type* a,int num){
	Type* ret=new Type[num];
	for(int i=0;i<num;i++){
		ret[i]=a[i];
	}
	return ret;
}


string getRealLine(ifstream& protocol){
	string line;
	getline(protocol,line);
	while ( line.size()==0 || line.at(0)=='#'){
		if (protocol.eof()) {line=="";break;}
		getline(protocol,line);
	}
	return line;
}

string getCOMError(){
	LPTSTR lpMsgBuf;//char*
	DWORD dw=GetLastError();
	if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL )){
			logFile.write("GetCOMError failed: "+getSystemError(),true);
			return "could not get error";
	}
		return string(lpMsgBuf);
}

string getSystemError(){
	LPTSTR lpMsgBuf;//char*
	DWORD dw=GetLastError();
	if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL )){
			logFile.write("getSystemError failed with system error "+GetLastError(),true);
	}
		return string(lpMsgBuf);
}

HWND initializeConsoleWindow(){
	SetConsoleTitle(string(CONSOLETITLE).c_str());
	HWND hwndFound=GetConsoleWindow();
	//get screen info
	COORD maxSize;
	RECT area;
	SystemParametersInfo(SPI_GETWORKAREA,0,&area,0);
	int widthPixels=area.right;//GetSystemMetrics(SM_CXMAXIMIZED);
	int heightPixels=area.bottom;//GetSystemMetrics(SM_CYMAXIMIZED);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//set console size and move it
	COORD SBsize;
	SBsize.X=9999;
	SBsize.Y=9999;
	SetConsoleScreenBufferSize(hOut,SBsize);
	//BOOL b=MoveWindow(hwndFound,0,0,widthPixels-1009,heightPixels/2,true);
	BOOL b=MoveWindow(hwndFound,0,0,widthPixels*(CONSOLEWIDTH)+1,heightPixels*(CONSOLEHEIGHT),true);
	if (b==0)
		logFile.write("Move Console window failed: "+toString((int) GetLastError()),true);
	
	CONSOLE_SCREEN_BUFFER_INFO SBInfo;
	GetConsoleScreenBufferInfo(hOut,&SBInfo);
	
	SBsize;
	SBsize.X=SBInfo.srWindow.Right+1;
	SBsize.Y=9999;
	SetConsoleScreenBufferSize(hOut,SBsize);
	return hwndFound;
}

std::string getUser(){
	char lpBuffer[256];
	DWORD pcbBuffer=256;
	GetUserName(lpBuffer,&pcbBuffer);
	return string(lpBuffer);
}
void showConsole(){
	SetForegroundWindow(console);
}
void showWindow(std::string title){
	// look for CONSOLETITLE
	HWND hwndFound=FindWindow(NULL, title.c_str());
	// If found, hide it
	if ( hwndFound != NULL){
		SetForegroundWindow(hwndFound);
		//ShowWindow(hwndFound,SW_MINIMIZE);
		//ShowWindow( hwndFound, SW_RESTORE);
	}else logFile.write(string("Could not find console wondow ")+title,true);
}

void clickAbort(bool iAbort){
	//DWORD result=WaitForSingleObject(abortEvent,1);
	//if (result==WAIT_TIMEOUT)
	//	return;
	SetEvent(abortEvent);
	internalAbort=iAbort;
	HWND WindowHandle=FindWindow(NULL, "ABORT");
	if (WindowHandle==NULL){
		logFile.write("Click Abort could not find ABORT window",true);
		return;
	}
	HWND ButtonHandle=FindWindowEx(WindowHandle, 0, "Button", "OK");
	if (ButtonHandle==NULL){
		logFile.write("Click Abort could not find OK button",true);
		return;
	}
	HWND WindowHandle2=FindWindow(NULL, "ABORT");
//	SetActiveWindow(WindowHandle);
	while(WindowHandle==WindowHandle2){
		SetForegroundWindow(WindowHandle);
		Sleep(100);
		SendNotifyMessage(ButtonHandle, BM_CLICK, 0 , 0);
		Sleep(100);//give the window time to close before we send another message;
		WindowHandle2=FindWindow(NULL, "ABORT");
	}
//	SendMessage(ButtonHandle, BM_CLICK, 0 , 0);
//	Sleep(1000);
	//exit(0);
	throw abortException("");//_endthreadex(0);
}
/*
bool DeleteDirectory(LPCTSTR str_folderpath, bool noRecycleBin)
{

TCHAR szSource[MAX_PATH + 2] = _T("");

::_tcsncpy(szSource, str_folderpath, MAX_PATH);
SHFILEOPSTRUCT fs;
::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

fs.pFrom = szSource;
fs.wFunc = FO_DELETE;
fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT |FOF_NOERRORUI);
int ret;
ret=::SHFileOperation(&fs);
return (ret==0);
} 
*/
/*
bool DeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin)
{
  int len = _tcslen(lpszDir);
  TCHAR *pszFrom = new TCHAR[len+2];
  _tcscpy(pszFrom, lpszDir);
  pszFrom[len] = 0;
  pszFrom[len+1] = 0;
  
  SHFILEOPSTRUCT fileop;
  fileop.hwnd   = NULL;    // no status display
  fileop.wFunc  = FO_DELETE;  // delete operation
  fileop.pFrom  = pszFrom;  // source file name as double null terminated string
  fileop.pTo    = NULL;    // no destination needed
  fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT;  // do not prompt the user
  
  if(!noRecycleBin)
    fileop.fFlags |= FOF_ALLOWUNDO;

  fileop.fAnyOperationsAborted = FALSE;
  fileop.lpszProgressTitle     = NULL;
  fileop.hNameMappings         = NULL;

  int ret = SHFileOperation(&fileop);
  delete [] pszFrom;  
  return (ret == 0);
}
*/
//extern int errno;
bool DeleteDirectory(LPCTSTR dir,bool noRecycleBin){
	if (GetFileAttributes(dir)==INVALID_FILE_ATTRIBUTES){
		logFile.write(string(dir)+" does not exist",false);
		return true;
	}
	string command=string("rmdir /S /Q \"")+dir+"\"";
	int val=errno;
	int i=system(command.c_str());
	int val2=errno;
	if (val2!=val){
		logFile.write("remove directory set errno to "+toString(errno)+". possible error",true);
		return false;
	}
	if (i==0) return true;
	logFile.write(string("Deleted ")+dir+" System returned "+toString(i)+" with errno="+toString(errno),false);
	return false;
}


string toString(int i, int width){
	char buf[256];
	sprintf(buf,"%0*d",width,i);
	return string(buf);
}

std::string toString(LONGLONG i){
	stringstream s(stringstream::in | stringstream::out);
	s.str("");
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s<<i;
	return s.str();
}

double toDouble(string st){
	stringstream s(stringstream::in | stringstream::out);
	s.str(st);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	double d;
	s>>d;
	return d;
}

double toDouble(string st,bool& b){
	stringstream s(stringstream::in | stringstream::out);
	s.str(st);
	double d=0;
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s>>d;
	if (s.rdstate()==ios_base::eofbit)
		b=true;
	else
		b=false;
	return d;
}


int toInt(string st){
	stringstream s(stringstream::in | stringstream::out);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s.str(st);
	int d;
	s>>d;
	return d;
}

int toInt(wstring st){
	wstringstream s(wstringstream::in | wstringstream::out);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s.str(st);
	int d;
	s>>d;
	return d;
}

int toInt(char c){
	char s[2];
	s[0]=c;
	s[1]=0;
	return toInt(string(s));
}

string toString(vector<double> &v){
	if (v.empty()) return "";
	string ret=toString(v.front());
	for(vector<double>::iterator i=v.begin()+1;i!=v.end();i++){
		ret=ret+", "+toString(*i);
	}
	return ret;
}

void sort(vector<double> &x,vector<double> &y){
	bool unsorted=true;
	while(unsorted){
		unsorted=false;
		vector<double>::iterator j=y.begin()+1;
		for(vector<double>::iterator i=x.begin()+1;i!=x.end();i++){
			if (*i==*(i-1)){
				double avg=(*j+*(j-1))/2;
				*(j-1)=avg;
				x.erase(i);
				y.erase(j);
				unsorted=true;
				break;				
			}else if (*i<*(i-1)){
				double x=*(i-1);
				double y=*(j-1);
				*(i-1)=*i;
				*(j-1)=*j;
				*i=x;
				*j=y;
				i=i-1;//stay on current position even after increment
				j=j-1;
				unsorted=true;
			}
			j++;
		}
	}
}

/*deprecated...too complicated split into separate functions
string toString(double d, bool sign, int precision, int beforeDecimal){
	stringstream s(stringstream::in | stringstream::out);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s<<setfill('0');
	if (precision==-1)
		s<<d<<flush;
	else if (precision>0)
		if ((d<.1 && d>-.1))
			s<<std::setprecision(precision)<<scientific<<d<<flush;
		else
			s<<std::setprecision(precision)<<fixed<<setw(beforeDecimal+1+precision)<<d<<flush;
	else if (d<1 && d>-1)
			s<<std::setprecision(precision)<<scientific<<d<<flush;
		else
			s<<std::setprecision(precision)<<fixed<<setw(beforeDecimal)<<d<<flush;
	//cout<<"testing"<<s.str()<<endl;
	string ret=s.str();
	if (sign && d>0) ret="+"+ret;
	return ret;
}
*/

std::string toString(double d, int precision,int totalWidth,bool showpos,char fill){
	stringstream s(stringstream::in | stringstream::out);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s<<setfill(fill);
	if (precision!=-1){
		s.setf(ios_base::fixed);
		s.precision(precision);
	}

	if (totalWidth!=-1)
		s.width(totalWidth);
	if (showpos)
		s.setf(ios_base::showpos);
	s<<d;
	string ret=s.str();
	return ret;
}

std::string toPercent(double d, int precision){
	return toString(100.0*d,precision)+"%";
}


std::string toStringSci(double d, int precision,int totalWidth,bool showpos,char fill){
	stringstream s(stringstream::in | stringstream::out);
	s.exceptions(std::ios::badbit | std::ios::failbit);
	s<<setfill(fill);
	s.setf(ios_base::scientific);
	if (precision!=-1){
		s.precision(precision);
	}
	if (totalWidth!=-1)
		s.width(totalWidth);
	if (showpos)
		s.setf(ios_base::showpos);
	s<<d;
	string ret=s.str();
	return ret;
}

string toString(float f){
	return toString((double) f);
}

//output string with time format: 00:00:00:00.000 days:hour:min:sec.ms
string toString(Timer t){
	double ms=t.getTime();
	int d=ms/1000.0/60.0/60.0/24.0;
	ms=ms-(d*1000*60*60*24);
	int h=ms/1000.0/60.0/60.0;
	ms=ms-(h*1000*60*60);
	int m=ms/1000.0/60.0;
	ms=ms-(m*1000*60);
	int s=ms/1000.0;
	ms=ms-(s*1000);
	return toString(d,2)+":"+toString(h,2)+":"+toString(m,2)+":"+toString(s,2)+"."+toString((int)ms,3)+"ms";	
}

string toString(bool b){
	if (b) return "TRUE";
	else return "FALSE";
}

//small memory leak here due to bad documentation for the PL32 driver will remember to clean it up with a delete[] call
char* toCharStar(string s){
	char* ret=new char[s.length()+1];
	return strcpy(ret,s.c_str());
}

string& removeWhite(std::string& s){
	s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
	return s;
}

double arrayMax(double* d, int length){
	double max=-1;
	for(int i=0;i<length;i++){
		if (d[i]>max)
			max=d[i];
	}
	return max;
}



double round(double r) {
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

double linearFit(vector<double> control,vector<double> intensity,double& slope,double& yintercept){
	NRVec<DP> x(control.size());	//y coordinate for surface approx
	NRVec<DP> y(intensity.size());	//x coordinate for surface approx
	for(int k=0;k<control.size();k++){
		x[k]=control.at(k);
		y[k]=intensity.at(k);
	}
	double siga,sigb,chi2,q,ave,var;
	NR::fit(x,y,x,false,yintercept,slope,siga,sigb,chi2,q);
	NR::avevar(y,ave,var);
	double tot=0;
	for(int k=0;k<control.size();k++){
		tot+=square(intensity.at(k)-ave);
	}
	//this is r-squared correlation
	return 1-chi2/tot;
}

double tubingLengthmm(double IDinches,double uLVolume){
	return uLVolume*4/M_PI/square(IDinches*25.4);
}



//input methods
int getInt(){
	string s;
	std::getline(cin,s);
	return toInt(s);
}

vector<int> getInts(){
	string s=getString();
	return toInts(s);
}

vector<int> toInts(string s){
	stringstream ss,ss2;
	vector<int> v;
	string sub,subh,subh2;
	ss<<s;
	while(getline(ss,sub,',') && !ss.fail()){
		if (sub=="")
			continue;
		//cout<<"npos is "<<std::string::npos<<endl;
		//cout<<"Sub find is "<<sub.find("-")<<endl;
		if (std::string::npos == sub.find("-")){
			//not found, single number
			v.push_back(toInt(sub));
			continue;
		}
		ss2=stringstream();
		ss2<<sub;
		getline(ss2,subh,'-');
		getline(ss2,subh2);
		if (ss2.fail())
			v.push_back(toInt(subh));
		else{
			if (toInt(subh2)<toInt(subh)){
				//reverse order
				for(int i=toInt(subh);i>=toInt(subh2);i--){
				v.push_back(i);
				}
			}
			for(int i=toInt(subh);i<=toInt(subh2);i++){
				v.push_back(i);
			}
		}
	}
	return v;
}

std::string commaSep(std::string s, int first, int last){
	vector<string> v;
	stringstream ss;
	ss<<s;
	for(int i=1;i<first;i++){
		getline(ss,s,',');
	}
	getline(ss,s,',');
	string ret="";
	int i=1;
	while(!ss.fail() && i<=last){
		ret+=s+",";
		i++;
		getline(ss,s,',');
	}
	return ret.substr(0,ret.size()-1);
}

double getDouble(){
	string s;
	std::getline(cin,s);
	return toDouble(s);
}

string getString(){
	string s;
	std::getline(cin,s);
	return s;
}

char getChar(){
/*	FlushConsoleInputBuffer(hInput);
	INPUT_RECORD ir[512];
	LPDWORD lpNumberOfEventsRead;
	DWORD result;
	DWORD num;
	while(true){
		result=WaitForMultipleObjects(2,inputWaitEvents,false,INFINITE);
		if (result==WAIT_OBJECT_0){
			ReadConsoleInput(hInput,ir,512,&num);
			if (isEnterPressed())
				break;
			FlushConsoleInputBuffer(hInput);
			continue;
		}else if (result==WAIT_OBJECT_0+1)
			clickAbort(true);
	}
	*/
	string s;
	DWORD res=WaitForSingleObject(abortEvent,0);
	if (res==WAIT_OBJECT_0)
		_endthreadex(0);
	std::getline(cin,s);
	res=WaitForSingleObject(abortEvent,0);
	if (res==WAIT_OBJECT_0)
		_endthreadex(0);
	if (s.size()>0)
		return s.at(0);
	else return 0;
}

bool isEnterPressed(){
	char c[1024];
	int i=0;
	bool ret=true;
	while(true && i<1024){
		if (cin.eof()) {
			ret=false;
			break;
		}
		c[i]=cin.get();
		if (c[i]==10) {
			ret=true;
			break;
		}
		i++;
	}
	for(;i>=0;i--)
		cin.putback(c[i]);
	return ret;
}
