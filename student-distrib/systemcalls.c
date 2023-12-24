#include "x86_desc.h"
#include "paging.h"
#include "systemcalls.h"
#include "file_system.h"
#include "lib.h"
#include "keyboard.h"
#include "RTC.h"
#include "pit.h"

#define TASK_RUNNING 1
#define TASK_PAUSED 2
#define MAX_FILES 8
#define EXCEPTION_CODE 256

extern syscall_func_t terminal_table[4];
extern syscall_func_t rtc_table[4];
extern syscall_func_t file_table[4];
extern syscall_func_t dir_table[4];

// only terminal 1 shells runs upon boot
int terminal_flag[3];
int current_terminal = 0; // booting into terminal 1


int32_t parent_cur_running;
int32_t parent_last_scheduled;
int32_t last_scheduled = -1;

// extern page_directory_entry page_directory[1024];

// static int32_t last_scheduled = -1;


static pcb_t* pcb_ptr_array[6] = {
    (pcb_t*)(0x800000 - 0x2000),
    (pcb_t*)(0x800000 - 0x4000),
    (pcb_t*)(0x800000 - 0x6000),
    (pcb_t*)(0x800000 - 0x8000),
    (pcb_t*)(0x800000 - 0xA000),
    (pcb_t*)(0x800000 - 0xC000)
};

int32_t pid = -1;
static int32_t cur_file = -1;


/*
 * flush_tlb
 *   DESCRIPTION: Wrapper function for flushing the Translation Lookaside Buffer (TLB).
 *   INPUTS: None.
 *   OUTPUTS: Flushes the TLB.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies the TLB and system state.
 */

void flush_tlb() {
    asm volatile(
            "mov %0, %%cr3;"
            :
            :"r"(page_directory)
    );
}



/*
 * select_terminal_handler
 *   DESCRIPTION: Wrapper function for selecting a terminal and performing related operations.
 *   INPUTS:
 *     - selected_terminal: The terminal to be selected (values 0, 1, or 2).
 *   OUTPUTS: Selects the specified terminal and performs related operations.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies process control blocks, memory, and system state.
 */

void select_terminal(int selected_terminal){

    // cli(); // NOT NEEDED, KEYBOARD INTERRUPT CLEARS ALL INTERRUPTS ANYWAYS 

    if (selected_terminal <= 2 && selected_terminal >= 0 && selected_terminal != current_terminal) { // only terminal 1, 2 or 3 allowed


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        store_cur_buffer(current_terminal);
        // store_cur_rtc(current_terminal);

        store_cur_cursor(parent_last_scheduled);
        if (current_terminal != parent_last_scheduled) {
            // store_cur_cursor(last_scheduled);

            remap_videoMap(-1);
        }


        int32_t * curr_term_allocated_memspace = (int32_t*)((185 + current_terminal)*4096);
        int32_t * selected_term_allocated_memspace = (int32_t*)((185 + selected_terminal)*4096);
        int32_t * curr_term_vidpage = (int32_t*)(184*4096);

        /* Store current cursor */
        // store_cur_cursor(current_terminal);

        /* Store current buffer */
        // store_cur_buffer(current_terminal);

        // remap_videoMap(-1); // to remap virtual vidmem to main physical vidmem
        // flush_tlb();

        memcpy(curr_term_allocated_memspace, curr_term_vidpage, 4096); // move current terminal video mem to specified video mem location
        clear_screen();
        memcpy(curr_term_vidpage, selected_term_allocated_memspace, 4096); // move new terminal video mem to current terminal video mem

        // remap_videoMap(pcb_ptr_array[pid] -> parent_program); // root temrinal  of the pid

        if (current_terminal != last_scheduled) {
            load_new_cursor(parent_last_scheduled);
            remap_videoMap(parent_last_scheduled);
        }
        else {
            // load_new_cursor(select_terminal);
            load_new_cursor(current_terminal);
        }
        // flush_tlb();
        

        current_terminal = selected_terminal; // update current terminal to argument -> info useful for switching buffer and tracking

        if (selected_terminal == last_scheduled) {
            remap_videoMap(-1);
        }
        else {
            remap_videoMap(parent_last_scheduled);
        }

        select_terminal_buffer(selected_terminal);
    }
}

/*
 * init_pcb
 *   DESCRIPTION: Initializes process control blocks (PCBs) and related flags.
 *   INPUTS: None.
 *   OUTPUTS: Initializes PCBs and flags.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies PCBs and flags.
 */

