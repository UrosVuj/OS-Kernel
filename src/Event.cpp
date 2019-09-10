#include "Event.h"
#include "Timer.h"
#include "Thread.h"
#include "Pcb.h"
#include <dos.h>


IVTEntry* IVTEntry::all_entries[256];

Event::Event(IVTNo ivtno){
	myImpl= new KernelEv((PCB*)PCB::running,ivtno);
}

Event::~Event(){
	delete myImpl;
}

void Event::wait(){
	myImpl->wait();
}

void Event::signal(){
	myImpl->signal();
}

IVTEntry::IVTEntry(IVTNo ivtno, InterruptPointer pointer){
	asm pushf
	asm cli

	ivtNo=ivtno;
	all_entries[ivtno]=this;
	new_int_pointer=pointer;
	event=0;

	asm popf
}

void IVTEntry::signal(){
	event->signal();
}

KernelEv::KernelEv(PCB* holder, IVTNo number){
	asm pushf
	asm cli

	holder_thread=holder;
	ivtno=number;
	value=0;
	IVTEntry::all_entries[ivtno]->event=this;
	inic();

	asm popf
}

void KernelEv::inic(){

(IVTEntry::all_entries[ivtno])->old_int_pointer = getvect(ivtno);
setvect(ivtno, (IVTEntry::all_entries[ivtno])->new_int_pointer);

}

void KernelEv::restore(){

	asm pushf
	asm cli

	setvect(ivtno, (IVTEntry::all_entries[ivtno])->old_int_pointer);

	asm popf
}

KernelEv::~KernelEv(){
	restore();
}


void KernelEv::signal(){
	asm pushf
	asm cli


	if(value++<0){
		holder_thread->state_thread=READY;
		put_thread(holder_thread);
	}
	asm popf
}

void KernelEv::wait(){
	asm pushf
	asm cli

	if(PCB::running==holder_thread){
		if(--value<0){
			PCB::running->state_thread=BLOCKED;
			dispatch();
		}
	}
	asm popf
}