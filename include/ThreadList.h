// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#pragma once
#include "Warn.h"
#include <process.h>
#include <windows.h>

class ThreadList{
public:
	class Thread{
		friend class ThreadList;
	public:
		Thread(HANDLE t):t(t),next(NULL),prev(NULL){}
	private:
		HANDLE t;
		Thread* next;
		Thread* prev;
		void stopThread(){
			if (WAIT_TIMEOUT==WaitForSingleObject(t,0))
				TerminateThread(t,0);
		}
		void waitThread(){
			WaitForSingleObject(t,INFINITE);
		}
		void removeThread(){
			CloseHandle(t);
				prev->next=next;
			if (next) next->prev=prev;
		}
	};
	
	ThreadList():head(new Thread(NULL)){
		modify=CreateMutex(NULL,FALSE,NULL);
	}
	~ThreadList(){
		delete head;
	}
	Thread* addThread(HANDLE thread){
		WaitForSingleObject(modify,INFINITE);
		Thread* t=new Thread(thread);
		t->next=head->next;
		t->prev=head;
		if (head->next)	head->next->prev=t;
		head->next=t;
		ReleaseMutex(modify);
		return t;
	}	
	void removeThread(Thread* t){
		WaitForSingleObject(modify,INFINITE);
		t->removeThread();
		delete t;
		ReleaseMutex(modify);
	}
	void waitThread(Thread* t){
		WaitForSingleObject(modify,INFINITE);
		t->waitThread();
		t->removeThread();
		delete t;
		ReleaseMutex(modify);
	}
	void stopAll(){
		WaitForSingleObject(modify,INFINITE);
		Thread *temp;
		while (head->next){
			head->next->stopThread();
			head->next->removeThread();
		}
		ReleaseMutex(modify);
	}
	void waitAll(){
		WaitForSingleObject(modify,INFINITE);
		Thread* temp;
		while (head->next){
			head->next->waitThread();
			head->next->removeThread();
		}
		ReleaseMutex(modify);
	}

	private:
	Thread* head;//dummy header, does not contain any information
	HANDLE modify;
};