void init_pcb (){
    pcb_ptr_array[0] -> process_number = -1;
    pcb_ptr_array[1] -> process_number = -1;
    pcb_ptr_array[2] -> process_number = -1;
    pcb_ptr_array[3] -> process_number = -1;
    pcb_ptr_array[4] -> process_number = -1;
    pcb_ptr_array[5] -> process_number = -1;

    pcb_ptr_array[0] -> program_state = TASK_PAUSED;
    pcb_ptr_array[1] -> program_state = TASK_PAUSED;
    pcb_ptr_array[2] -> program_state = TASK_PAUSED;


    terminal_flag[0] = 0; // only have the 1st terminal running at boot time
    terminal_flag[1] = 0;
    terminal_flag[2] = 0;
}


/*
 * halt_handler
 *   DESCRIPTION: Halts the currently executing process and returns to the parent process.
 *   INPUTS:
 *     - status_in: Pointer to the status information of the process.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: Halts the current process and returns to the parent process.
 *   RETURN VALUE:
 *     - Should not return.
 *   SIDE EFFECTS: Modifies process control blocks and system state.
 */

int32_t halt_handler(void* status_in, void* arg2, void* arg3){ // arguments mean nothing
    cli();
    // printf("In halt handler");

    // status comes from user program 
    int i;
    uint32_t status = (uint32_t)status_in;
    uint32_t eax = (uint32_t)(status & 0xFF); // get status in lowest byte of eax

    // any healthily closed function will update this eax with a 0 and the next if statement won't touch it, so your execute handler
    // will give the user shell program a 0

    // so somehow, in our case... we are getting a weird value of eax somewhere... causing pingpong to fail when we switch terminals back


    // was exception generated in that user program?
    if ((uint32_t)status_in==EXCEPTION_CODE){
        eax=EXCEPTION_CODE; // for value 256 in shell.c handling "program terminated by exception"
    }

    // if nothing is to be halted? we should never end up here tbh 
    if(pcb_ptr_array == NULL)
        return -1;

    // terminate this program by taking away everything it loves
    for(i = 0; i < MAX_FILES; i++) {
        pcb_ptr_array[pid]->file_descriptor_table[i].file_operations_jump_table = NULL;
        pcb_ptr_array[pid]->file_descriptor_table[i].file_position = 0;
        pcb_ptr_array[pid]->file_descriptor_table[i].inode = -1;
        pcb_ptr_array[pid]->file_descriptor_table[i].flags = 0;
    }

    pcb_ptr_array[pid]->process_number = -1; // clearing this program in the pcb array... no use of this bih

    // if trying to exit shell, just execute that mf and clear screen for illusion of restart
    if (pid == 0 || pid == 1 || pid ==  2) {
        clear_screen();
        execute_handler("shell", NULL, NULL); // start new shell process
    }

    int32_t esp, ebp;
    // if(!(pid == 0 || pid == 1 || pid == 2)) {

    if(pid != 0) {
        esp = pcb_ptr_array[pcb_ptr_array[pid]->parent_program]->ESP_val;
        ebp = pcb_ptr_array[pcb_ptr_array[pid]->parent_program]->EBP_val;

        // if you are in base shell, you have no parents and hence must not have an older esp and ebp to go to... SO YOU WONT COME HERE

        // Restore Parent paging
        add_page_entry(pcb_ptr_array[pid]->parent_program);
        flush_tlb(); // should be here instead
    }

    pcb_ptr_array[pid]->program_state = TASK_PAUSED;

    // Flush TLB
    // asm volatile(
    //     "mov %0, %%cr3;"
    //     :
    //     :"r"(page_directory)
    // );

    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - (0x2000*(pcb_ptr_array[pid]->parent_program)); // setting kernel stack of next program after halting (naturally a parent)

    // pid--;
    
    pid = pcb_ptr_array[pid]->parent_program; // pid will now be of the parent program

    last_scheduled = pid;

    pcb_ptr_array[pid] -> program_state = TASK_RUNNING;
    sti();

    // updating esp and ebp so you go back to parent program, and then do jump rety back to execute handler so you can return from it
    asm volatile("movl %0, %%esp;"
                "movl %1, %%ebp;"
                "movl %2, %%eax;"
                "jmp rety"
                :
                : "r"(esp), "r" (ebp), "r"(eax)
                : "esp", "ebp"
                );

    return status; // do we ever reach here? NOPE

}


