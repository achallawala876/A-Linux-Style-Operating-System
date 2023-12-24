#ifndef _INIT_IDT_H
#define _INIT_IDT_H

#include "idt_linkage.h"
#include "syscall_linkage.h"


extern void idt_initialize();

extern void syscall_handler_linkage();
extern void default_exception_handler();
extern void default_interrupt_handler();

/* System call handlers*/
// int32_t halt_handler(void* status, void*, void*);
// int32_t execute_handler(void* command_in, void*, void*);
// int32_t read_handler(void* fd_in, void* buf_in, void* nbytes_in);
// int32_t write_handler(void* fd_in, void* buf_in, void* nbytes_in);
// int32_t open_handler(void* filename_in, void* arg2, void* arg3);
// int32_t close_handler(void* fd_in, void* arg2, void* arg3);

/* Exceptions */
extern void division_error_handler();
extern void debug_exception_handler();
extern void nmi_handler();
extern void breakpoint_handler();
extern void overflow_handler();
extern void bound_range_exceeded_handler();
extern void invalid_opcode_handler();
extern void device_noa_handler();
extern void double_fault_handler();
extern void cso_handler();
extern void invalid_tss_handler();
extern void segment_np_handler();
extern void stack_seg_fault_handler();
extern void general_protection_fault_handler();
extern void page_fault_handler();

/* Interrupts */
extern void keyboard_intr_handler();
extern void rtc_intr_handler();
extern void pit_intr_handler();
extern void mouse_intr_handler();
// /* PCB and stuff related to the active process */
// typedef struct file_descriptor {
//     // each is 4 bytes
//     syscall_func_t *file_operations_jump_table;
//     uint32_t file_position;
//     uint32_t inode;
//     uint32_t flags;
// } file_descriptor_t;

// typedef struct process_control_block {
//     int32_t EBP_val, ESP_val;
//     uint32_t process_number;
//     pcb_t *parent_program; // think of data structure; if this doies not work just store address as an integer
//     uint32_t program_state;
//     file_descriptor_t file_descriptor_table [8]; // 8 entries in FDT
// } pcb_t;

// pcb_t pcb_array[8]; // max 8 programs?

#endif /* _INIT_IDT_H */
