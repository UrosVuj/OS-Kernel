#ifndef PCB_H_
#define PCB_H_

#include "Thread.h"

enum State{NEW=1,READY,BLOCKED,ENDED,IDLE,MAIN};


class PCB{

public:


	volatile static ID _id;
	volatile static PCB* running;        //PCB of the running thread, global variable
	volatile static int numberofthreads; // Number of threads that aren't finished
	volatile State state_thread;


	unsigned sp;
	unsigned ss;
	unsigned bp;
	volatile	unsigned ended; //1 if Thread ended its job, 0 otherwise
	volatile	int time;       //Amount of time the thread can hold the CPU
	unsigned *stack;

	volatile ID personal_id;  //ID of a certain thread



	volatile PCB* Blocked_first;		//pointer to the first in the list of blocked threads
	volatile PCB* Blocked_last;      //due to waitToComplete();

	volatile PCB* next_blocked;            // next in the waitToComplete()
   	   	   	   	   	   	   	   	   	   	  //list, starts with 0




	volatile PCB* next_sem_blocked;       //fields used by the semaphore class
	volatile Time timeleft_semblock;
	volatile int signaled;
	volatile int is_wait_0;



	static PCB* All_threads_first;
	PCB* next;


  
	PCB (Thread* myThread, StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);
	PCB();
	PCB (void (*body)() );   //constructor for Idle thread
  

	friend class Thread;
	Thread* myThread;


    void start();

    void fill(void (*body)(), StackSize size);



    void waitToComplete();
    void block_add();	//helper for waitToComplete()
    					//for adding running thread to
    					//the called threads blocked list


    void exitThread();
    void remove_from_list();
    void deblock_threads();


    virtual ~PCB();


    ID getId();
    static ID getRunningId();
    static Thread* getThreadById(ID id);


    static void wrapper();

			};

void idle();


#endif /* PCB_H_ */
