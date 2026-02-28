//implementing fifo queue that was declared in readyqueue.h

#include "readyqueue.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

extern PCB *head;
extern PCB *tail;
extern pthread_mutex_t queue_mutex;

//adding pcb to end of ready queue
void enqueue(PCB *process){
    pthread_mutex_lock(&queue_mutex);
    if (process == NULL){ //if it doesn't exist yet
        pthread_mutex_unlock(&queue_mutex);
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
    pthread_mutex_unlock(&queue_mutex);
}

//insert at front of queue (for batch script to run first)
void enqueue_head(PCB *process){
    pthread_mutex_lock(&queue_mutex);
    if (process == NULL){
        pthread_mutex_unlock(&queue_mutex);
        return;
    }
    process->next = head; //new process points to old head
    head = process; //head becomes the new process
    if (tail == NULL){ //if queue was empty, tail also points to this process
        tail = process;
    }
    pthread_mutex_unlock(&queue_mutex);
}

//SJF enqueue: insert process sorted by code_len. Shortest job first
void enqueue_sjf(PCB *process) {
    pthread_mutex_lock(&queue_mutex);
    if(!process) {
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    //insert at head if the queue is empty or the job is the shortest
    if(!head || process->code_len < head->code_len) {
        process->next = head;
        head = process;
        if(!tail) tail = head;
        pthread_mutex_unlock(&queue_mutex);
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
    pthread_mutex_unlock(&queue_mutex);
}

//adding pcb to end of ready queue
// mode = 0 for initial enqueue (ties go to end)
// mode = 1 for re-enqueue after running (ties go to front)
// readyqueue.c
void enqueue_aging(PCB *process, int mode) {
    pthread_mutex_lock(&queue_mutex);

    if (!process) {
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    // empty queue
    if (!head || process->job_score < head->job_score || 
        (mode == 1 && process->job_score == head->job_score)) {
        process->next = head;
        head = process;
        if (!tail) tail = head;
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    PCB *cur = head;

    // traverse queue for insertion
    while (cur->next) {
        if (process->job_score < cur->next->job_score) break;
        if (process->job_score == cur->next->job_score && mode == 0) break; // normal insert at end of tie
        cur = cur->next;
    }

    process->next = cur->next;
    cur->next = process;

    if (!process->next) tail = process;

    pthread_mutex_unlock(&queue_mutex);
}

//decrease score of all jobs in queue except the running one
void age_ready_queue() {
    pthread_mutex_lock(&queue_mutex);
    PCB *cur = head;
    while(cur) {
        if(cur->job_score > 0) cur->job_score--;
        cur = cur->next;
    }
    pthread_mutex_unlock(&queue_mutex);
}

PCB* dequeue(){
    pthread_mutex_lock(&queue_mutex);
    if (head == NULL){ //if queue empty
        pthread_mutex_unlock(&queue_mutex);
        return NULL; 
    }
    PCB *process = head; //getting head
    head = head->next; //update head to second process
    if (head == NULL){ //case where dequeueing make queue empty
        tail = NULL;
    }
    process->next = NULL; //remove process from queue
    pthread_mutex_unlock(&queue_mutex);
    return process; 
}

//returns 1 if empty, 0 if not
int is_empty(){
    pthread_mutex_lock(&queue_mutex);
    int empty = (head == NULL);
    pthread_mutex_unlock(&queue_mutex);
    return empty;
}


