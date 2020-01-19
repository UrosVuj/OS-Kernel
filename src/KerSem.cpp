#include "Semaphor.h"
#include "KerSem.h"
#include "Pcb.h"
#include "Timer.h"
#include <iostream.h>

KernelSem::KernelSem(int init){
	//Regular constructor for a Kernel Semaphore

	asm pushf
	asm cli

	int_value = init;
	sem_next = 0;
	first_blocked = 0;
	id=sem_global_id++;
	add_to_list();

	asm popf
}

int KernelSem::wait(Time t){

	/* 
	Blocks the running thread indefinitely if 
	the argument _ Time t _ is 0 and it can only
	be unblocked with the call of signal(int num).

	If the argument is larger than 0, the thread
	can be unblocked either by the use of signal(int num)
	or by simple time passing (t number of ticks)
	*/

	asm pushf
	asm cli

	if(t==0)PCB::running->is_wait_0 = 1;
	else PCB::running->is_wait_0 = 0;

	if(--int_value < 0){
		PCB::running->timeleft_semblock = t;
		PCB::running->state_thread = BLOCKED;
		add_to_blocked_list();
		asm popf

		dispatch();
		if(PCB::running->signaled){
			return 1;
		}
		else {
			return 0;
		}
	}

	else {
	asm popf
	return 1;
	}
}

int KernelSem::signal(int number){
	//deblocks abs(number) threads if possible.

	asm pushf
	asm cli

	if(number < 0){
		asm popf
		return number;
	}

	if(number == 0) number++;
	int number_for_deblocking = number;

	PCB* temp = (PCB*)first_blocked;
	PCB* temp2;
	int_value += number;

	while(temp && number_for_deblocking != 0){
		temp2 = temp;

		number_for_deblocking--;

		temp->state_thread = READY;
		temp->signaled = 1;
		temp->timeleft_semblock = 0;
		put_thread(temp);

		if(temp->next_sem_blocked == 0){
			first_blocked = 0;
			temp = 0;
		}

		else{
			temp = (PCB*)temp->next_sem_blocked;
			temp2->next_sem_blocked = 0;
			first_blocked = temp;
			}

	}
		asm popf
		return number-number_for_deblocking;

}

void KernelSem::add_to_list(){

	 KernelSem* temp = (KernelSem*)All_sem_first;
	 if(!temp)All_sem_first = this;
	 else {
		 while(temp->sem_next)temp = (KernelSem*)temp->sem_next;
		 temp->sem_next = this;
	 }

}

void KernelSem::add_to_blocked_list(){

	 PCB* temp = (PCB*)first_blocked;
	 if(!temp)first_blocked = PCB::running;
	 else {
		 while(temp->next_sem_blocked)
			 temp = (PCB*)temp->next_sem_blocked;
		 temp->next_sem_blocked = PCB::running;
	 }
}

void KernelSem::remove_from_list(){

	 KernelSem* temp_sem = ((KernelSem*)All_sem_first);
	 KernelSem* temp2_sem;
	 if(!temp_sem) return;
	 if(this->id == All_sem_first->id){

	 	 if(temp_sem->sem_next){
			 temp2_sem = (KernelSem*)temp_sem->sem_next;
			 All_sem_first = temp2_sem;
			 return;
		 }
		 else {
			 All_sem_first = 0;
			 return;
		 }
	 }
	 while(temp_sem && temp_sem->id != id)   {
		 temp2_sem = temp_sem;
		 temp_sem = (KernelSem*)temp_sem->sem_next; 
	 }

	 temp2_sem->sem_next = temp_sem->sem_next;
	 return;
}

int KernelSem::value(){
	return int_value;
}

KernelSem::~KernelSem(){
	KernelSem::remove_from_list();

}


volatile KernelSem* KernelSem::All_sem_first = 0;
int KernelSem::sem_global_id = 1;