#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

#define CODE_SIZE 1000
static char *code_memory[CODE_SIZE] = {NULL}; 
static int code_used = 0; //tracking how many lines stored so far

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}

//loading script file into code memory
//start_index and nb_lines are filled (or -1 on fail)
int store_script(const char *filename, int *start_index, int *nb_lines){
    FILE *file = fopen(filename, "r");
    if (!file){
        return -1; 
    }

    int start = code_used; //index of first line of script
    int lines = 0;
    char line[100];

    while (fgets(line, sizeof(line), file)) {
        if (code_used >= CODE_SIZE) {     // out of memory
            fclose(file);
            // free any lines we already stored
            for (int i = start; i < code_used; i++) {
                free(code_memory[i]);
                code_memory[i] = NULL;
            }
            code_used = start;
            return -1; // out of memory
        }
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n'){
            line[len-1] = '\0'; //remove newline
        }
        code_memory[code_used] = strdup(line); //store line in code memory
        code_used++;
        lines++;
}

    fclose(file);
    *start_index = start;
    *nb_lines = lines;
    return 0; 
}

//store remaining lines from stdin as a script
//0 on success and -1 on failure (out of memory)
int store_remaining_script(int *start_index, int *nb_lines) {
    int start = code_used; //remember where we start
    int lines = 0;
    char line[100];

    //reading lines from stdin until EOF
    while (fgets(line, sizeof(line), stdin)) {
        if (code_used >= CODE_SIZE) {
            // out of memory – free already stored lines
            for (int i = start; i < code_used; i++) {
                free(code_memory[i]);
                code_memory[i] = NULL;
            }
            code_used = start;
            return -1;
        }
        //remove trailing \n
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n')
            line[len-1] = '\0';
        code_memory[code_used] = strdup(line);
        code_used++;
        lines++;
    }
    *start_index = start;
    *nb_lines = lines;
    return 0;
}

//returning ptr to line stored at given index
char* get_line(int index){
    if (index < 0 || index >= code_used){
        return NULL;
    }
    return code_memory[index];
}

//freeing lines from start to start+len-1
void free_lines(int start, int len){
    for (int i=start; i<start + len; i++){
        if (code_memory[i]){
            free(code_memory[i]);
            code_memory[i] = NULL;
        }
    }
}

