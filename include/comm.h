// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Monday, May 23, 2011</lastedit>
// ===================================
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "time.h"
extern HANDLE abortEvent;
extern Controller cont;
using namespace std;

class comm{
	public:
char comportstr[8];
long int rs485setup;
long int comportnum;
long int timemult;

HANDLE hPref, hCommAccess;    
DCB dcb;
HANDLE hCom;
DWORD dwError;
BOOL fSuccess;
DWORD dwBytesWritten;
DWORD dwBytesRead;

COMMTIMEOUTS commto;
DWORD cmask;

unsigned char ser_in_use;
unsigned char SER_FLAG;

comm(){
	rs485setup=0;
	SER_FLAG=0;
	hCommAccess=CreateMutex(NULL, false, NULL);
}


void int_ascii(long int lval,char *str){
    unsigned long int ans;
    unsigned long int leftover;
    unsigned long int tempb;
    unsigned int tempc;
    unsigned int tempd;
    ans=lval;
    while(*str)str++;
    tempb=268435456;
    for(tempc=0;tempc<8;tempc++){
        leftover=ans%tempb;
        tempd=(ans/tempb);
        if(tempd<10){
                *str=tempd+'0';
                }
        else {
                *str=(tempd-10)+'a';
                }
        ans=leftover;
        tempb=tempb/16;
        str++;
        }
    *str=0;
}
   

void fixtoa(long int ival,char *str,int dp){
    int ans;
    int leftover;
    int zero=0;

    if(ival < 0){
        ival=ival * -1;
        *str++='-';
        }
    ans=ival/10000;
    leftover=ival%10000;
    if(ans == 0)zero=1;
    else {
        *str=ans+'0';
        str++;
    }
    ans=leftover/1000;
    leftover=leftover%1000;
    if(zero && (ans==0))zero=2;
    else {
        *str=ans+'0';
        str++;
    }
    ans=leftover/100;
    leftover=leftover%100;
    if((zero==2)&& (ans==0))zero=3;
    else {
        *str=ans+'0';
        str++;
    }
    if(dp==2){
        if(zero==3)*str++='0';
        *str='.';
        str++;
        }
    ans=leftover/10;
    leftover=leftover%10;
    *str=ans+'0';
    str++;
    if(dp==1){
        *str='.';
        str++;
        }
    *str=leftover+'0';
    str++;
    *str='\0';
}
    

long int conv_ret_str(char *str){
long int v1=0;
int v2=0;
char *cs1ptr;
v1=0;
cs1ptr=str;
if(*cs1ptr=='*')cs1ptr++;
while(*cs1ptr){
   if(*cs1ptr<='9')v2=*cs1ptr-'0';
   else v2=(*cs1ptr-'a')+10;
   v1=(v1*16)+v2;
   cs1ptr++;
   if(*cs1ptr=='^')break;
   }
return v1;
}

long int conv_dec_str(char *str,long int dp_mult){
long int v1=0;
int pos_neg=1;
int v2=0;
char *cs1ptr;
int dp_present=0;
v1=0;
cs1ptr=str;
while(*cs1ptr==' ')cs1ptr++;
if(*cs1ptr=='-'){
    pos_neg=-1;
    cs1ptr++;
}
if(*cs1ptr=='.'){
    cs1ptr++;
    dp_present=1;
}
while(*cs1ptr){
   while(*cs1ptr==' ')cs1ptr++;
   if(*cs1ptr=='.'){
       cs1ptr++;
       dp_present=1;
       }
   if((*cs1ptr >='0') && (*cs1ptr<='9'))v2=*cs1ptr-'0';
   else v2=0;
   v1=(v1*10)+v2;
   cs1ptr++;
   if(dp_present && (dp_mult < 100)){
       break;
       }
   else if(dp_present){
       dp_present++;
       if(dp_present > 2)break;
       }
   }
if(!dp_present)v1=v1*dp_mult;
else if((dp_mult > 10) && (dp_present < 3))v1=v1*10;
v1=v1*pos_neg;
return v1;
}


void closecom(void)
    {
        // Close serial port here
//		EscapeCommFunction(hCom,CLRRTS);
        CloseHandle(hCom);
		CloseHandle(hCommAccess);
    }


int opencom(char *str)
    {
      // Open Serial Port here
		ser_in_use=1;
        
        hCom = CreateFile(str,GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
        if(hCom==INVALID_HANDLE_VALUE){
			logFile.write(string("Could not open com port ")+string(str)+". Error: "+getCOMError() );
			return 0;
		}
        GetCommState(hCom,&dcb);
        dcb.BaudRate=9600;
        dcb.ByteSize=8;
        dcb.Parity=NOPARITY;
        dcb.StopBits=ONESTOPBIT;
        dcb.fRtsControl=RTS_CONTROL_DISABLE;
        SetupComm(hCom,4096,4096);
        SetCommState(hCom, &dcb);
        commto.ReadIntervalTimeout=1000;
        commto.ReadTotalTimeoutMultiplier=100;
        commto.ReadTotalTimeoutConstant=1000;
        commto.WriteTotalTimeoutMultiplier=10;
        commto.WriteTotalTimeoutConstant=200; 
        if (!SetCommTimeouts(hCom,&commto)){
			logFile.write("SetCommTimeouts failed",true);
			cont.abort();
		}
        SetCommMask(hCom,EV_TXEMPTY);
        cmask=EV_TXEMPTY;
        PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
        ser_in_use=0;
        return 1;
    }

long int readint(char *str)
{
	if (!abortEvent) 
		WaitForSingleObject(hCommAccess, INFINITE);
	else {
		HANDLE h[]={abortEvent,hCommAccess};//give preference to abortEvent
		DWORD eventNum=WaitForMultipleObjects(2,h,false,INFINITE);
		if (eventNum==WAIT_OBJECT_0)
			return 0;
	}

   char temp_str[100]="\0";
   long int tint1;
	unsigned char charout;
   ser_in_use=1;
   //PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
   //EscapeCommFunction(hCom,SETRTS);
	string t(str);
	DWORD nwritten;
	char lpMsgBuf[512];
	if (!WriteFile(hCom,t.c_str(),t.length(),&nwritten,NULL)){
		DWORD dw=GetLastError();
		if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL )){
			logFile.write("could not get formatted message",true);
		}
		//cout<<"error on te module com port:" <<(char*)lpMsgBuf<<endl;
			logFile.write(string("Failed to write character to com port: ")+string((char*)lpMsgBuf),true);
		}
	if (nwritten!=t.length()){
		logFile.write("te comm: did not write all characters to com port",true);
		return -1;
	}
   /* while(*str){
        delay(1);
		charout=*str++;
		WriteFile(hCom
		if (!TransmitCommChar(hCom,charout)){
			logFile.write(string("Failed to write character to com port:")+::toString(int(GetLastError())),true);
		}
		WaitCommEvent(hCom,&cmask,NULL);
	}*/
    
