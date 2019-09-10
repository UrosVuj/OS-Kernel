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
	cout << "Operating Systems Project " << endl <<"Student: Uros Vujosevic"<< endl <<"Student ID: 0209/2017"<< endl;



	inic();

	return_value=userMain(argc, argv);

	restore();                    

	cout<<"End."<<endl;


	return return_value;


};