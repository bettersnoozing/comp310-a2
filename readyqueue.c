//implementing fifo queue that was declared in readyqueue.h

#include "readyqueue.h"
#include <stdlib.h>
#include <stdio.h>

static PCB *head = NULL;
static PCB *tail = NULL;

//adding pcb to end of ready queue
void enqueue(PCB *process){
    if (process == NULL){ //if it doesn't exist yet
        return;
    }
    process->next = NULL; //bc it's the end of the queue

    if (head==NULL){
        head = process; //if queue is empty, head and tail are the same
        tail = process;
    } 
    else {
        tail->next = process; //link curr tail to new process
        tail = process; 
    }

}

//SJF enqueue: insert process sorted by code_len. Shortest job first
void enqueue_sjf(PCB *process) {

	if(!process) return;

	//insert at head if the queue is empty or the job is the shortest
	if(!head || process->code_len < head->code_len) {
		process->next = head;
		head = process;
		if(!tail) tail = head;
		return;
	}

	//traverse the queue in order to find the insertion point
	PCB *cur = head;
	while(cur->next && cur->next->code_len <= process->code_len) {
		cur = cur->next;
	}

	process->next = cur->next;
	cur->next = process;

	//update the tail if inserted at the end
	if(!process->next) tail = process;
}

//insert sorted by job score with the lowest first
void enqueue_aging(PCB *process) {

	if(!process) return;
	if(!head || process->job_score < head->job_score) {
		process->next = head;
		head = process;
		if (!tail) tail = head;
		return;
	}

	PCB *cur = head;

	//match example tie-breaking
	while (cur->next && cur->next->job_score <= process->job_score) {
		cur = cur->next;
	}

	process->next = cur->next;
	cur->next = process;

	if(!process->next) {
		tail = process;
	}
}

//decrease score of all jobs in queue except the running one
void age_ready_queue() {

	PCB *cur = head;
	while(cur) {
		if(cur->job_score > 0) cur->job_score--;
		cur = cur->next;
	}
}

PCB* dequeue(){
    if (head == NULL){ //if queue empty
        return NULL; 
    }
    PCB *process = head; //getting head
    head = head->next; //update head to second process
    if (head == NULL){ //case where dequeueing make queue empty
        tail = NULL;
    }
    process->next = NULL; //remove process from queue

    return process; 
}

//returns 1 if empty, 0 if not
int is_empty(){
    return head == NULL; 
}


