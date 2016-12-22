#include "ProtocolYing_Ja.h"
#include "ScanSpots.h"
#include "GridScan.h"
#include "PerimeterFocus.h"
#include "SpotFocus.h"
#include "GridFocus.h"
#include "DefiniteFocus.h"
#include "XYStage.h"
#include <iostream>
#include <limits>
using namespace std;
extern Controller cont;
extern Record logFile, focusLogFile;

void ProtocolYing_Ja::runProtocol(){
	char c;
	int i;
	while(true){
		cout<<"Please select a protocol to run:"<<endl;
		cout<<"1: Test protocols or modules"<<endl;
		cout<<"2: Set working directory of all related log files"<<endl;
		cout<<"3: Scan while ramping temperature"<<endl;
		cout<<"4: Focus many times to test autofocus module"<<endl;
		cout<<"5: Calibrate the Z focus vs. temperature"<<endl;
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':
				testProtocols();
				break;
			case '2':
				cont.modifyWorkingDir();
				cont.te.changeLogFile(cont.workingDir);
				focusLogFile.close();
				focusLogFile.open(cont.workingDir+"focusLog.txt");
				break;
			case '3':
				TemperatureScan();
				break;
			case '4':
				cout<<"Please enter the focusing mode to use: \n"
					<<"1: manual focusing\n"
					<<"2: autofocusing\n";
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				FocusNTimes(i);
				break;
			case '5':
				cout<<"Please enter the focusing mode to use: \n"
					<<"1: manual focusing\n"
					<<"2: autofocusing\n";
				cin>>i;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ZFocusVsTemperature(i);
				break;
			case 'e':
				return;
				break;

		}
	}
}
void ProtocolYing_Ja::testProtocols(){ //This is used to test modules that I wrote for the system.
	char c;
	string comment;
	while(true){
		cout<<"Please select a protocol to test:"<<endl;
		cout<<"1: Test scanSpots with perimeter focus"<<endl;
		cout<<"2: Test TEModule trigger recording"<<endl;
		cout<<"3: While already in trigger recording mode, send a trigger for recording"<<endl;
		cout<<"4: Test DG\n";
		cout<<"5: Test Ludl\n";
		cout<<"6: Test Definite Focus System\n";
		cout<<"7: Test SpotScan\n";
		cout<<"8: Test SpotFocus\n";
		cout<<"9: Test focus drive\n";
		cout<<"e: Exit"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		switch(c){
			case '1':
				testScanSpotsPerimeterFocus();
				break;
			case '2':
//				cont.te.triggerRecording();
				cont.te.comment="testTriggerRecording 1st comment";
				cont.te.triggerRecord=true;
				break;
			case '3':
				cout<<"Type in the comment";
				cin>>comment;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.te.comment=comment;
				cont.te.triggerRecord=true;				
				break;
			case '4':
				
				break;
			case '5':
				
				break;
#ifdef OBSERVER
			case '6':
				testDFS();
				break;
#endif
			case '7':
				testScanSpots();
				break;
			case '8':
				testSpotFocus();
				break;
			case '9':
				
				break;
			case 'e':
				return;
				break;

		}
	}
}

#ifdef OBSERVER
void ProtocolYing_Ja::testDFS(){
	Record testDFSfile(cont.workingDir+"\\testDFSfile.txt",0);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2()),true,"begin DFS test");
//	cont.takePicture(&cont.currentChannel(),"beginDFStest",NULL,false,false,false);
	ObserverDefiniteFocus DFS(&(cont.axio));
	DFS.initializeDefiniteFocus();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"initializeDFS");
//	cont.takePicture(&cont.currentChannel(),"initializeDFS",NULL,false,false,false);
	int pos0x=cont.stg->getX();
	int pos0y=cont.stg->getY();
	cont.stg->stepDown();
	cont.stg->stepDown();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"pos1");
//	cont.takePicture(&cont.currentChannel(),"pos1",NULL,false,false,false);
	//DFS.initializeDefiniteFocus(1);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"initialize pos1");
