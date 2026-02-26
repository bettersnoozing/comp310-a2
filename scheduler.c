//implementing shceduler - fcfs (non preemptive)

// while ready queue is not empty, take head process
// run it to completion (execute all lines)
// after finishing, free its code and pcb and move to next process in queue

#include "scheduler.h"
#include "readyqueue.h"
#include "shellmemory.h"
#include "interpreter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RR_QUANTUM 2 //number of instructions per RR time slice

void scheduler(Policy policy){
    while (!is_empty()){
        PCB *cur = dequeue(); //get head process

	if(policy == RR_POLICY) {
		//RR: run up to 2 instructions
		int instructions_to_run = RR_QUANTUM;
		while(cur->pc < cur->code_len && instructions_to_run > 0) {
			char *line = get_line(cur->start_index + cur->pc);
			if(line) {
				char *copy = strdup(line); //copy to avoid modification
				if(copy) {
					parseInput(copy); //execute line
					free(copy);
				}
			}

			cur->pc++; //advacne the program counter
			instructions_to_run--; //decrement quantum
		}

		if(cur->pc < cur->code_len) {
			enqueue(cur); //RR uses FCFS for cycling
		} else {
			//process is finished so free code and PCB
			free_lines(cur->start_index, cur->code_len);
			free(cur);
		}
	} else {

	        while (curr->pc < curr->code_len){
        	    char *line = get_line(curr->start_index + curr->pc); //get line of code to execute
           	 	if (line == NULL){
                		curr->pc++;
                		continue;
            		}

            		char *copy = strdup(line); //making a copy to pass to parseInput to prevent modifications
            		if (copy){ 
                		parseInput(copy); 
            			free(copy);
            		}
          		curr->pc++; //move to next line
        	}
        free_lines(curr->start_index, curr->code_len); //free code lines after finishing process
        free(curr); //free pcb
    }
}
