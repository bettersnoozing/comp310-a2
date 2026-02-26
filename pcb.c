//implements pcb functions

#include "pcb.h"
#include <stdlib.h>

static int next_pid = 1;

//returning new pid 
int get_pid() {
    return next_pid++;
}

//creating new pcb for script at code_start
PCB* make_pcb(int start, int len){
    PCB *p = (PCB*) malloc(sizeof(PCB)); //allocating memory for pcb
    if(!p) return NULL;
    p->pid = next_pid++; 
    p->start_index = start; 
    p->code_len = len; //nb of lines
    p->pc = 0; //starting line of script
    p->job_score = len; //job length = initial score
    p->next = NULL;
    return p;
}