//	cont.takePicture(&cont.currentChannel(),"initialize pos1",NULL,false,false,false);
	int pos1x=cont.stg->getX();
	int pos1y=cont.stg->getY();
	cont.stg->stepDown();
	cont.stg->stepDown();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"pos2");
//	cont.takePicture(&cont.currentChannel(),"pos2",NULL,false,false,false);
	//DFS.initializeDefiniteFocus(2);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"initialize pos2");
//	cont.takePicture(&cont.currentChannel(),"initialize pos2",NULL,false,false,false);
	int pos2x=cont.stg->getX();
	int pos2y=cont.stg->getY();
	DFS.startDefiniteFocus();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"startDFS");
//	cont.takePicture(&cont.currentChannel(),"startDFS",NULL,false,false,false);
	Timer::wait(10000);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"wait10s");
//	cont.takePicture(&cont.currentChannel(),"wait10s",NULL,false,false,false);
	cont.stg->move(pos0x,pos0y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"startpos0");
//	cont.takePicture(&cont.currentChannel(),"startpos0",NULL,false,false,false);
	cont.stg->move(pos1x,pos1y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"startpos1");
//	cont.takePicture(&cont.currentChannel(),"startpos1",NULL,false,false,false);
	cont.stg->move(pos2x,pos2y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"startpos2");
//	cont.takePicture(&cont.currentChannel(),"startpos2",NULL,false,false,false);
	cont.te.setFixedTemp(cont.te.getTemp()+15,true);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"focus at +15C");
//	cont.takePicture(&cont.currentChannel(),"foc+15C",NULL,false,false,false);
	cont.stg->move(pos0x,pos0y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"15Cpos0");
//	cont.takePicture(&cont.currentChannel(),"15Cpos0",NULL,false,false,false);
	cont.stg->move(pos1x,pos1y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"15Cpos1");
//	cont.takePicture(&cont.currentChannel(),"15Cpos1",NULL,false,false,false);
	cont.stg->move(pos2x,pos2y);
	cont.stg->wait();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"15Cpos2");
//	cont.takePicture(&cont.currentChannel(),"15Cpos2",NULL,false,false,false);
			
	OutputRampParam *rmp=new OutputRampParam();
	rmp->iniVolt=-20;
	rmp->endVolt=-100;
	rmp->slope=-0.025;
	rmp->targetTemp=80;
	rmp->timeout=10*60;
	rmp->temod=&(cont.te);
	cout << "Current temperature is " << cont.te.getTemp() << endl;
	cout << "Current output power is " << cont.te.getOutputPower() << endl;
	unsigned threadIDrp, threadIDrd;
	HANDLE hRampThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::rampOutputThread, rmp, 0, &threadIDrp);
	int picnum=0;
	double prevTemp=cont.te.getTemp();
	bool DFSisOn=DFS.isOn();
	while (cont.te.ramp && DFSisOn){
		if (cont.te.getTemp()>prevTemp+0.3){
			cont.stg->move(pos0x,pos0y);
			cont.stg->wait();
			testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"ramp pic"+toString(picnum)+"pos0");
//			cont.takePicture(&cont.currentChannel(),"ramppic"+toString(picnum)+"pos0",NULL,false,false,false);
			cont.stg->move(pos1x,pos1y);
			cont.stg->wait();
			testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"ramp pic"+toString(picnum)+"pos1");
//			cont.takePicture(&cont.currentChannel(),"ramppic"+toString(picnum)+"pos1",NULL,false,false,false);
			cont.stg->move(pos2x,pos2y);
			cont.stg->wait();
			testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"ramp pic"+toString(picnum)+"pos2");
//			cont.takePicture(&cont.currentChannel(),"ramppic"+toString(picnum)+"pos2",NULL,false,false,false);
			prevTemp=cont.te.getTemp();
			DFSisOn=DFS.isOn();
			picnum++;
		}
	}
	cont.te.ramp=false;
	cont.te.powerOff();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"TEoff");
//	cont.takePicture(&cont.currentChannel(),"TEoff",NULL,false,false,false);
	Timer::wait(10000);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"TEoff10s");
