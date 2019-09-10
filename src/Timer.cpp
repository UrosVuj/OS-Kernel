#include <iostream.h>
#include <dos.h>
#include "SCHEDULE.H"
#include "Pcb.h"
#include "KerSem.h"
#include "Timer.h"




PCB* Main_thread = new PCB();
PCB* Idle_thread = new PCB(idle);




// helper variables for context change in timer();
volatile unsigned tsp;
volatile unsigned tbp;
volatile unsigned tss;
unsigned oldTimerOFF, oldTimerSEG; // old interrupt routine


extern void tick();



volatile int global_timer=5;       			  //Global timer which ticks how much time is left for the current thread

volatile int context_change = 0;              //flag that determines if there will or won't
											  // be a change of context

volatile unsigned lockFlag=1;


void put_thread(PCB* pcb){
	asm pushf
	asm cli
	if(pcb->state_thread==READY)Scheduler::put(pcb);
	asm popf
}





PCB* get_thread(){              //function that can return a thread even if
								// there is no threads in the scheduler.
								// ** returns the main thread or the idle thread
								// in that case

	asm pushf
	asm cli

	PCB* temp_thread=Scheduler::get();
	if(temp_thread){
		asm popf
		return temp_thread;

	}
	else if(PCB::numberofthreads) {
		asm popf
		return Idle_thread;
	}
	asm popf
	return Main_thread;

}











void interrupt timer(){




	int pom;
	if(context_change==0)pom=1;
	else pom=0;

	if(pom) {
	tick_semaphores();
	tick();
	asm int 60h

}

	if (!context_change && global_timer!=-1) global_timer--;
	if (global_timer == 0 || context_change) {
		if(lockFlag){
		context_change=0;
		asm {

			// saves the pointers needed for stack manipulation

			mov tbp, bp
			mov tsp, sp
			mov tss, ss
		}

		PCB::running->sp = tsp;
		PCB::running->bp = tbp;
		PCB::running->ss = tss;


		if(PCB::running && PCB::running->ended==0 && PCB::running->state_thread!=BLOCKED  && PCB::running->state_thread!=IDLE && PCB::running->state_thread!=MAIN){
			put_thread((PCB*)PCB::running);
		}

		PCB::running=(PCB*)get_thread();

		tsp = PCB::running->sp;
		tss = PCB::running->ss;
		tbp = PCB::running->bp;

		global_timer = PCB::running->time;
		if(PCB::running->time==0)global_timer--;
		asm {
			mov sp, tsp   // restores the above mentioned pointers
			mov ss, tss
			mov bp, tbp
		}
	}
		else context_change=1;

	}




}


void inic(){

	// sets the new interrupt routine in a location that used
	// to be occupied by a different routine

	asm{
		cli
		push es
		push ax

		mov ax,0   // initialises the timer routine
		mov es,ax

		mov ax, word ptr es:0022h // remembers the old routine
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax

		mov word ptr es:0022h, seg timer //  sets the new routine
		mov word ptr es:0020h, offset timer 

		mov ax, oldTimerSEG		  //	places the old routine at
		mov word ptr es:0182h, ax //    the 60h memory location
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}


void real_dispatch(){
	asm cli
		context_change = 1;
	asm int 8h
	asm sti
}


void restore(){

	// returns the old interrupt routine in its initial memory location

	asm {
		cli
		push es
		push ax

		mov ax,0
		mov es,ax


		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}

void tick_semaphores(){

	/* 
	Helper routine which is called every 55ms

	Used for ticking the timers in all of the threads which are time-blocked
	The function ignores the threads which were blocked in the signal-only deblocking
	way.

	Goes through the lists of blocked threads in all of the semaphores
	which are currently alive and have threads blocked.

	*/


	KernelSem* temp_sem=(KernelSem*)KernelSem::All_sem_first;
	if(!temp_sem){
		
		return;

	}
	while(temp_sem){

		PCB* temp_first=(PCB*)temp_sem->first_blocked;
		if(temp_first){




		PCB* temp1=(PCB*)temp_first->next_sem_blocked;
		PCB* temp2=temp_first;
		PCB* temp_del;

		while(temp1){

			temp1->timeleft_semblock--;
			if(temp1->timeleft_semblock==0 && temp1->is_wait_0==0){

				temp_del=temp1;
				temp1->state_thread=READY;
				temp1->signaled=0;
				put_thread((PCB*)temp1);
				temp2->next_sem_blocked=temp1->next_sem_blocked;
				temp1=(PCB*)temp1->next_sem_blocked;
				temp_del->next_sem_blocked=0;

			}
			else{
				temp1=(PCB*)temp1->next_sem_blocked;
				temp2=(PCB*)temp2->next_sem_blocked;
			}

			}
		temp_first->timeleft_semblock--;
		if(temp_first->timeleft_semblock==0 && temp_first->is_wait_0==0){


			temp_first->state_thread=READY;
			put_thread((PCB*)temp_first);
			temp_first->signaled=0;

			temp_sem->first_blocked=temp_first->next_sem_blocked;
			temp_first->next_sem_blocked=0;
		}



		}

		temp_sem=(KernelSem*)temp_sem->sem_next;


	}

}






