// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, December 13, 2011</lastedit>
// ===================================
#pragma once
#include "Definitions.h"
#include <string>
#include <sstream>
#include <iomanip>
//#define NDEBUG uncomment this to get rid of asserts (probably won't happen ever)
#include <assert.h>
#include "Timer.h"
#include <vector>
#include <set>
#define _USE_MATH_DEFINES
#include "Math.h"
//calling clickAbort in a class constructor will fail
void clickAbort(bool iAbort=true);
HWND initializeConsoleWindow();
void showConsole();
void showWindow(std::string title);
std::string getCOMError();
std::string getSystemError();
std::string getUser();

double toDouble(std::string st);
double toDouble(std::string st,bool& b);
int toInt(std::string st);
int toInt(std::wstring st);
int toInt(char c);


//original precision of -1 means not fixed width after decimal (i.e. no trailing zeros). total width means left padding with fill character 
//std::string toString(double d);
std::string toString(double d,int precision=-1, int totalWidth=-1,bool showpos=false,char fill=' ');

std::string toStringSci(double d, int precision=-1, int totalWidth=-1, bool showpos=false,char fill=' ');
std::string toPercent(double d, int precision=-1);
std::string toString(float f);
std::string toString(int i, int width=0);
std::string toString(LONGLONG i);
std::string toString(Timer t);
std::string toString(bool b);
std::string toString(std::vector<double> &v);
std::string getRealLine(std::ifstream& protocol);
void sort(std::vector<double>& x,std::vector<double>& y);
 template < typename T >
 inline T highbit(T& t)
 {
    return t = (((T)(-1)) >> 1) + 1;
 }
 
 template < typename T >
 std::string toBinary(T& value)
 {
	 std::string o;
    for ( T bit = highbit(bit); bit; bit >>= 1 )
    {
 	  o += ( ( value & bit ) ? "1" : "0" );
    }
    return o;
 }
 
std::string& removeWhite(std::string& str);
char* toCharStar(std::string);
double arrayMax(double* d, int length);
template <class Type> Type* copyArray(Type* a,int num);
double square(double x);
double round(double x);
double linearFit(std::vector<double> control,std::vector<double> intensity,double& slope,double& yintercept);
bool DeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin = true);

double tubingLengthmm(double IDinches,double uLVolume);


//input methods....safe
int getInt();
std::vector<int> getInts(); //e.g. 1-5 or 1-3,5,6
std::vector<int> toInts(std::string s);
double getDouble();
std::string getString();
char getChar();
bool isEnterPressed();
std::string commaSep(std::string, int first, int last);//get subsection of comma separated string
