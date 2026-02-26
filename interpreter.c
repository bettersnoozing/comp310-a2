//#define DEBUG 1

#ifdef DEBUG
#   define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#   define debug(...)
// NDEBUG disables asserts
#   define NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>              // tolower, isdigit
#include <dirent.h>             // scandir
#include <unistd.h>             // chdir
#include <sys/stat.h>           // mkdir
// for run:
#include <sys/types.h>          // pid_t
#include <sys/wait.h>           // waitpid

#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "readyqueue.h"
#include "scheduler.h"
#include "interpreter.h"

//arguments for this are n number of files, and final arg is the scheduling type
int exec_command(char *args[], int arg_count);

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int badcommandMkdir() {
    printf("Bad command: my_mkdir\n");
    return 4;
}

int badcommandCd() {
    printf("Bad command: my_cd\n");
    return 5;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int echo(char *tok);
int ls();
int my_mkdir(char *name);
int touch(char *path);
int cd(char *path);
int source(char *script);
int run(char *args[], int args_size);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    // these bits of debug output were very helpful for debugging
    // the changes we made to the parser!
    debug("#args: %d\n", args_size);
#ifdef DEBUG
    for (size_t i = 0; i < args_size; ++i) {
        debug("  %ld: %s\n", i, command_args[i]);
    }
#endif

    if (args_size < 1) {
        // This shouldn't be possible but we are defensive programmers.
        fprintf(stderr, "interpreter called with no words?\n");
        exit(1);
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand();
        return ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2)
            return badcommand();
        return touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2)
            return badcommand();
        return cd(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "run") == 0) { //run = what assignment 1 accomplished
        if (args_size < 2)
            return badcommand();
        return run(&command_args[1], args_size - 1);
    }else if (strcmp(command_args[0], "exec") == 0) { //exec = run multiple scripts concurrently as processes
        if (args_size < 3) //at minimum must have "exec" + script + scheduling type 
            return badcommand();
        return exec_command(&command_args[1], args_size - 1); 
        //NOTE TO AAHAAN: AS U KEEP WRITING MORE PARTS, U NEED TO UPDATE THE INTERPRETER SO IT CAN TAKE UR SCHEDULING TYPE
    }else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    mem_set_value(var, value);
    return 0;
}

int print(char *var) {
    char *value = mem_get_value(var);
    if (value) {
        printf("%s\n", value);
        free(value);
    } else {
        printf("Variable does not exist\n");
    }
    return 0;
}

int echo(char *tok) {
    int must_free = 0;
    // is it a var?
    if (tok[0] == '$') {
        tok++;                  // advance pointer, so that tok is now the stuff after '$'
        tok = mem_get_value(tok);
        if (tok == NULL) {
            tok = "";           // must use empty string, can't pass NULL to printf
        } else {
            must_free = 1;
        }
    }

    printf("%s\n", tok);

    // memory management technically optional for this assignment
    if (must_free) free(tok);

    return 0;
}

// We can hide dotfiles in ls using either the filter operand to scandir,
// or by checking the first character ourselves when we go to print
// the names. That would work, and is less code, but this is more robust.
// And this is also better since it won't allocate extra dirents.
int ls_filter(const struct dirent *d) {
    if (d->d_name[0] == '.') return 0;
    return 1;
}

int ls_compare_char(char a, char b) {
    // assumption: a,b are both either digits or letters.
    // If this is not true, the characters will be effectively compared
    // as ASCII when we do the lower_a - lower_b fallback.

    // if both are digits, compare them
    if (isdigit(a) && isdigit(b)) {
        return a - b;
    }
    // if only a is a digit, then b isn't, so a wins.
    if (isdigit(a)) {
        return -1;
    }

    // lowercase both letters so we can compare their alphabetic position.
    char lower_a = tolower(a), lower_b = tolower(b);
    if (lower_a == lower_b) {
        // a and b are the same letter, possibly in different cases.
        // If they are really the same letter, this returns 0.
        // Otherwise, it's negative if A was capital,
        // and positive if B is capital.
        return a - b;
    }

    // Otherwise, compare their alphabetic position by comparing
    // them at a known case.
    return lower_a - lower_b;
}

