//scheduler declaration
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "pcb.h"

//Scheduler policies
typedef enum {

	FCFS_POLICY
	SJF_POLICY
	RR_POLICY
} Policy;


void scheduler(Policy policy); //starts executing processes from the ready queue until empty

#endif