//	cont.takePicture(&cont.currentChannel(),"TEoff10s",NULL,false,false,false);
	DFS.stopDefiniteFocus();
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"stop");
//	cont.takePicture(&cont.currentChannel(),"stop",NULL,false,false,false);
	Timer::wait(10000);
	testDFSfile.write(toString(cont.stg->getX())+"\t"+toString(cont.stg->getY())+"\t"+toString(cont.focus->getZ())+"\t"+toString(cont.te.getTemp())+"\t"+toString(cont.te.getTemp2())+"\t"+toString(DFS.isOn())+"\t"+toString(DFS.getOnOffChangedReason()),true,"stop10s");
//	cont.takePicture(&cont.currentChannel(),"stop10s",NULL,false,false,false);
	testDFSfile.close();
}
#endif OBSERVER
void ProtocolYing_Ja::testScanSpots(){
	ScanSpots *scanspots=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots");
	//ScanSpots *scanspots2=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots2");
	int n;
	cout<<"How many times to scan?\n";
	cin>>n;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	for(int i=0;i<n;i++){
		scanspots->runScan(1,i*2);
	//	scanspots2->runScan(1,i*2+1);
	}
	int again=1;
	while (again!=0){
		cout<<"Enter 0 when you want to delete the scan object.\n"
			<<"Enter n if you want to run it again n times.\n";
		cin>>again;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (again!=0){
			for(int i=0;i<again;i++){
				scanspots->runScan(1,i);
	//			scanspots2->runScan(1,i);
			}
		}
	}
	delete scanspots;
}
void ProtocolYing_Ja::testScanSpotsPerimeterFocus(){
	ScanSpots *scanspots=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots");
	int minX,minY,maxX,maxY;
	scanspots->getScanRegion(minX,maxX,minY,maxY);
	scanspots->sf=new PerimeterFocus(cont.currentFocus,minX-scanspots->getChan(0).out->cam->getImageWidth(scanspots->getChan(0).ap),minY-scanspots->getChan(0).out->cam->getImageHeight(scanspots->getChan(0).ap),maxX+scanspots->getChan(0).out->cam->getImageWidth(scanspots->getChan(0).ap),maxY+scanspots->getChan(0).out->cam->getImageHeight(scanspots->getChan(0).ap),2,2);
	int n;
	cout<<"How many times to scan?\n";
	cin>>n;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	scanspots->runScan(n,1);
	scanspots->recordFocus(1,2);
	bool del;
	cout<<"Enter 1 when you want to delete the scan object.\n";
	cin>>del;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	if (del) delete scanspots;
}

void ProtocolYing_Ja::testSpotFocus(){
	ScanSpots *scanspots=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots");
	//Solution s=cont.pmp.selectReagent();
	//bool prime;
	//cont.pmp.initialize();
	//cout << "Prime fluidics? (1 for yes, 0 for no)\n";
	//cin >> prime;
	//if (prime) {
	//	cont.pmp.primeReagent();
	//}
//	cont.te.triggerRecording();
	cont.te.linearOutputRamp(10*60,false);
	int scanNum=1;
	while(cont.te.ramp){
	//	cont.pmp.wait();
		scanspots->recordFocus(1,scanNum);
	//	cont.pmp.wash(s,1);
		scanNum++;
	}
	string comment;
	comment="Finish";
	cont.te.triggerRecord=true;
	WaitForSingleObject(cont.te.hRecordAccess,INFINITE);
	cont.te.record=false;
	ReleaseMutex(cont.te.hRecordAccess);
}


void ProtocolYing_Ja::denature(Solution *s){
	cont.te.setFixedTemp(80);
	cont.te.setPropBand(10);
	cont.te.setIntegGain(1.5);
	cont.te.setDerGain(0.2);
	cont.te.powerOn();
	Timer t(true);
	int begintime=0;
	cont.te.wait();
	t.waitAfterLastStart(5*60*1000);
	//cont.pmp.wait();
	//cont.pmp.wash(*s,1);
	//cont.pmp.wait();
	cont.te.powerOff();
}