int ls_compare_str(const char *a, const char *b) {
    // a simple strcmp implementation that uses ls_compare_char.
    // We only check if *a is zero, since if *b is zero earlier,
    // it would've been unequal to *a at that time and we would return.
    // If *b is zero at the same point or later than *a, we'll exit the
    // loop and return the correct value with the last comparison.

    while (*a != '\0') {
        int d = ls_compare_char(*a, *b);
        if (d != 0) return d;
        a++, b++;
    }
    return ls_compare_char(*a, *b);
}

int ls_compare(const struct dirent **a, const struct dirent **b) {
    return ls_compare_str((*a)->d_name, (*b)->d_name);
}

int ls() {
    // straight out of the man page examples for scandir
    // alphasort uses strcoll instead of strcmp,
    // so we have to implement our own comparator to match the ls spec.
    // Note that the test cases weren't very picky about the specified order,
    // so if you just used alphasort with scandir, you should have passed.
    // This was intentional on our part.
    struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, NULL, ls_compare);
    if (n == -1) {
        // something is catastrophically wrong, just give up.
        perror("my_ls couldn't scan the directory");
        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        printf("%s\n", namelist[i]->d_name);
        free(namelist[i]);
    }
    free(namelist);

    return 0;
}

int str_isalphanum(char *name) {
    for (char c = *name; c != '\0'; c = *++name) {
        if (!(isdigit(c) || isalpha(c))) return 0;
    }
    return 1;
}

int my_mkdir(char *name) {
    int must_free = 0;

    debug("my_mkdir: ->%s<-\n", name);

    if (name[0] == '$') {
        ++name;
        // lookup name
        name = mem_get_value(name);
        debug("  lookup: %s\n", name ? name : "(NULL)");
        if (name) {
            // name exists, should free whatever we got
            must_free = 1;
        }
    }
    if (!name || !str_isalphanum(name)) {
        // either name doesn't exist, or isn't valid, error.
        if (must_free) free(name);
        return badcommandMkdir();
    }
    // at this point name is definitely OK

    // 0777 means "777 in octal," aka 511. This value means
    // "give the new folder all permissions that we can."
    int result = mkdir(name, 0777);

    if (result) {
        // description doesn't specify what to do in this case,
        // (including if the directory already exists)
        // so we just give an error message on stderr and ignore it.
        perror("Something went wrong in my_mkdir");
    }

    if (must_free) free(name);
    return 0;
}

int touch(char *path) {
    // we're told we can assume this.
    assert(str_isalphanum(path));
    // if things go wrong, just ignore it.
    FILE *f = fopen(path, "a");
    fclose(f);
    return 0;
}

int cd(char *path) {
    // we're told we can assume this.
    assert(str_isalphanum(path));

    int result = chdir(path);
    if (result) {
        // chdir can fail for several reasons, but the only one we need
        // to handle here for the spec is the ENOENT reason,
        // aka Error NO ENTry -- the directory doesn't exist.
        // Since that's the only one we have to handle, we'll just assume
        // that that's what happened.
        // Alternatively, you can check if the directory exists
        // explicitly first using `stat`. However it is often better to
        // simply try to use a filesystem resource and then recover when
        // you can't, rather than trying to validate first. If you validate
        // first while two users are on the system, there's a race condition!
        return badcommandCd();
    }
    return 0;
}

//source: run a script as a process using the scheduler
int source(char *script) {
    int start, len;

    //load entire script onto code memory
    if (store_script(script, &start, &len) != 0) {
        return badcommandFileDoesNotExist(); 
    }

    PCB *process = make_pcb(start, len);
    Policy policy = FCFS_POLICY;
    enqueue(process);
    scheduler(policy);    
    return 0;
}

//global flag for multi-thread scheduling
int mt_enabled = 0; //the defaul is single-threaded

