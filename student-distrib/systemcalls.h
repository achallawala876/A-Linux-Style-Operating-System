#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

// #include "idt_linkage.h"

extern int terminal_flag[3];
extern int current_terminal;
extern int32_t parent_cur_running;
extern int32_t parent_last_scheduled;

extern int32_t pid;
extern int32_t last_scheduled;

int32_t halt_handler(void* status, void*, void*);
int32_t execute_handler(int8_t* command, void*, void*);
int32_t read_handler(void* fd_in, void* buf_in, void* nbytes_in);
int32_t write_handler(void* fd_in, void* buf_in, void* nbytes_in);
int32_t open_handler(void* filename_in, void* arg2, void* arg3);
int32_t close_handler(void* fd_in, void* arg2, void* arg3);
int32_t getargs(void* buf, void* nbytes, void* arg3);
int32_t vidmap (void* screen_start, void* arg2, void* arg3);
int32_t set_handler (void *signum, void* handler_address, void* arg3);
int32_t sigreturn (void* arg1, void* arg2, void* arg3);

int32_t get_program_state(int32_t num);

void scheduler();

int32_t get_offset(void);

void init_pcb();
void select_terminal();

void flush_tlb();

typedef int32_t (*syscall_func_t)(int32_t, void*, int32_t);

/* PCB and stuff related to the active process */
typedef struct file_descriptor {
    // each is 4 bytes
    syscall_func_t *file_operations_jump_table;
    uint32_t file_position;
    uint32_t inode;
    uint32_t flags;
} file_descriptor_t;

typedef struct process_control_block {
    int32_t EBP_val, ESP_val;
    uint32_t process_number;
    uint32_t parent_program; // think of data structure; if this doies not work just store address as an integer
    uint32_t program_state;
    uint32_t kernel_stack_addr;
    uint32_t running_prg_pid;
    uint8_t get_argument[33];
    file_descriptor_t file_descriptor_table [8]; // 8 entries in FDT
} pcb_t;



 // max 8 programs?




#endif /* _SYSTEMCALLS_H */