/*
 * execute_handler
 *   DESCRIPTION: Executes the program given by the command and switches to user mode.
 *   INPUTS:
 *     - command_in: Pointer to a null-terminated string representing the command to be executed.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: Executes the program and switches to user mode.
 *   RETURN VALUE:
 *     - Should not return on successful execution.
 *     - -1 on failure.
 *   SIDE EFFECTS: Modifies process control blocks, memory, and system state.
 */

int32_t execute_handler(int8_t* command, void* arg2, void* arg3) {
    int j;
    if(!command) {
        /* Invalid */
        return -1; // bad value so directly send -1 to shell main loop for a "no such command"
    }

    // printf("%d", pid);


    // find next available pid to assign
    // store the parent pid
    // set global pid = new pid

    cli();
    for (j = 0; j < 6; j++){
        if (pcb_ptr_array[j]->process_number == -1){
            pcb_ptr_array[j]->parent_program = pid; // parent for first shell is -1 // store the parent pid
            pid = j; // set global pid = new pid
            pcb_ptr_array[j]->process_number = pid; // store this assigned pid
            break;
        }
    }

    /* If 1st shell, lock pid 1 & 2 so any prgs from 1st shell fill pid 3 and onwards */
    if (pid == 0) {
        pcb_ptr_array[1] -> process_number = 1;
        pcb_ptr_array[2] -> process_number = 2;
    }

    if (pid == 0 || pid == 1 || pid == 2) {
        pcb_ptr_array[pid] -> parent_program = pid;
    }

    // if no pid available
    if(j == 6) {
        /* Max programs reached */
        sti();
        return 7; // not found, 7th program
    }

    uint8_t file_name[32];
    uint8_t buf[4]; // to check first 4 bytes of file for magic words
    uint32_t i, cnt;
    uint8_t* virtual_ptr =  (uint8_t*)(0x08048000); // starting of 128 MB plus offset (entire executable's starting point)

    /* Initializing get_argument */
    for (i = 0; i <= 32; i++){
        pcb_ptr_array[pid]->get_argument[i] = NULL;
    }

    /* Parse command for file to run */
    for (i = 0; i < strlen((int8_t*)command); i++) {
        if (command[i] == ' ')
            break;
        else if (command[i] == '\n')
            break;
        else if (command[i] == '\0')
            break;
        else
            file_name[i] = command[i];
    }

    file_name[i] = '\0';
    
    /* Parse command for arguments */
    for(j = i + 1; j < strlen((int8_t*)command); j++){
        if (j - i - 1 >= 32)
        {
            pcb_ptr_array[pid]->process_number = -1;
            pid = pcb_ptr_array[pid]->parent_program;
            sti();
            return 1;
            // pcb_ptr_array[pid + 1] -> get_argument[j - i - 1] = '\0';
            // break;
        }
        pcb_ptr_array[pid] -> get_argument[j - i - 1] = command[j];
    }
    
    // add paging entry for new program to run
    if (add_page_entry(pid) == -1) {
        /* Invalid */
        pcb_ptr_array[pid]->process_number = -1;
        pid = pcb_ptr_array[pid]->parent_program;
        sti();
        return -1;
    }

    // /* Parse command for arguments */
    // for(j = i + 1; j < strlen((int8_t*)command); j++){
    //     if (j - i - 1 >= 32)
    //     {
    //         add_page_entry(pcb_ptr_array[pid]->parent_program);
    //         flush_tlb();             
    //         pcb_ptr_array[pid]->process_number = -1;
    //         pid = pcb_ptr_array[pid]->parent_program;
    //         sti();
    //         return 1;
    //         // pcb_ptr_array[pid + 1] -> get_argument[j - i - 1] = '\0';
    //         // break;
    //     }
    //     pcb_ptr_array[pid] -> get_argument[j - i - 1] = command[j];
    // }

    // flushing the tlb after adding a new program
    // asm volatile(
    //     "mov %0, %%cr3;"
    //     :
    //     :"r"(page_directory)
    // );
    flush_tlb();

    // printf("Flush TLB passed\n");

    /* Read first 4 bytes of file for magic code */
    if (-1 == (cnt = file_read(file_name, 4, buf) ) ) {
        /* invalid */
        add_page_entry(pcb_ptr_array[pid]->parent_program); // restore parent program
        flush_tlb();
        pcb_ptr_array[pid]->process_number = -1;
        pid = pcb_ptr_array[pid]->parent_program;
        sti();
        return -1;
    }

    // printf("File read passed with bytes read: %d\n", cnt);

    /* Check for magic code */
    if (!(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4C && buf[3] == 0x46)) {
        /* Invalid */
        add_page_entry(pcb_ptr_array[pid]->parent_program);
        flush_tlb(); 
        pcb_ptr_array[pid]->process_number = -1;
        pid = pcb_ptr_array[pid]->parent_program;
        sti();
        return -1;
    }

    // /* Parse command for arguments */
    // for(j = i + 1; j < strlen((int8_t*)command); j++){
    //     if (j - i - 1 >= 32)
    //     {
    //         add_page_entry(pcb_ptr_array[pid]->parent_program);
    //         flush_tlb();             
    //         pcb_ptr_array[pid]->process_number = -1;
    //         pid = pcb_ptr_array[pid]->parent_program;
    //         sti();
    //         return 1;
    //         // pcb_ptr_array[pid + 1] -> get_argument[j - i - 1] = '\0';
    //         // break;
    //     }
    //     pcb_ptr_array[pid] -> get_argument[j - i - 1] = command[j];
    // }

    /* Copy file contents into physical mem*/
    if (-1 == (cnt = file_read(file_name, 10000, virtual_ptr) ) ) {
        /* invalid */
        add_page_entry(pcb_ptr_array[pid]->parent_program);
        flush_tlb();
        pcb_ptr_array[pid]->process_number = -1;
        pid = pcb_ptr_array[pid]->parent_program;
        sti();
        return -1;
    }

    uint32_t entry_point_addr = *((uint32_t*)(virtual_ptr + 24)); // user program entry point - where user program starts running (bytes 24-27 contain 32
    // bit integer which is the entry point of the first instruction to be run) -> dereference from mem location containing bytes 24 to 27 to get entry point address

    uint32_t cur_ks_addr; // unncessary, we already have the pcb storing the kernel address as well



    /* Initialize pcb_ptr_array for this pid */
    pcb_ptr_array[pid] -> program_state = TASK_RUNNING;
    pcb_ptr_array[pid] -> kernel_stack_addr = 0x800000 - (0x2000*(pid)); // kernel stack addr = 8MB - (pid+1)*2KB
    pcb_ptr_array[pid] -> file_descriptor_table[0].file_operations_jump_table = terminal_table;
    pcb_ptr_array[pid] -> file_descriptor_table[0].flags = 1;
    pcb_ptr_array[pid] -> file_descriptor_table[1].file_operations_jump_table = terminal_table; 
    pcb_ptr_array[pid] -> file_descriptor_table[1].flags = 1;

    //  setting stdin and stdout

    /* Set flags for all fdt entries to 0 */
    for (i = 2; i <=7; i++) {
        pcb_ptr_array[pid] -> file_descriptor_table[i].flags = 0;
    }

    // if current process not shell, mark parent as paused
    if (!(pid == 0 || pid == 1 || pid == 2)) {
        pcb_ptr_array[pcb_ptr_array[pid] -> parent_program] -> program_state = TASK_PAUSED;
    }

    // setting flags for all other file descriptor fields

    cur_ks_addr = 0x800000 - (0x2000*(pid)); // kernel stack addr = 8MB - (pid+1)*2KB (readded to a new variable, redundant)

    if (pid > 2) {
        last_scheduled = pid;
    }
    

    // printf("PCB creation passed\n");

    // set tss as 0 is privilege level of kernel
    tss.ss0 = KERNEL_DS; // kernel data segment loaded
    tss.esp0 = cur_ks_addr; // current kernel stack address loaded

    // printf("TSS passed \n");

    int32_t temp_esp;
    int32_t temp_ebp;

    /* Storing the esp and ebp of the parent program */

    if (pid != 0) { // shell is starting (pid > -1)
        asm volatile(
            "mov %%esp, %0\n" // newline character fixes issue of eax temp register in asm dump not working correctlys
            "mov %%ebp, %1\n"
            :"=r"(temp_esp), "=r"(temp_ebp)
        );

        pcb_ptr_array[pcb_ptr_array[pid]->parent_program]->ESP_val = temp_esp;
        pcb_ptr_array[pcb_ptr_array[pid]->parent_program]->EBP_val = temp_ebp;
    }


    uint32_t new_esp = 0x8400000 - sizeof(int32_t); // user program stack ptr -> bottom of 128 MB page // VADDR
    // why size of int 32?
    uint32_t eax_val;
    
    // printf("%d\n", pid);
    uint32_t modified_flags;

    asm volatile (
        "pushfl;"
        "popl %0;"
        : "=r" (modified_flags)
        :
        : "memory"
    );

    modified_flags = modified_flags | 0x200; // THIS IS REQUIRED TO RE-ENABLE INTERRUPTS WITHOUT STI CALL BEFORE CONTEXT PUSH... CHECK WITH ADNAN'S NOTES TO UNDERSTAND
    
    // context switching
    asm volatile (
        "pushl %0;" // user_ds
        "pushl %1;" // esp
        "pushl %2;" // eflags
        "pushl %3;" // user_cs
        "pushl %4;" // eip
        "iret;" // need to add sti??
        "rety:" // label we return to after IRET takes us to userspace
        :
        : "i"(USER_DS), "r"(new_esp), "r"(modified_flags) , "i"(USER_CS), "r"(entry_point_addr) // i: known at runtime, r known later
    );

    asm volatile(
        "movl %%eax, %0\n"
        :"=r"(eax_val)
    );

    // printf("EAX: %d\n", eax_val);
    return eax_val;
}