void ProtocolYing_Ja::ZFocusVsTemperature(int mode){
	cout<<"Please choose the channel to focus in.";
	cont.currentChannel().modify();
	cont.te.linearOutputRamp(60*60);
	cont.te.comment=cont.currentChannel().toString();
//	cont.te.triggerRecording();
	cont.te.comment="Focused position";
	cont.te.triggerRecord=true;
	int t;
	cout<<"Enter wait time in ms:\n";
	cin>>t;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	char c;
	int pixnum=1;
	cont.pmp.sendCommand("IR",1);
	while(cont.te.ramp){
//		cont.pmp.wait();
		switch (mode){
			case 1: //manual focus
				cont.currentChannel().on();
				cont.currentChannel().out->cam->startLiveView();
				cout<<"Manually find the focus and enter 1 when done\n";
				cin>>c;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.currentChannel().out->cam->stopLiveView();
				cont.currentChannel().off();
				break;
			case 2: //autofocus
				cont.currentFocus.a=cont.currentChannel();
				cont.currentFocus.getFocus(true);
				break;
		}
		cont.te.comment=toString(pixnum)+": "+toString(cont.focus->getZ());
		cont.te.triggerRecord=true;
		cont.currentChannel().takePicture(cont.workingDir+toString(pixnum));
		//cont.pmp.washChamber(2,50,30,false);
		Timer::wait(t);
		pixnum++;
	}
	cont.pmp.sendCommand("OR",1);
	cont.te.record=false;
}

void ProtocolYing_Ja::FocusNTimes(int mode){
	int n;
	cout<<"Please choose the channel to focus in.";
	cont.currentChannel().modify();
	cout<<"How many times do you want to autofocus?\n";
	cin>>n;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	int t;
	cout<<"Enter wait time in ms:\n";
	cin>>t;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	char c;
	int pixnum=1;
	cont.pmp.sendCommand("IR",1);
	for(int i=0;i<n;i++){
//		cont.pmp.wait();
		switch (mode){
			case 1: //manual focus
				cont.currentChannel().on();
				cont.currentChannel().out->cam->startLiveView();
				cout<<"Manually find the focus and enter 1 when done\n";
				cin>>c;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.currentChannel().out->cam->stopLiveView();
				cont.currentChannel().off();
				break;
			case 2: //autofocus
				cont.currentFocus.a=cont.currentChannel();
				cont.currentFocus.getFocus(true);
				break;
		}
		double z=cont.focus->getZ();
		cont.currentChannel().takePicture("t"+toString(pixnum,3)+"p000c0");
		cont.focus->move(z+0.1);
		cont.currentChannel().takePicture("t"+toString(pixnum,3)+"p001c0");
		cont.focus->move(z-0.2);
		cont.currentChannel().takePicture("t"+toString(pixnum,3)+"p002c0");
		Timer::wait(t);
		pixnum++;
	}
	cont.pmp.sendCommand("OR",1);
}

