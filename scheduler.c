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
#include <pthread.h>

#define RR_QUANTUM 2 //number of instructions per RR time slice
#define RR30_QUANTUM 30 //new for 1.2.5

int mt_enabled = 0; // default single-threaded

//worker thread function for MT scheduler
void *worker_thread(void *arg) {
    Policy policy = *(Policy *)arg;

    while (!is_empty()) { //while ready queue has processes
        PCB *cur = dequeue();
        if (!cur) continue;

        int quantum = (policy == RR30_POLICY) ? RR30_QUANTUM : RR_QUANTUM;

        for (int i = 0; i < quantum && cur->pc < cur->code_len; i++) {
            char *line = get_line(cur->start_index + cur->pc);
            if (line) {
                char *copy = strdup(line); //avoid modifying original code
                if (copy) {
                    parseInput(copy); //execute instruction
                    free(copy);
                }
            }
            cur->pc++;
        }

        //requeue if process not finished
        if (cur->pc < cur->code_len) enqueue(cur);
        else { //finished so free code lines and PCB
            free_lines(cur->start_index, cur->code_len);
            free(cur);
        }
    }

    return NULL;
}

void run_process(PCB *cur, Policy policy) {
	 if(policy == RR_POLICY || policy == RR30_POLICY) {
            //Decide the number of instructions to execute this turm
            int instructions_to_run = (policy == RR_POLICY) ? RR_QUANTUM : RR30_QUANTUM;

	   //execute instructions up to the quantum or until the process finishes
            while(cur->pc < cur->code_len && instructions_to_run > 0) {
                char *line = get_line(cur->start_index + cur->pc);
                if(line) {
                    char *copy = strdup(line); //copy to avoid modifications
                    if(copy) {
                        parseInput(copy); //execute the line
                        free(copy);
                    }
                }
                cur->pc++; //advance program counter
                instructions_to_run--; //decrement quantum
            }

            if(cur->pc < cur->code_len) {
                enqueue(cur); // not finished, put back in queue. RR uses FCFS for cycling
            } else {
                //process is finished so free code and PCB
                free_lines(cur->start_index, cur->code_len);
                free(cur);
            }
        }
        else if(policy == AGING_POLICY) {
            // Aging SJF: run 1 instruction per time slice
            if(cur->pc < cur->code_len) {
                char *line = get_line(cur->start_index + cur->pc);
                if(line) {
                    char *copy = strdup(line);
                    if(copy) {
                        parseInput(copy);
                        free(copy);
                    }
                }
                cur->pc++;
            }

            // check if process finished
            if(cur->pc >= cur->code_len) {
                free_lines(cur->start_index, cur->code_len);
                free(cur);
            } else {
                // age all other processes in queue
                age_ready_queue();

                // reinsert current job using sorted insert
                enqueue_aging(cur, 1);
            }
        }
        else {
            while(cur->pc < cur->code_len) {
                char *line = get_line(cur->start_index + cur->pc); //get line of code to execute
                if(line == NULL) {
                    cur->pc++;
                    continue;
                }

                char *copy = strdup(line); //making a copy to pass to parseInput to prevent modifications
                if(copy) {
                    parseInput(copy);
                    free(copy);
                }
                cur->pc++; //move to next line
            }

            free_lines(cur->start_index, cur->code_len); //free code lines after finishing process
            free(cur); //free pcb
        }
}

void scheduler(Policy policy) {
    if (mt_enabled && (policy == RR_POLICY || policy == RR30_POLICY)) {
        // Multi-threaded scheduler
        pthread_t threads[2];
        for (int i = 0; i < 2; i++) {
            //pass policy to worker threads or a struct with shared ready queue
            pthread_create(&threads[i], NULL, worker_thread, &policy);
        }
        //wait for both threads to finish
        for (int i = 0; i < 2; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        //single-threaded scheduler
        while(!is_empty()) {
            PCB *cur = dequeue();
            run_process(cur, policy); //helper function, same code as before
        }
    }
}


