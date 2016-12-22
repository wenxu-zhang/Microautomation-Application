// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Warn.h"
#include "Lambda10_3.h"
#include <iostream>
#include <string>
#include "Controller.h"
#include "Record.h"

extern Record logFile;
//#include "Camera.h"
using namespace std;

const bool debug=false;

extern Record logFile;

Lambda10_3::Lambda10_3(){
	waited=true;
	isPresent=Fusb.Open((LPCSTR) "SIUJY0GZ");
	if (!isPresent){
		logFile.write("Filter Wheel NOT PRESENT");
		return;
	}
	//power on motors and set default shutter mode to FAST
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	txbuf[0] = 0xCE;
	FT_STATUS status=Fusb.Write(txbuf,1,&ret_bytes);
	if (status ||ret_bytes==0){
		cout<<"open motors write command did not send"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	status=Fusb.Read(rxbuf,2,&ret_bytes);
	if (status || ret_bytes<2){
		cout<<"could not receive confirmation of motors on"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	if (rxbuf[0]!=0xCE || rxbuf[1]!=0x0D){
		cout<<"we don't understand USB communication"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	txbuf[0] = 0xDC;
	txbuf[1] = 0x01;
	status = Fusb.Write(txbuf, 2, &ret_bytes);
	if (status ||ret_bytes<2) {
		cout<<"Switch to fast mode shutter A error"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	status=Fusb.Read(rxbuf,3,&ret_bytes);
	if (status ||ret_bytes<3){
		cout<<"Switch to fast mode shutter A not confirmed"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	if (rxbuf[0]!=0xDC || rxbuf[1]!=0x01 ||rxbuf[2]!=0x0D){
		cout<<"somehow we dont understand USB communications fast mode shutter A"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	
	/*
	txbuf[1] = 0x02;
	status = Fusb.Write(txbuf, 2, &ret_bytes);
	if (status || ret_bytes<2){
		cout<<"Switch to fast mode shutter B error"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	status=Fusb.Read(rxbuf,3,&ret_bytes);
	if (status ||ret_bytes<3){
		cout<<"Switch to fast mode shutter B not confirmed"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	if (rxbuf[0]!=0xDC ||rxbuf[1]!=0x02 ||rxbuf[2]!=0x0D){
		cout<<"somehow we dont understand USB communications fast mode shutter B"<<endl;
		Fusb.Close();
		system("pause");
		exit(0);
	}
	*/
	/*////open shutters
	Bopen=false;
	Aopen=false;
	OpenExShutter();
	OpenEmShutter();
	*/
	
	//close shutters
	Bopen=true;
	Aopen=true;
	//CloseShutters(true);
	closeShutterA(true);
	closeShutterB(true);
	//Fusb.Purge(FT_PURGE_RX || FT_PURGE_TX);
	/*CloseExShutter();
	Sleep(1000);
	CloseEmShutter();
	Sleep(1000);
	*/

	//set ex and em filter wheels to position 1 and 0 respectively
	posB=0;
	posA=1;
	switchFilterA(posA,true);
	switchFilterB(posB,true);
	//SwitchFilters(1,0,true);
	//Fusb.Purge(FT_PURGE_RX || FT_PURGE_TX);

	//set filter wheels to position 0
	//posEx=1;
	//posEm=1;
	//SwitchExFilter(0);
	//SwitchEmFilter(0);

	cout<<"Filter Wheels ready"<<endl;

}

Lambda10_3::~Lambda10_3(){
	CheckExists()
	CloseShutters();
	//SwitchExFilter(1);
	//SwitchEmFilter(0);
	Fusb.Close();
}

/*
void Lambda10_3::SwitchFilters(int posX,int posM,bool w8){
		DWORD ret_bytes;
	FT_STATUS status;
	unsigned char txbuf[25],rxbuf[25];
	if (!waited){
		cout<<"you didn't w8 for the filter wheels to finish execution: the controller might ignore our command..purging receive buffer"<<endl;
		Fusb.Purge(FT_PURGE_RX);
		//waited=true;changed 09/20/2006
	}
	waited=false;//added 09/20/2006
	if (posEx!=posX && posM!=posEm){//batch change filters
		bytes_sent=4;
		posEm=posM;
		posEx=posX;
		waited=false;
		txbuf[0]=0xBD;
		txbuf[1]=speed*16+posEx;
		txbuf[2]=(speed+8)*16+posEm;
		txbuf[3]=0xBE;
		status=Fusb.Write(txbuf,4,&ret_bytes);
		if (debug){		cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
		if (status || ret_bytes<4){
			cout<<"could not write command to USB"<<endl;
			system("pause");
			exit(0);
		}
		if (w8) wait();

	}else if(posEm!=posM){//single close
		SwitchEmFilter(posM,w8);
	}else if(posEx!=posX){//single close
		SwitchExFilter(posX,w8);
	}else {
		if (debug) cout<<"filters are already in correct positions"<<endl;	
		waited=true;
		return; //no need to issue any commands 
	}
}
*/
void Lambda10_3::switchFilterA(int pos,bool w8){
	CheckExists()
	if (!waited){
		logFile.write("FilterWheel: did not w8 for device to finish last command...purging receive buffer",false);
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (posA==pos){
		if (debug) cout << "Excitation filter already in position: " << pos <<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	posA=pos;
	txbuf[0] = speed*16+posA;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes==0) {
		cout<<"EXFILTER WRITE ERROR going to pos: "<<pos<<endl;
		system("pause");
		exit(0);
	}
	if (w8) wait();
}

void Lambda10_3::switchFilterB(int pos, bool w8){
	CheckExists()
	if (!waited){
		cout<<"did not w8 for device to finish last command...purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (posB==pos){
		if (debug) cout << "Emmission filter already in position: " << pos <<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	posB=pos;
	txbuf[0] = (speed+8)*16+posB;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes==0) {
		cout<<"EMFILTER WRITE ERROR going to pos: "<<pos<<endl;
		system("pause");
		exit(0);
	}
	if (w8) wait();
}
void Lambda10_3::openShutterA(bool w8){
	CheckExists()
	if(!waited){
		cout<<"didn't w8 for last command to finish...purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (Aopen){
		if (debug) cout<<"Excitation shutter already open!"<<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	txbuf[0] = 0xAA;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes<1) cout<<"EXSHUTTER Open WRITE ERROR"<<endl;
	if (w8) wait();
	Aopen=true;
}

void Lambda10_3::closeShutterA(bool w8){
	CheckExists()
	if(!waited){
		cout<<"didn't w8 for last command to finish...purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (!Aopen){
		if (debug) cout<<"Excitation shutter already closed!"<<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	txbuf[0] = 0xAC;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes<1) cout<<"EXSHUTTER Close WRITE ERROR"<<endl;
	if (w8) wait();
	Aopen=false;
}

void Lambda10_3::openShutterB(bool w8){
	CheckExists()
	if(!waited){
		cout<<"didn't w8 for last command to finish...purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (Aopen){
		if (debug) cout<<"Emission shutter already open!"<<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	txbuf[0] = 0xBA;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes<1) cout<<"EMSHUTTER Open WRITE ERROR"<<endl;
	if (w8) wait();
	Aopen=true;
}

void Lambda10_3::closeShutterB(bool w8){
	CheckExists()
	if(!waited){
		cout<<"didn't w8 for last command to finish...purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
	}
	waited=false;
	if (!Aopen){
		if (debug) cout<<"emmission shutter already closed!"<<endl;
		waited=true;
		return;
	}
	bytes_sent=1;
	unsigned char txbuf[25],rxbuf[25];
	DWORD ret_bytes;
	txbuf[0] = 0xBC;
	FT_STATUS status = Fusb.Write(txbuf, 1, &ret_bytes);
	if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
	if (status || ret_bytes<1) cout<<"EXSHUTTER Open WRITE ERROR"<<endl;
	if (w8) wait();
	Aopen=false;
}

void Lambda10_3::CloseShutters(bool w8){
	CheckExists()
	DWORD ret_bytes;
	FT_STATUS status;
	unsigned char txbuf[25],rxbuf[25];
	if (!waited){
		cout<<"you didn't w8 for the filter wheels to finish execution: the controller might ignore our command..purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		wait();
		//waited=true;
	}
	if (Bopen &&Aopen){//batch close
		bytes_sent=4;
		waited=false;
		txbuf[0]=0xBD;
		txbuf[1]=0xAC;
		txbuf[2]=0xBC;
		txbuf[3]=0xBE;
		status=Fusb.Write(txbuf,4,&ret_bytes);
		if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
		if (status || ret_bytes<4){
			cout<<"could not write command to USB"<<endl;
			system("pause");
			exit(0);
		}
		if (w8) wait();
		Bopen=false;
		Aopen=false;
	}else if(Bopen){//single close
		closeShutterB(w8);
		Bopen=false;
	}else if(Aopen){//single close
		closeShutterA(w8);
		Aopen=false;
	}else {
		if (debug) cout<<"shutters are already closed"<<endl;	
		waited=true;
		return; //no need to issue any commands 
	}
}

void Lambda10_3::OpenShutters(bool w8){
	CheckExists()
	DWORD ret_bytes;
	FT_STATUS status;
	unsigned char txbuf[25],rxbuf[25];
	if (!waited){
		cout<<"you didn't w8 for the filter wheels to finish execution: the controller might ignore our command..purging receive buffer"<<endl;
		//Fusb.Purge(FT_PURGE_RX);
		//waited=true;
		wait();
	}
	if (!Bopen &&!Aopen){//batch close
		bytes_sent=4;
		waited=false;
		txbuf[0]=0xBD;
		txbuf[1]=0xAA;
		txbuf[2]=0xBA;
		txbuf[3]=0xBE;
		status=Fusb.Write(txbuf,4,&ret_bytes);
		if (debug){	cout<<"sent commands:";
	for(int i=0;i<ret_bytes;i++){
		cout<<"0x"<<hex<<(unsigned int)txbuf[i]<<" "<<dec;
	}
	cout<<endl;}
		if (status || ret_bytes<4){
			cout<<"could not write command to USB"<<endl;
			system("pause");
			exit(0);
		}
		if (w8) wait();
		Aopen=true;
		Bopen=true;
	}else if(!Bopen){//single close
		openShutterB(w8);
		//Bopen=true;
	}else if(!Aopen){//single close
		openShutterA(w8);
		//Aopen=true;
	}else {
		if (debug) cout<<"shutters are already open"<<endl;	
		waited=true;
		return; //no need to issue any commands 
	}
}

void Lambda10_3::wait(){
	CheckExists()
	if (waited) {
		if (debug) cout<<"we already waited...maybe we were already at that position..otherwise check your code"<<endl;
		waited=true;
		return;
	}
	unsigned char rxbuf[25];
	DWORD ret_bytes;
	FT_STATUS status;
	//Timer t(true);
	status=Fusb.Read(rxbuf, bytes_sent+1, &ret_bytes);
	if (debug){	cout<<"returned commands:";
		for(int i=0;i<ret_bytes;i++){
			cout<<hex<<"0x"<<(unsigned int)rxbuf[i]<<" "<<dec;
		}
		cout<<endl;}
	if (status||ret_bytes<=bytes_sent||rxbuf[bytes_sent]!=0x0D){
		cout<<"didn't receive confirmation of last command...exiting"<<endl;
		system("pause");
		exit(0);
	}
	//t.waitAfterLastStart(200);
	waited=true;
}
/*

void Controller::testFilters(){
	filt->closeShutter();
	Timer t;
	int de=2;
	//0 to 1
	cout<<"testing with AUTOBLANKING ON..."<<endl;
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->closeShutter();
	}
	t.stopTimer();
	cout<<"100 moves to position 1 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<(t.getTime()/200.0)<< "ms per adjacent switch"<<endl;
	
	//1 to 2
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 2 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//2 to 3
	filt->switchFilter(2);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 2 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//3 to 4
	filt->switchFilter(3);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 3 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//0 to 2
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 2 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//0 to 3
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//0 to 4
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//1 to 3
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//1 to 4
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;


	///////////////////////////////////////////////
	cout<<"testing with AUTOBLANK OFF..."<<endl;
//	outportb(Filter::OUTPUT, 0xBC);
	filt->delay(filt->del);
	filt->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->closeShutter();
	}
	t.stopTimer();
	cout<<"100 moves to position 1 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<(t.getTime()/200.0)<< "ms per adjacent switch"<<endl;
	
	//1 to 2
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 2 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//2 to 3
	filt->switchFilter(2);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 2 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//3 to 4
	filt->switchFilter(3);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 3 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//0 to 2
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(2);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 2 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;
	
	//0 to 3
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//0 to 4
	filt->switchFilter(0);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(0);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 0 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//1 to 3
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(3);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 3 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//1 to 4
	filt->switchFilter(1);
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(4);
		t.stopTimer();
		Sleep(de);
		t.startTimer();
		filt->switchFilter(1);
	}
	t.stopTimer();
	cout<<"100 moves to position 4 and back to position 1 took: "<<t.getTime()<<"ms.  Approx "<<t.getTime()/200.0<< "ms per adjacent switch"<<endl;

	//autoblank on
//	outportb(Filter::OUTPUT, 0xBA);
	Sleep(60);
	filt->delay(filt->del);
	filt->wait();
	
	
	////display on
	//outportb(Filter::OUTPUT, 0xDB);
	//Sleep(60);
	////enable strobe trigger;
	//outportb(Filter::OUTPUT,0xCA);
	//Sleep(60);
	//filt->delay(Filter::del);//	Sleep(100);
	//filt->wait();
	//filt->delay(Filter::del);
	//filt->wait();
	//filt->closeShutter();
	//Sleep(1000);

	int filts[6]={0,1,2,3,4,-1};
	filt->setRingBuffer(filts);
	d->triggerDG();
	Sleep(1000);
	d->triggerDG();
	Sleep(1000);
	d->triggerDG();
	Sleep(1000);
	d->triggerDG();
	Sleep(1000);
	d->triggerDG();
	Sleep(1000);
	filt->closeShutter();
	
}	*/
/*
void Lambda10_3::waitShutters(){

}

void Lambda10_3::waitFilters(){
	
}*/
/*//MAIN FUNCTION FOR TESTING
int main(int argc, char* argv[])
{
	return 0;
}*/

