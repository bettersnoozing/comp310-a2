//declaring operations on ready queue (fifo queue of ready processes)
#include "pcb.h"

void enqueue(PCB *proc); //adding pcb to end of ready queue
PCB* dequeue(); //removing first pcb from queue
int is_empty(); //check to see if queue is empty