/*
 * read_handler
 *   DESCRIPTION: Reads data from the specified file descriptor.
 *   INPUTS:
 *     - fd_in: File descriptor index indicating the file to read from.
 *     - buf_in: Pointer to a buffer where the read data will be stored.
 *     - nbytes_in: Number of bytes to read.
 *   OUTPUTS: Reads data from the file descriptor and stores it in the buffer.
 *   RETURN VALUE:
 *     - Number of bytes read on success.
 *     - -1 on failure.
 *   SIDE EFFECTS: Modifies the buffer with the read data.
 */

int32_t read_handler(void* fd_in, void* buf_in, void* nbytes_in) {
    // printf("Comes in read handler\n");
    // return 0;
    int32_t fd = (int32_t)fd_in;
    uint8_t* buf = (uint8_t*)buf_in;
    int32_t nbytes = (int32_t)nbytes_in;
    int32_t bytes_written = 0;


    if (pcb_ptr_array[pid]->file_descriptor_table[fd].flags == 0) {
        /* File not open yet */
        return -1;
    }

    if (fd == 0) {
        /* stdin */
        return read_terminal((int32_t)buf, (void*)nbytes, NULL);
        // return -1;
    }

    else if (fd == 1) {
        /*stdout*/
        return -1;
    }

    else if (fd>=2 && fd <=7) {
        cur_file = fd;
        bytes_written = pcb_ptr_array[pid]->file_descriptor_table[fd].file_operations_jump_table[1](
            pcb_ptr_array[pid]->file_descriptor_table[fd].inode,
            (void*)buf,
            nbytes
        );
        pcb_ptr_array[pid]->file_descriptor_table[fd].file_position += bytes_written;
        // printf("Leaves read handler with bytres read: %d\n", bytes_written);
        return bytes_written;
    }

    else {
        // printf("Invalid fd\n");
        return -1;
    }
}


