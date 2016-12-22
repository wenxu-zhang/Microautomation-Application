// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Warn.h"
#include "CImg.h"

using namespace cimg_library;

class DisplayList{

public:
	class Display{
		friend class DisplayList;
	public:
		
		Display(CImgDisplay* disp):disp(disp),next(NULL),prev(NULL){}
	private:
		CImgDisplay* disp;
		Display* next;
		Display* prev;

		void closeDisplay(){
			disp->close();
		}

		void removeDisplay(){
			prev->next=next;
			if (next) next->prev=prev;
		}
	}; 

	DisplayList():head(new Display(NULL)){
		modify=CreateEvent(NULL,FALSE,TRUE,NULL);
	}
	~DisplayList(){
		delete head;
	}
	Display* addDisplay(CImgDisplay* disp){
		WaitForSingleObject(modify,INFINITE);
		Display* d=new Display(disp);
		d->next=head->next;
		d->prev=head;
		head->next=d;
		if (d->next)
			d->next->prev=d;
		SetEvent(modify);
		return d;
	}
	void removeDisp(Display* d){
		WaitForSingleObject(modify,INFINITE);
		d->removeDisplay();
		delete d;
		SetEvent(modify);
	}
	void closeAll(){
		WaitForSingleObject(modify,INFINITE);
		Display* temp=head->next;
		while(temp){
			temp->closeDisplay();
			temp=temp->next;
		}
		SetEvent(modify);
	}

private:
	Display* head;
	HANDLE modify;

};