// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#include "Warn.h"
#include <process.h>
#include <windows.h>

class Thread{
private:
	static HANDLE modify;
	HANDLE thread;
	Thread* next;
	Thread* prev;

public:
	Thread();
	Thread* addThread(HANDLE thread);
	static void removeThread(Thread* thread);
	static void stopAll();
};

Thread::modify=CreateEvent(