/*
 * write_handler
 *   DESCRIPTION: Writes data to the specified file descriptor.
 *   INPUTS:
 *     - fd_in: File descriptor index indicating the file to write to.
 *     - buf_in: Pointer to a buffer containing the data to be written.
 *     - nbytes_in: Number of bytes to write.
 *   OUTPUTS: Writes data to the file descriptor.
 *   RETURN VALUE:
 *     - Number of bytes written on success.
 *     - -1 on failure.
 *   SIDE EFFECTS: Modifies the file specified by the file descriptor.
 */

int32_t write_handler(void* fd_in, void* buf_in, void* nbytes_in) {
    // printf("Comes in write handler\n");
    int32_t fd = (int32_t)fd_in;
    // const void* buf = buf_in;
    int32_t nbytes = (int32_t)nbytes_in;

    if (fd <= 0 || fd >= 8)
        return -1;

    if (pcb_ptr_array[pid]->file_descriptor_table[fd].flags == 0) {
        /* File not open yet */
        return -1;
    }


    switch (fd) {
    case 1:
        /* terminal write*/
        // return write_terminal(fd, (void*)buf_in, nbytes);
        return pcb_ptr_array[pid]->file_descriptor_table[1].file_operations_jump_table[2](
            fd,
            (void*)buf_in,
            nbytes
        );
    
    default:
        // printf("Not yet handled\n");
        return pcb_ptr_array[pid]->file_descriptor_table[fd].file_operations_jump_table[2](
            fd,
            (void*)buf_in,
            nbytes
        );
    }

}

/*
 * open_handler
 *   DESCRIPTION: Opens the file specified by the filename and returns a file descriptor.
 *   INPUTS:
 *     - filename_in: Pointer to a null-terminated string representing the name of the file to be opened.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: Opens the file and returns a file descriptor.
 *   RETURN VALUE:
 *     - File descriptor index on success (>= 2).
 *     - -1 on failure.
 *   SIDE EFFECTS: Modifies process control blocks and file descriptor table.
 */

