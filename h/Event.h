

#ifndef EVENT_H_
#define EVENT_H_

#include "Pcb.h"



typedef unsigned char IVTNo;
typedef void interrupt (*InterruptPointer)(...);
class KernelEv;

class Event{
public:
	Event(IVTNo ivtNo);
	~Event();

	void wait();

protected:
	friend class KernelEv;
	void signal();  //can call KernelEv

private:
	KernelEv* myImpl;
};


class IVTEntry{
public:
	static IVTEntry* all_entries[256];
	InterruptPointer old_int_pointer;
	InterruptPointer new_int_pointer;
	IVTNo ivtNo;
	KernelEv* event;

	IVTEntry(IVTNo ivtNo, InterruptPointer pointer);
	void signal();

};

class KernelEv{
public:
	int value;
	IVTNo ivtno;
	PCB* holder_thread;
	void inic();
	void restore();
	void signal();
	void wait();

	KernelEv(PCB* holder, IVTNo number);
	~KernelEv();


};

#define PREPAREENTRY(numEntry,callOld)\
	void interrupt inter##numEntry(...);\
	IVTEntry newEntry##numEntry(numEntry,inter##numEntry);\
	void interrupt inter##numEntry(...){\
		newEntry##numEntry.signal();\
		if(callOld==1)\
			newEntry##numEntry.old_int_pointer();\
	}

#endif /* EVENT_H_ */