  //  EscapeCommFunction(hCom,CLRRTS);
    //delay(3);
    temp_str[0]='\0';
	int ret;

	ret=ReadFile(hCom,temp_str,12,&dwBytesRead,NULL);
	if (!ret) {
		
		DWORD dw=GetLastError();
		FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

		cout<<"error on te module com port:" <<(char*)lpMsgBuf<<endl;
	}
	if (!dwBytesRead){
		return -1;
	}
    if(dwBytesRead==12)SER_FLAG=0;
    else {
		SER_FLAG=1;
		logFile.write("te comm: failed to read 12 bytes",true);
	}
    tint1=0;
    while(temp_str[tint1]){
        tint1++;
    }
    temp_str[tint1-3]='\0';
    tint1=conv_ret_str(temp_str);
    ser_in_use=0;
	ReleaseMutex(hCommAccess);
    return tint1;
}

unsigned char getcksum(char *str){
unsigned int cksumval=0;
        if(*str=='*')str++;
        while(*str){
            if((*str!='^')&&(*str!=0x0d)&&(*str!='*'))cksumval+=*str;
            str++;
        }
        return cksumval % 256;
}

long int getcomval(char *str){
    char temp_str[25]="\0";
    char strtemp2[25]="\0";
    long int retval;
    unsigned char ck;

    int_ascii(rs485setup,strtemp2);
    strtemp2[0]='*';
    strtemp2[1]=strtemp2[6];
    strtemp2[2]=strtemp2[7];
    strtemp2[3]='\0';
    strcpy(temp_str,strtemp2);
    strcat(temp_str,str);
    ck=getcksum(temp_str);
    strtemp2[0]='\0';
    int_ascii(ck,strtemp2);
    strtemp2[0]=strtemp2[6];
    strtemp2[1]=strtemp2[7];
    strtemp2[2]=0x0d;
    strtemp2[3]='\0';
    strcat(temp_str,strtemp2);
    retval=readint(temp_str);
    return retval;
}
    

int setint(char *command_str,long int value){
    char temp_str[25]="\0";
    char temp_str1[25]="\0";
    char strtemp2[25]="\0";
    unsigned char ck;

    int_ascii(rs485setup,strtemp2);
    strtemp2[0]='*';
    strtemp2[1]=strtemp2[6];
    strtemp2[2]=strtemp2[7];
    strtemp2[3]='\0';
    strcpy(temp_str,strtemp2);
    strcat(temp_str,command_str);
    int_ascii(value,temp_str1);
    strcat(temp_str,temp_str1);

    ck=getcksum(temp_str);
    strtemp2[0]='\0';
    int_ascii(ck,strtemp2);
    strtemp2[0]=strtemp2[6];
    strtemp2[1]=strtemp2[7];
    strtemp2[2]=0x0d;
    strtemp2[3]='\0';
    strcat(temp_str,strtemp2);
    return readint(temp_str);
}
};