int32_t open_handler(void* filename_in, void* arg2, void* arg3) {
    // printf("Comes in open handler\n");
    uint8_t *filename = (uint8_t*)filename_in;
    dentry_t dentry_1;
    uint32_t i;

    if(file_open((int32_t)filename, (void*)&dentry_1, NULL) == 0) {
        /* file exists*/
        for (i = 2; i <= 7; i++) {
            if (pcb_ptr_array[pid]->file_descriptor_table[i].flags == 0) {
                /* found empty entry in fd table*/
                break;
            }
        }

        if (i > 7) {
            /* No empty entry avaialable*/
            return -1;
        }

        /* We have found an empty entry at index i*/

        if(dentry_1.filetype == 0){
            /* RTC */
            pcb_ptr_array[pid]->file_descriptor_table[i].file_operations_jump_table = rtc_table;
            pcb_ptr_array[pid]->file_descriptor_table[i].file_position = 0;
            pcb_ptr_array[pid]->file_descriptor_table[i].inode = dentry_1.inode_num;
            pcb_ptr_array[pid]->file_descriptor_table[i].flags = 1;
            return i;
        }


        else if(dentry_1.filetype == 1) {
            /* Type directory */
            set_dir_index();
            pcb_ptr_array[pid]->file_descriptor_table[i].file_operations_jump_table = dir_table;
            pcb_ptr_array[pid]->file_descriptor_table[i].file_position = 0;
            pcb_ptr_array[pid]->file_descriptor_table[i].inode = dentry_1.inode_num;
            pcb_ptr_array[pid]->file_descriptor_table[i].flags = 1;
            return i;
        }

        else {
            /* Normal file */
            pcb_ptr_array[pid]->file_descriptor_table[i].file_operations_jump_table = file_table;
            pcb_ptr_array[pid]->file_descriptor_table[i].file_position = 0;
            pcb_ptr_array[pid]->file_descriptor_table[i].inode = dentry_1.inode_num;
            pcb_ptr_array[pid]->file_descriptor_table[i].flags = 1;
            return i;
        }
    }
    else {
        /* file dne */
        return -1;
    }
}


/*
 * close_handler
 *   DESCRIPTION: Closes the file specified by the file descriptor.
 *   INPUTS:
 *     - fd_in: File descriptor index indicating the file to be closed.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: Closes the file associated with the file descriptor.
 *   RETURN VALUE:
 *     - 0 on success.
 *     - -1 on failure.
 *   SIDE EFFECTS: Modifies process control blocks and file descriptor table.
 */

int32_t close_handler(void* fd_in, void* arg2, void* arg3) {
    // printf("Comes in close handler\n");
    int32_t fd = (int32_t)fd_in;

    if (fd < 2) {
        /* Invalid */
        return -1;
    }

    if (fd > 8) {
        /* Invalid */
        return -1;
    }

    if (pcb_ptr_array[pid]->file_descriptor_table[fd].flags == 0) { // was an = so we were always setting this to 0, and hence we weren't getting -1!
        /* Already closed */
        return -1;
    }

    pcb_ptr_array[pid]->file_descriptor_table[fd].file_operations_jump_table = NULL;
    pcb_ptr_array[pid]->file_descriptor_table[fd].file_position = 0;
    pcb_ptr_array[pid]->file_descriptor_table[fd].inode = -1;
    pcb_ptr_array[pid]->file_descriptor_table[fd].flags = 0;

    return 0;
}

/*
 * getargs
 *   DESCRIPTION: reads the program’s command line arguments into a user-level buffer
 *   INPUTS:
 *     - buf_in: pointer to user-level buffer.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: stores the command line args into the user level bbuffer.
 *   RETURN VALUE:
 *     - 0 on success.
 *     - -1 on failure.
 *   SIDE EFFECTS: None.
 */

int32_t getargs(void* buf_in, void* nbytes_in, void* arg3){
    //load new pcb? HOW TO?

    uint8_t* buf = (uint8_t*)buf_in;
    int32_t nbytes = (int32_t)nbytes_in;

    // if (nbytes > 32)
    //     nbytes = 32;

    if((buf == NULL) || (strlen((int8_t*)pcb_ptr_array[pid]->get_argument) > nbytes) || (strlen((int8_t*)pcb_ptr_array[pid]->get_argument) == 0))
        return -1;

    strncpy((int8_t*)buf, (int8_t*)pcb_ptr_array[pid]->get_argument, nbytes); //file_name is command line argument as specified in execute_handler
        return 0;
}

