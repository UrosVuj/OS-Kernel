#ifndef TIMER_H_
#define TIMER_H_

#include "Pcb.h"

void put_thread(PCB* pcb);
PCB* get_thread();

void interrupt timer();

void inic();
void restore();

void real_dispatch();

void tick_semaphores();

#endif /* TIMER_H_ */
