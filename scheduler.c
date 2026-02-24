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

void scheduler(){
    while (!is_empty()){
        PCB *curr = dequeue(); //get head process

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