/*
 * vidmap
 *   DESCRIPTION: Maps the text-mode video memory into user space at a pre-set virtual address.
 *   INPUTS:
 *     - screen_start_in: Pointer to address where returned address should be stored.
 *     - arg2: Pointer to the second argument (not used in this function).
 *     - arg3: Pointer to the third argument (not used in this function).
 *   OUTPUTS: Maps the text-mode video memory into user space at the pre-set virtual address.
 *   RETURN VALUE:
 *     - 0 on success.
 *     - -1 on failure.
 *   SIDE EFFECTS: None.
 */

int32_t vidmap (void* screen_start_in, void* arg2, void* arg3) {
    uint8_t** screen_start = (uint8_t**)screen_start_in;
    
    // sending in variable from userspace... you send ptr to value
    
    if (screen_start == NULL)
        return -1;
    if ((uint32_t)screen_start < 0x8000000 || (uint32_t)screen_start > 0x8400000) // Beyond bounds of virtual mem addr for user program
    // 8400000 is the NEXT PAGE... WANNA ACCESS 4B BEFORE THIS AS WE HAVE BLOCKS OF 4B AS ONE ADDRESS... MT2 QUESTION
        return -1;
    
    //create new vidmap paging structure

    // videoMap();


    *screen_start = (uint8_t*)((34 << 22) + ((0) << 12));
    // page table entry addreses and offset -> screen_start is a virtual address 

    return 0;
}

int32_t set_handler (void *signum, void* handler_address, void* arg3) {
    return -1;
}
int32_t sigreturn (void* arg1, void* arg2, void* arg3) {
    return -1;
}

// ------
/*
 * get_offset
 *   DESCRIPTION: Gets the current offset of the file associated with the current file descriptor.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE:
 *     - Current offset of the file on success.
 *     - 0 if there is no file open.
 *   SIDE EFFECTS: None
 */

int32_t get_offset(void) {
    if (cur_file == -1)
        return 0;
    return pcb_ptr_array[pid]->file_descriptor_table[cur_file].file_position;
}


/*
 * scheduler
 *   DESCRIPTION: Performs process scheduling and context switches between different programs.
 *   INPUTS: None.
 *   OUTPUTS: Performs process scheduling and context switches.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies process control blocks, memory, and system state.
 */