// exec command- run multiple scripts concurrently 
int exec_command(char *args[], int arg_count) {
    //args are list of programs, then the scheduling type at the end (need to parse)
    int num_progs = arg_count - 1;
    char *policy_str = args[arg_count - 1];

    // checking scheduling type. for now, it's only FCFS (first come first serve)
    //NOTE TO AAHAAN: AS U KEEP WRITING MORE PARTS, U NEED TO UPDATE THIS PART SO IT CAN TAKE UR SCHEDULING TYPE

//determine the policy type
    Policy policy;

    if (strcmp(policy_str, "FCFS") == 0) {
        policy = FCFS_POLICY;
    } else if (strcmp(policy_str, "SJF") == 0) {
        policy = SJF_POLICY;
    } else if (strcmp(policy_str, "RR") == 0) {
        policy = RR_POLICY;
    } else if (strcmp(policy_str, "RR30") == 0) {
	policy = RR30_POLICY;
    } else if (strcmp(policy_str, "AGING") == 0) {
        policy = AGING_POLICY;
    } else {
	printf("Unknown policy\n");
	return 1;
    }

   //check for background flag "#" in arguments
   int background = 0; //default so run normally
   int last_prog_index = num_progs - 1;
   if(strcmp(args[last_prog_index], "#") == 0) {
	background = 1; //found the background flag
	num_progs--; //adjust the number of programs to ignore #
   }

    //Check for multi-threaded option
    if(num_progs >= 1 && strcmp(args[num_progs], "MT") == 0) {
	mt_enabled = 1;
	num_progs--; //exclude MT from program count
    }

    //can only be at more 3 programs
    if (num_progs < 1 || num_progs > 3) {
        printf("Error: exec takes 1 to 3 programs\n");
        return 1;
    }

    //checking for duplicate program names to avoid errors
    for (int i = 0; i < num_progs; i++) {
        for (int j = i + 1; j < num_progs; j++) {
            if (strcmp(args[i], args[j]) == 0) {
                printf("Error: duplicate program name\n");
                return 1;
            }
        }
    }

    //array for loading results
    int starts[3];
    int lens[3];
    int loaded = 0;   //#s successfully loaded so far

    //load each script onto code memory (at fail print error and free)
    for (int i = 0; i < num_progs; i++) {
        if (store_script(args[i], &starts[i], &lens[i]) != 0) {
            //loading fail
            printf("Error: cannot load %s\n", args[i]);
            for (int j = 0; j < loaded; j++) {
                free_lines(starts[j], lens[j]);
            }
            return 1;
        }
        loaded++;
    }

   //if background mode is requested, convert the remaining batch script into a PCB
   PCB *batch_pcb = NULL;
   if(background) {
	//the batch script starts at the line after this exec command
	int batch_start, batch_len;
	if(store_remaining_script(&batch_start, &batch_len) == 0) {
		batch_pcb = make_pcb(batch_start, batch_len);
		enqueue(batch_pcb); //batch script runs first
	} else {
		printf("Error: could not load batch script\n");
	}
   }

    //creating pcb's and enqueueing them (fcfs order = order of args)
    for (int i = 0; i < num_progs; i++) {
        PCB *proc = make_pcb(starts[i], lens[i]);
        if(policy == AGING_POLICY) {
		enqueue_aging(proc);
	} else if(policy == SJF_POLICY) {
		enqueue_sjf(proc); //insert sorted by job length
	} else {
		enqueue(proc); //FCFS or RR
	} 
    }

    //execute all processes using scheduler until wait queue is empty
    scheduler(policy);

    //if scheduler returns, all processes finished = success!!! 
    return 0;
}

int run(char *args[], int arg_size) {
    // copy the args into a new NULL-terminated array.
    char **adj_args = calloc(arg_size + 1, sizeof(char *));
    for (int i = 0; i < arg_size; ++i) {
        adj_args[i] = args[i];
    }

    // always flush output streams before forking.
    fflush(stdout);
    // attempt to fork the shell
    pid_t pid = fork();
    if (pid < 0) {
        // fork failed. Report the error and move on.
        perror("fork() failed");
        return 1;
    } else if (pid == 0) {
        // we are the new child process.
        execvp(adj_args[0], adj_args);
        perror("exec failed");
        // The parent and child are sharing stdin, and according to
        // a part of the glibc documentation that you are **not**
        // expected to know for this course, a shared input handle
        // should be fflushed (if it is needed) or closed
        // (if it is not). Handling this exec error case is not even
        // necessary, but let's do it right.
        // (Failure to do this can result in the parent process
        // reading the remaining input twice in batch mode.)
        fclose(stdin);
        exit(1);
    } else {
        // we are the parent process.
        waitpid(pid, NULL, 0);
    }

    return 0;
}
