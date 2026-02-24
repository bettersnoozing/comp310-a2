//implementing fifo queue that was declared in readyqueue.h

#include "readyqueue.h"
#include <stdlib.h>

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


