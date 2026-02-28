//declaring operations on ready queue (fifo queue of ready processes)
#ifndef READYQUEUE_H
#define READYQUEUE_H

#include "pcb.h"

void enqueue(PCB *proc); //adding pcb to end of ready queue
PCB* dequeue(); //removing first pcb from queue
int is_empty(); //check to see if queue is empty
void enqueue_sjf(PCB *proc);
void enqueue_aging(PCB *proc, int mode);
void age_ready_queue();
void enqueue_head(PCB *proc);

#endif
