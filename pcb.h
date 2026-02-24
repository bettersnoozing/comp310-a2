//process control block
//defining pcb structure and func prototypes 

typedef struct pcb{
    int pid;
    int start_index; //index where script will start
    int code_len; //to have full location
    int pc;
    struct pcb *next; //for ready queue
} PCB;

PCB* make_pcb(int start, int len);
int get_pid();