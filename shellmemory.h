#define MEM_SIZE 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

//declarations for functions for code memory management and retrieval
int store_script(const char *file, int *start_index, int *nb_lines);
char* get_line(int index);
void free_lines(int start_index, int len);