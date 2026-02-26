//process control block
//defining pcb structure and func prototypes 
#ifndef PCB_H
#define PCB_H

typedef struct pcb{
    int pid;
    int start_index; //index where script will start
    int code_len; //to have full location
    int pc;
    int job_score; //used for SJF with Aging

    struct pcb *next; //for ready queue
} PCB;

PCB* make_pcb(int start, int len); //declaration only

int get_pid();

#endif