void ProtocolYing_Ja::TemperatureScan(){
	int num;
	char dfsmode;
	Scan *scan, *scan2, *scan3;
	FocusInTemp zpos;//, zpos2;
	Record recordFile(cont.workingDir+"record.txt",0,logFile.swatch);
	cout << "Please choose scanning mode:\n"
		<< "1: Scan region\n"
		<< "2: Scan spots\n";
	cin >>num;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	switch (num){
		case 1:
			//scan=new GridScan(AcquisitionChannel::getAcquisitionChannels(),"scangrid");
			break;
		case 2:
			scan=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots");
			break;
	}
	int minX,minY,maxX,maxY;
	scan->getScanRegion(minX,maxX,minY,maxY);
	cout<<"Please select focusing method:\n"
		<<"1: no focus correction\n"
		<<"2: focus around the perimeter and interpolate for scan region\n"
		<<"3: focus at a spot\n"
		<<"4: focus at a spot near the sample and a spot near the control\n"
		<<"5: focus at 2 spots at the 2 sides of the sample\n"
		<<"6: use the Definite Focus System to stabilize\n";
	cin >>num;
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
	switch (num){
		case 1: 
			scan->sf=NULL;
			break;
		case 2: 
			scan->sf=new PerimeterFocus(cont.currentFocus,minX-scan->getChan(0).out->cam->getImageWidth(scan->getChan(0).ap),minY-scan->getChan(0).out->cam->getImageHeight(scan->getChan(0).ap),maxX+scan->getChan(0).out->cam->getImageWidth(scan->getChan(0).ap),maxY+scan->getChan(0).out->cam->getImageHeight(scan->getChan(0).ap),2,2);
			break;
		case 3: 
			scan->sf=new SpotFocus(cont.currentFocus);
			zpos=FocusInTemp(scan->sf->f,cont.stg->getX(),cont.stg->getY(),cont.focus->getZ(),cont.te.getTemp());
			break;
		case 4: 
			scan->sf=new SpotFocus(cont.currentFocus);
			zpos=FocusInTemp(scan->sf->f,cont.stg->getX(),cont.stg->getY(),cont.focus->getZ(),cont.te.getTemp());
			cout<<"Please select the 2nd sample spot\n";
			scan2=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots2");
			//zpos2=FocusInTemp(scan2->sf->f,cont.stg->getX(),cont.stg->getY(),cont.focus->getZ(),cont.te.getTemp());
			cout<<"Now select the focus spot near the 2nd sample.\n";
			scan2->sf=new SpotFocus(cont.currentFocus);
			//cout<<"Please select the control spot\n";
			//scan3=new ScanSpots(AcquisitionChannel::getAcquisitionChannels(),"scanspots3");
			//cout<<"Now select the focus spot near the control.\n";
			//scan3->sf=new SpotFocus(cont.currentFocus);
			break;
		case 5:
			scan->sf=new PerimeterFocus(1,2);
			break;
#ifdef OBSERVER
		case 6:
			//scan->sf=new DefiniteFocus(scan);
			break;
#endif
	}
#ifdef OBSERVER
	ObserverDefiniteFocus DFS(&(cont.axio));
	DFS.initializeDefiniteFocus();
	DFS.startDefiniteFocus();
#endif
	//Solution *s=cont.pmp.selectReagent();
	//bool prime;
	//cont.pmp.initialize();
	//cout << "Prime fluidics? (1 for yes, 0 for no)\n";
	//cin >> prime;
	//if (prime) {
	//	cont.pmp.primeReagent();
	//}
	float targetTemp;
	targetTemp=80;
//	cont.te.triggerRecording();
	cont.te.setFixedTemp(35);
	Timer::wait(30*1000);
	OutputRampParam *rmp=new OutputRampParam();
	rmp->iniVolt=cont.te.getOutputPower();
	rmp->endVolt=-100;
	rmp->slope=-0.015;
	rmp->targetTemp=targetTemp;
	rmp->timeout=10*60;
	rmp->temod=&(cont.te);
	cout << "Current temperature is " << cont.te.getTemp() << endl;
	cout << "Current output power is " << cont.te.getOutputPower() << endl;
	unsigned threadIDrp, threadIDrd;
	HANDLE hRampThread=(HANDLE) _beginthreadex(NULL, 0, &TEModule::rampOutputThread, rmp, 0, &threadIDrp);
	cont.te.comment="Start temperature ramp.\nInitial voltage: "+toString(rmp->iniVolt)+" End voltage: "+toString(rmp->endVolt)+" Slope: "+toString(rmp->slope)+" Target temperature: "+toString(targetTemp);
	cont.te.triggerRecord=true;

	cont.pmp.sendCommand("IR",0);
	//scan->sf->updateFocus();
	//Timer::wait(60*1000);
#ifdef OBSERVER
	DFS.stopDefiniteFocus();
	DFS.wait();
	if(scan->sf) scan->sf->updateFocus();
	DFS.startDefiniteFocus();
#endif

	//Take pictures in each position in BF when the scan is about to start
	scan->sf->updateFocus();
	cont.currentChannel().takePicture("startscan");
	AcquisitionChannel *ac=new AcquisitionChannel(cont.BF,cont.OUT_CAM,cont.MAG_20x,AcquisitionParameters(.05,15,1),1.35);
	for(int i=0;i<scan->numSpotsGlobal();i++){
		scan->move2NextSpotGlobal(scan->sf);
		cont.stg->wait();
		cont.focus->wait();
//		cont.takePicture(ac,"BFp"+toString(i,3),NULL,false,false,false);
	}

	int scanNum=0;
	bool edgeOfRange=false;
	double tempz,tempT;
	while(cont.te.ramp && scanNum<150){
		//cont.pmp.wait();
		while(cont.te.getTemp()<scanNum*(targetTemp-35)/150+35 && cont.te.ramp){};
		Timer::wait(0);
		if(scan->sf) {
#ifdef OBSERVER	
			DFS.stopDefiniteFocus();
			DFS.wait();
#endif
			scan->sf->updateFocus();
#ifdef OBSERVER
			DFS.startDefiniteFocus();
#endif
			tempz=cont.focus->getZ();
			tempT=cont.te.getTemp();
			recordFile.write(toString(tempz)+"\t"+toString(edgeOfRange)+"\t"+toString(tempT)+"\t"+toString(zpos.getFocusInTemp(tempT)+scan->sf->f.range*0.5-scan->sf->f.step*2)+"\t"+toString(zpos.getFocusInTemp(tempT)-scan->sf->f.range*0.5+scan->sf->f.step*2),true);
			if((tempz>=(zpos.getFocusInTemp(tempT)+scan->sf->f.range*0.5-scan->sf->f.step*2))||(tempz<=(zpos.getFocusInTemp(tempT)-scan->sf->f.range*0.5+scan->sf->f.step*2))){
				if (edgeOfRange){
#ifdef OBSERVER
					DFS.stopDefiniteFocus();
#endif
					scan->sf->f.range=scan->sf->f.range*5;
#ifdef OBSERVER
					DFS.wait();
#endif
					scan->sf->updateFocus();
					scan->sf->f.range=scan->sf->f.range/5;
					tempz=cont.focus->getZ();
					tempT=cont.te.getTemp();
					recordFile.write("new focus : "+toString(tempz),true);
#ifdef OBSERVER
					DFS.startDefiniteFocus();
#endif
					edgeOfRange=false;
				}else {
					edgeOfRange=true;
					//recordFile.write("edgeOfRange is true",true,"ProtocolYing_Ja::TemperatureScan");
				}
			}else edgeOfRange=false;
			zpos.setZeroTempFocus(tempz,tempT);
		}
		scan->runScan(1,scanNum);
		if(num==4){
			scanNum++;
#ifdef OBSERVER	
			DFS.stopDefiniteFocus();
			DFS.wait();
#endif
			if(scan2->sf) scan2->sf->updateFocus();
#ifdef OBSERVER	
			DFS.startDefiniteFocus();
#endif
			scan2->runScan(1,scanNum);
			//scanNum++;
			//if(scan3->sf) scan3->sf->updateFocus();
			//scan3->runScan(1,scanNum);
		}
		//cont.pmp.washChamber(2,50,30,false);
		scanNum++;
	}
	scan->sf->updateFocus();
	for(int i=0;i<scan->numSpotsGlobal();i++){
		scan->move2NextSpotGlobal(scan->sf);
		cont.stg->wait();
		cont.focus->wait();
//		cont.takePicture(ac,"endBFp"+toString(i,3),NULL,false,false,false);
	}
	cont.te.ramp=false;
	//cont.te.setFixedTemp(60,true);
	//cont.te.powerOn();
	string comment;
	comment="start denaturation";
	cont.te.triggerRecord=true;
	logFile.write("scans done. Now denaturing probe",true);
	//denature(s);
	//Timer::wait(5*60*1000);
	cont.te.powerOff();
	cont.pmp.sendCommand("OR",0);
	comment="after denaturation";
	cont.te.triggerRecord=true;
	WaitForSingleObject(cont.te.hRecordAccess,INFINITE);
	cont.te.record=false;
	ReleaseMutex(cont.te.hRecordAccess);
}



