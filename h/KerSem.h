#ifndef KERSEM_H_
#define KERSEM_H_


#include "Pcb.h"
#include "Semaphor.h"

class KernelSem{

public:

	KernelSem(int init);
	~KernelSem();

	volatile int int_value;

	volatile KernelSem* sem_next;  //Next Semaphore in the list off all Semaphores
	volatile PCB* first_blocked;   //First thread blocked by this Semaphore


	volatile static KernelSem* All_sem_first;
	static int sem_global_id;
	int id;


	int wait(Time t);
	int signal(int number);

	void add_to_list();
	void add_to_blocked_list();
	void remove_from_list();
	int value();

};

#endif /* KERSEM_H_ */
