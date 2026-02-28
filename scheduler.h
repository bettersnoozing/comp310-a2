//scheduler declaration
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "pcb.h"

extern int mt_enabled;
extern int scheduler_running;

//Scheduler policies
typedef enum {
	FCFS_POLICY,
	SJF_POLICY,
	RR_POLICY,
	AGING_POLICY,
	RR30_POLICY
} Policy;


void scheduler(Policy policy); //starts executing processes from the ready queue until empty

#endif
