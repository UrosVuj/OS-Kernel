#include "Thread.h"
#include "Pcb.h"
#include "Timer.h"
#include <iostream.h>

void Thread::start(){
	
	myPCB->start();

}

ID Thread::getId(){

	return myPCB->getId();

}

Thread::~Thread(){

	asm pushf
	asm cli

	delete myPCB;

	asm popf
}

void Thread::waitToComplete(){

	myPCB->waitToComplete();

}

Thread* Thread::getThreadById(ID _id){
	
	PCB* temp = PCB::All_threads_first;
	if(!temp) return 0;
	while(temp && temp->personal_id!=_id)temp=temp->next;
	if(temp)return temp->myThread;
	else return 0;

}

 ID Thread::getRunningId(){

	return PCB::getRunningId();

}

void dispatch(){

	real_dispatch();

}

//When you make a new Thread, it just makes a new PCB and links them
Thread::Thread (StackSize stackSize, Time timeSlice){

	myPCB = new PCB(this,stackSize,timeSlice);
}