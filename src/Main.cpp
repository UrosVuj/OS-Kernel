#include <iostream.h>
#include <dos.h>
#include "SCHEDULE.H"
#include "Thread.h"
#include "Pcb.h"
#include "Timer.h"
#include "Semaphor.h"
#include "Event.h"

int return_value;
extern int userMain (int argc, char* argv[]);


extern PCB* Main_thread;


int main(int argc, char* argv[]){


	PCB::running=Main_thread;
	cout << "Kernel Demonstration!" << endl <<"The demonstration consists of displaying the famous Producers/Consumers concurrency problem"<< endl
		<<"The user can modify the number of threads which work as producers or consumers and the size of the buffer in which the produce is put"<< endl<<endl<<endl;

	inic();

	return_value=userMain(argc, argv);

	restore();                    

	cout<<"End."<<endl;


	return return_value;


};