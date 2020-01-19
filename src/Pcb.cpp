#include <iostream.h>
#include <dos.h>
#include "Pcb.h"
#include "Timer.h"
#include "SCHEDULE.H"

extern int context_change;

	//Regular constructor for the  PCB class
	//used for creating the PCB structure for regular,
	//user-made Threads.
	PCB::PCB (Thread* thread, StackSize _stackSize, Time timeSlice){

		asm pushf
		asm cli

		personal_id=_id++;
		next_sem_blocked = 0;
		signaled = 0;

		state_thread = NEW;

		Blocked_first = 0;
		Blocked_last = 0;
		next_blocked = 0;

		if(!All_threads_first){
			All_threads_first = this;
		}
		else {
			PCB* temp_;
			temp_ = All_threads_first;
			while(temp_->next)temp_ = temp_->next;
			temp_->next = this;
		}

		next = 0;
		ended = 0;
		time = timeSlice;
		myThread = thread;

		fill(PCB::wrapper,_stackSize);

		asm popf

	}


	//Constructor used for creating
	//the Main thread.
	PCB::PCB(){

		asm pushf
		asm cli


		state_thread=MAIN;
		next_blocked=0;
		next=0;
		next_sem_blocked=0;
		signaled=0;

		ended=0;
		time=2;
		personal_id=-1;
		myThread=0;

		asm popf

	}



	//Constructor used only
	//for creating the IDLE thread
	PCB::PCB(   void (*body)()    ){

		asm pushf
		asm cli

		ended = 0;

		time=1;
		state_thread = IDLE;
		personal_id = 0;
		myThread = 0;
		Blocked_first = 0;
		Blocked_last = 0;
		next = 0;
		next_blocked = 0;
		next_sem_blocked = 0;
		signaled = 0;

		fill(body, defaultStackSize);

		asm popf

	}


	void PCB::fill(void (*body)(), StackSize size  ){

		//Initialises the stack of a thread

		stack = new unsigned[size/sizeof(unsigned int)];
		stack[size-1] = 0x200
		#ifndef BCC_BLOCK_IGNORE
		stack[size - 2] = FP_SEG(body);
		stack[size - 3] = FP_OFF(body);
		sp = FP_OFF(stack + size - 12);
		ss = FP_SEG(stack + size - 12);
		bp = FP_OFF(stack + size - 12);
		#endif

	}


	ID PCB::getId(){
		return this->personal_id;
	}

	Thread* PCB::getThreadById(ID func_id){
		asm pushf
		asm cli

		PCB* temp=All_threads_first;
		if(!temp)return 0;
		while(temp && temp->personal_id!=func_id)temp=temp->next;
		if(temp) return temp->myThread;
		else return 0;

		asm popf

	}

	ID PCB::getRunningId(){
		return ((PCB*)PCB::running)->personal_id;
	}

	void PCB::wrapper(){

		/*
		Acts as the regular Thread::run() function.
		It gets put on the stack where run() should be
		*/

		((PCB*)PCB::running)->myThread->run();
		((PCB*)PCB::running)->exitThread();

	}

	void PCB::start(){

		asm pushf
		asm cli

		if(this->state_thread == NEW)this->state_thread = READY;
		numberofthreads++;
		put_thread(this);
		context_change=1;

		asm popf


	}


	void PCB::exitThread(){

		/*
		Called at the end of the wrapper function
		after the thread finishes its run() method.
		Does thecleanup needed when a thread finishes
		its lifetime
		*/
		asm pushf
		asm cli

		PCB::running->ended=1;
		numberofthreads--;
		PCB::running->state_thread=ENDED;

		deblock_threads();
		remove_from_list();

		asm popf
		dispatch();


	}

	PCB::~PCB(){
		asm pushf
		asm cli
		delete[] stack;
		asm popf

	}


	void PCB::block_add(){

		/*
		adds the running thread to the list
		of threads waiting for the completion
		of the callee thread
		*/

		if(Blocked_first == 0){
			Blocked_last = (PCB*)PCB::running;
			Blocked_first = Blocked_last;
		}
		else{
			Blocked_last->next_blocked = (PCB*)PCB::running;
			Blocked_last = Blocked_last->next_blocked;
		}
	}

	
	void PCB::waitToComplete(){
		/*
		if the thread on which waitToComplete()
		is called on isn't done, blocks the thread
		otherwise it just passes through. 
		*/
		asm pushf
		asm cli

		if(this->ended == 0){
			running->state_thread = BLOCKED;
			block_add();
			dispatch();
		}
		asm popf
	}

	
	void PCB::remove_from_list(){

		// removes the thread from the list of "living" threads. 

		PCB* temp_thread = All_threads_first;
		PCB* temp2_thread;
		if(!temp_thread) return;
		if(PCB::running->personal_id == All_threads_first->personal_id){

			if(temp_thread->next){
			temp2_thread = temp_thread->next;
			All_threads_first = temp2_thread;
			return;
			}

			else {
			All_threads_first = 0;
			return;
			}

		}
		while(temp_thread && temp_thread->personal_id != personal_id)   {
			temp2_thread = temp_thread;
			temp_thread = temp_thread->next;
		}
		temp2_thread->next = temp_thread->next;
		return;

	}


	void PCB:: deblock_threads(){

		/* 
		Deblocks all threads who used
		waitToComplete() on the this thread
		used in the exitThread() function
		which is called right after
		the thread finishes its run() method.
		*/

		volatile PCB* temp = Blocked_first;
		volatile PCB* temp2;
		if(!temp)return;
		while(temp){
			temp->state_thread = READY;
			temp2 = temp;
			put_thread((PCB*)temp);
			temp = temp->next_blocked;
			temp2->next_blocked = 0;
		}
		Blocked_first = 0;
		Blocked_last = 0;

	}


	void idle(){

		/* 
		Function used as the run method in the Idle thread.
		call of the dispatch() method is not needed but 
		with the nature of this kernel, event based situations
		have a better response time with the instantaneous call
		of dispatch()
		*/

		while(PCB::numberofthreads){
				dispatch();
		}
	}
	 volatile ID PCB::_id = 1;
	 volatile PCB* PCB::running;
	 volatile int PCB::numberofthreads = 0;
	 PCB* PCB::All_threads_first = 0;