void scheduler() {
    // send_eoi(0);
    // static int32_t last_scheduled = -1;
    static int32_t flag = 0;
    int32_t temp_esp;
    int32_t temp_ebp;
    int32_t cur_running = last_scheduled;
    // printf("Andar hain\n");


    /* launch 1st shell */
    if (terminal_flag[0] == 0) {
        last_scheduled = 0;
        terminal_flag[0] = 1;
        keyboard_init();
        send_eoi(0);
        execute_handler("shell", NULL, NULL);
        return;
    }

    /* launch 2nd shell */
    if (terminal_flag[1] == 0) {
        asm volatile(
            "mov %%esp, %0\n" // newline character fixes issue of eax temp register in asm dump not working correctlys
            "mov %%ebp, %1\n"
            :"=r"(temp_esp), "=r"(temp_ebp)
        );

        store_cur_cursor(0);
        store_cur_buffer(0);

        pcb_ptr_array[0]->ESP_val = temp_esp;
        pcb_ptr_array[0]->EBP_val = temp_ebp;
        pcb_ptr_array[0]->kernel_stack_addr = tss.esp0;

        last_scheduled = 1;
        pcb_ptr_array[1] -> process_number = -1; //unlock
        terminal_flag[1] = 1;

        load_new_cursor(1);
        keyboard_init();
        remap_videoMap(1);
        // select_terminal(2); 

        send_eoi(0);
        execute_handler("shell", NULL, NULL);
        return;
    }

    /* launch 3rd shell */
    if (terminal_flag[2] == 0) {
        asm volatile(
            "mov %%esp, %0\n" // newline character fixes issue of eax temp register in asm dump not working correctlys
            "mov %%ebp, %1\n"
            :"=r"(temp_esp), "=r"(temp_ebp)
        );

        store_cur_cursor(1);
        store_cur_buffer(1);

        pcb_ptr_array[1]->ESP_val = temp_esp;
        pcb_ptr_array[1]->EBP_val = temp_ebp;
        pcb_ptr_array[1]->kernel_stack_addr = tss.esp0;

        last_scheduled = 2;
        pcb_ptr_array[2] -> process_number = -1; //unlock
        terminal_flag[2] = 1;

        keyboard_init();
        load_new_cursor(2);
        remap_videoMap(2);
        // select_terminal(3);
        
        flag = 1;
        send_eoi(0);
        
        execute_handler("shell", NULL, NULL);
        return;
    }

    
    /* Come back to 1st shell */
    if (flag == 1) {
        flag = 2;
        store_cur_cursor(2);
        store_cur_buffer(2);

        select_terminal_buffer(0);
        load_new_cursor(0);
        remap_videoMap(-1);
        last_scheduled = 0;
        pid = 0;

        asm volatile(
            "mov %%esp, %0\n" // newline character fixes issue of eax temp register in asm dump not working correctlys
            "mov %%ebp, %1\n"
            :"=r"(temp_esp), "=r"(temp_ebp)
        );

        pcb_ptr_array[2]->ESP_val = temp_esp;
        pcb_ptr_array[2]->EBP_val = temp_ebp;
        pcb_ptr_array[2]->kernel_stack_addr = tss.esp0;

        tss.ss0 = KERNEL_DS;
        tss.esp0 = pcb_ptr_array[0]->kernel_stack_addr;

        temp_esp = pcb_ptr_array[0]->ESP_val;
        temp_ebp = pcb_ptr_array[0]->EBP_val;

        asm volatile("movl %0, %%esp;"
                    "movl %1, %%ebp;"
                    :
                    : "r"(temp_esp), "r" (temp_ebp)
                    : "esp", "ebp"
                    );

        return;

    }


    last_scheduled++;
    last_scheduled = last_scheduled%6;

    
    /* find next prg to run */
    while(pcb_ptr_array[last_scheduled] -> process_number == -1 || pcb_ptr_array[last_scheduled] -> program_state == TASK_PAUSED) {
        last_scheduled++;
        last_scheduled = last_scheduled % 6;
    }

    pid = last_scheduled;


    parent_cur_running = cur_running;
    parent_last_scheduled = last_scheduled;

    while (parent_cur_running > 2) {
        parent_cur_running = pcb_ptr_array[parent_cur_running] -> parent_program;
    }

    while (parent_last_scheduled > 2) {
        parent_last_scheduled = pcb_ptr_array[parent_last_scheduled] -> parent_program;
    }


    // printf("last sched: %d\n", last_scheduled);
    // if(ticks < 200) {





    store_cur_cursor(parent_cur_running); // cur_running  

    store_enter_flag(parent_cur_running);

    if (parent_last_scheduled == current_terminal) {
        remap_videoMap(-1);
        remap_fish(-1);
    }
    else {
        remap_videoMap(parent_last_scheduled);
        remap_fish(parent_last_scheduled);
    }

    load_new_cursor(parent_last_scheduled);
    load_enter_flag(parent_last_scheduled);



    /* Storing the esp and ebp of the parent program */
    asm volatile(
        "movl %%esp, %0\n" // newline character fixes issue of eax temp register in asm dump not working correctlys
        "movl %%ebp, %1\n"
        :"=r"(temp_esp), "=r"(temp_ebp)
    );

    // state of current programs
    pcb_ptr_array[cur_running]->ESP_val = temp_esp;
    pcb_ptr_array[cur_running]->EBP_val = temp_ebp;
    pcb_ptr_array[cur_running]->kernel_stack_addr = tss.esp0;



    // pid is now i so extract esp and ebp of program that will run next
    temp_esp = pcb_ptr_array[last_scheduled]->ESP_val;
    temp_ebp = pcb_ptr_array[last_scheduled]->EBP_val;

    add_page_entry(last_scheduled); // VA changed to new program

    tss.ss0 = KERNEL_DS;
    tss.esp0 = pcb_ptr_array[last_scheduled]->kernel_stack_addr;

    // if esp0 stored wrongly, when does it crash?
    // -> esp0 for kernel stack bottom... when you have an interrupt or system call or exception occurs, we
    // crash as the esp0 is wrong

    asm volatile("movl %0, %%esp;"
                "movl %1, %%ebp;"
                :
                : "r"(temp_esp), "r" (temp_ebp)
                : "esp", "ebp"
                );

}

/*
 * get_program_state
 *   DESCRIPTION: Retrieves the program state for the current terminal.
 *   INPUTS: None.
 *   OUTPUTS: Returns the parent program ID.
 *   RETURN VALUE: Parent program ID.
 *   SIDE EFFECTS: None.
 */

int32_t get_program_state(int32_t num) {
    return pcb_ptr_array[num] -> program_state;
}

