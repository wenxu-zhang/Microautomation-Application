// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================


void testStage(){
	focus->move(0);
	focus->wait();
	filt->closeShutter();
	int startX=stg->getX();
	int startY=stg->getY();
	//clock_t start=clock(),finish;
	int x20=-cam->getPixelSize()*1004/(stg->getStepSize()*20);
	int y20=-cam->getPixelSize()*1002/(stg->getStepSize()*20);
	int x40=-cam->getPixelSize()*1004/(stg->getStepSize()*40);
	int y40=-cam->getPixelSize()*1002/(stg->getStepSize()*40);
	int x60=-cam->getPixelSize()*1004/(stg->getStepSize()*63);
	int y60=-cam->getPixelSize()*1004/(stg->getStepSize()*63);
	Timer t;
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incX(x20);
		stg->wait();
	}
	t.stopTimer();
	//finish=clock();
	
	cout<<"100 movements of 1Kx1K field at 20xMag in x direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incX(x40);
		stg->wait();
	}
	t.stopTimer();
	//finish=clock();
	cout<<"100 movements of 1Kx1K field at 40xMag in x direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incX(x60);
		stg->wait();
	}
	t.stopTimer();
	//finish=clock();
	cout<<"100 movements of 1Kx1K field at 63xMag in x direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY(y20);
		stg->wait();
	}
	t.stopTimer();
	//finish=clock();
	cout<<"100 movements of 1Kx1K field at 20xMag in y direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY(y40);
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 40xMag in y direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY(y60);
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 63xMag in y direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;


	//cout<<"PLEASE PLACE DEVICE ON STAGE"<<endl;
	//system("pause");
	
	cout<<"incremental moves"<<endl;
	t.resetTimer();
	stg->setXinc(x20);
	t.startTimer();
	
	for (int i=0; i<100;i++){
		stg->incX();
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 20xMag in x direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	stg->setXinc(x40);
	t.startTimer();
	
	for (int i=0; i<100;i++){
		stg->incX();
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 40xMag in x direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	stg->setXinc(x60);
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incX();
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 63xMag in x direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	stg->setYinc(y20);
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY();
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 20xMag in y direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	stg->setYinc(y40);
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY();
		stg->wait();
	}
	t.stopTimer();
	cout<<"100 movements of 1Kx1K field at 40xMag in y direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	stg->move(startX,startY);
	stg->wait();
	t.resetTimer();
	stg->setYinc(y60);
	t.startTimer();
	for (int i=0; i<100;i++){
		stg->incY();
		stg->wait();
	}
	t.stopTimer();
	stg->move(startX,startY);
	cout<<"100 movements of 1Kx1K field at 63xMag in y direction with DEVICE took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;

}

void testFocus(){
	filt->closeShutter();
	focus->move(0);
	focus->wait();
	Timer t;
	//clock_t start=clock(),finish;
	t.startTimer();
	for (int i=0; i<100;i++){
		focus->inc(.5);
		//cout<<"current z pos is: "<<focus->getPos()<<endl;
		focus->wait();
	}
	
	t.stopTimer();
	cout<<"focus started at 0 ended at: "<<focus->getPos()<<endl;
	//finish=clock();
	cout<<"100 movements of .5um in Z direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	focus->move(0);
	focus->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		focus->inc(1.0);
		//cout<<"current z pos is: "<<focus->getPos()<<endl;
		focus->wait();
	}
	t.stopTimer();
	cout<<"focus started at 0 ended at: "<<focus->getPos()<<endl;
	//finish=clock();
	
	cout<<"100 movements of 1um in Z direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	focus->move(0);
	focus->wait();
	t.resetTimer();
	t.startTimer();
	for (int i=0; i<100;i++){
		focus->inc(10.0);
		//cout<<"current z pos is: "<<focus->getPos()<<endl;
		focus->wait();
	}
	t.stopTimer();
	cout<<"focus started at 0 ended at: "<<focus->getPos()<<endl;
	//finish=clock();
	cout<<"100 movements of 10um in Z direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;
	t.resetTimer();
	focus->move(0);
	focus->wait();
	t.startTimer();
	for (int i=0; i<50;i++){
		focus->inc(100.0);
		//cout<<"current z pos is: "<<focus->getPos()<<endl;
		focus->wait();
	}
	t.stopTimer();
	cout<<"focus started at 0 ended at: "<<focus->getPos()<<endl;
	focus->move(0);
	focus->wait();
	t.startTimer();
	for (int i=0; i<50;i++){
		focus->inc(100.0);
		//cout<<"current z pos is: "<<focus->getPos()<<endl;
		focus->wait();
	}
	t.stopTimer();
	cout<<"focus started at 0 ended at: "<<focus->getPos()<<endl;
	cout<<"100 movements of 100um in Z direction took: "<<t.getTime()/1000<<" seconds.  Approx "<<t.getTime()/1000/100<< "second per move"<<endl;

}