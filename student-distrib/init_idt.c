#include "x86_desc.h"
#include "lib.h"
#include "init_idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "../syscalls/ece391sysnum.h"
#include "i8259.h"
#include "paging.h"
#include "systemcalls.h"
#include "pit.h"
// #include "file_system.h"
// #include "RTC.h"

// #define KEYBOARD_DATA_PORT 0x60
static unsigned char keyboard_map[128] = {
  KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
  '-', '=', KEY_BACKSPACE, KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u',
  'i', 'o', 'p', '[', ']', KEY_ENTER, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
  'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
  ',', '.', '/', 0, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
  KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
  KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
  KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
};
// --------------------------- Default Handlers -------------------------


void default_exception_handler() {
    printf("Exception caught\n");
    while(1){}
}

void default_interrupt_handler() {
    printf("Interrupt caught\n");
}

// --------------------------- Exceptions 0x00 - 0x1F -------------------------

void division_error_handler () {
    printf(" Division error caught\n");
    while(1){}
}

void debug_exception_handler() {
    printf(" Debug exception caught\n");
    while(1){}    
}

void nmi_handler () {
    printf(" Non-maskable interrupt caught\n");
    while(1){}
}

void breakpoint_handler() {
    printf(" Breakpoint interrupt caught\n");
    while(1){}
}

void overflow_handler () {
    printf(" Overflow caught\n");
    while(1){}
}

void bound_range_exceeded_handler () {
    printf(" Bound range exceeded caught\n");
    while(1){}
}

void invalid_opcode_handler () {
    printf(" Invalid opcode caught\n");
    // halt_handler(256, 6, 8);
    while(1){}
}

void device_noa_handler() {
    printf(" Device not available caught\n");
    while(1){}    
}
void double_fault_handler() {
    printf(" Double fault caught\n");
    while(1){}    
}
void cso_handler() {
    printf(" Coprocessor Segment Overrun caught\n");
    while(1){}    
}
void invalid_tss_handler() {
    printf(" Invalid TSS caught\n");
    while(1){}     
}

void segment_np_handler () {
    printf(" Segment not present caught\n");
    while(1){}
}

void stack_seg_fault_handler () {
    printf(" Stack segment fault caught\n");
    while(1){}
}

void general_protection_fault_handler () {
    printf(" General protection fault caught\n");
    while(1){}
}

void page_fault_handler () {
    printf("Page fault caught  \n");
    uint32_t faulting_address;

    // Retrieve the faulting address from CR2
    asm volatile("movl %%cr2, %0" : "=r" (faulting_address));

    printf("Page Fault at address 0x%x  \n", faulting_address);
    halt_handler((void*)256, NULL, NULL);
    // while(1){}
}

// --------------------------- Interrupts 0x20 - 0x2F ------------------------- 
void keyboard_intr_handler(){
    // disbale irq and send EOI??
    // printf(" keyboard interrupt caught\n");
    cli();
    uint8_t key_in = inb(KEYBOARD_DATA_PORT);
    uint8_t scan_code = key_in & 0x7f;
    uint8_t key_state = !(key_in & 0x80);
    keyboard.keys[scan_code] = key_state;
    keyboard.chars[keyboard_map[scan_code]] = key_state;

    char ascii;
    // int prev_terminal = current_terminal;
    // int prev_pid = pid;
    // int parent_prg = get_parent_program();
    

    // if (parent_prg != current_terminal - 1) { // we were servicing program in another terminal
    //     store_cur_buffer(current_terminal); // store buffer of program being serviced
    //     select_terminal_buffer(current_terminal); // select new buffer
    //     store_cur_cursor(parent_prg + 1); // store buffer of program that was being serviced
        
    //     remap_videoMap(-1); // remap to current screen (184)
    //     flush_tlb();
    //     load_new_cursor(current_terminal); // load new cursor

    //     // pid = current_terminal - 1;
    // }

    // printf("%d: ", key_in);

    store_enter_flag(parent_last_scheduled);
    load_enter_flag(current_terminal);


    ascii = get_ascii(key_in);

    store_enter_flag(current_terminal);
    load_enter_flag(parent_last_scheduled);
    // remap_videoMap(-1);
    // flush_tlb(); 
    if (ascii == 0) {
    
        // if (parent_prg != current_terminal - 1) {
        //     store_cur_buffer(current_terminal);
        //     select_terminal_buffer(get_parent_program() + 1);
        //     store_cur_cursor(current_terminal);
            
        //     remap_videoMap(parent_prg);
        //     flush_tlb();
        //     load_new_cursor(parent_prg + 1);
        // }
        // remap_videoMap(get_parent_program());
        // flush_tlb();
        // pid = prev_pid;
        send_eoi(1);
        sti();
        return;
    }

    else {
        // printf("%d: ", key_in);
        // store_cur_cursor()
        // remap_videoMap(-1);
        // flush_tlb(); 

        if (current_terminal != parent_last_scheduled) {
            store_cur_cursor(parent_last_scheduled);
            
            load_new_cursor(current_terminal);
            remap_videoMap(-1);
        }

        // if (get_program_state(current_terminal) != 2) {
        putc(ascii);
            // update_cursor();
        // }

        if (current_terminal != parent_last_scheduled) {
            store_cur_cursor(current_terminal);
            load_new_cursor(parent_last_scheduled);
            remap_videoMap(parent_last_scheduled);
        }

        // if (parent_prg != current_terminal - 1) {
        //     store_cur_buffer(current_terminal);
        //     select_terminal_buffer(get_parent_program() + 1);
        //     store_cur_cursor(current_terminal);
            
        //     remap_videoMap(parent_prg);
        //     flush_tlb();
        //     load_new_cursor(parent_prg + 1);
        // }
        // flush_tlb();
        // pid = prev_pid;
        send_eoi(1);
        sti();
    }

    // while (1) {} // this is interesting... shouldn't it get interrupted?
    
    // if (select_t1) select_terminal(1);

    // else if (select_t2) select_terminal(2);

    // else if (select_t3) select_terminal(3);
        
}


// --------------------------- Initialization ------------------------- 

/*
 * set_system_call_in_idt
 *   DESCRIPTION: Sets up the system call entry in the Interrupt Descriptor Table (IDT).
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Configures the IDT entry for system calls.
 */

void set_system_call_in_idt() {
    idt[0x80].seg_selector = KERNEL_CS;
    idt[0x80].reserved4 = 0;
    idt[0x80].reserved3 = 1; // trap gate
    idt[0x80].reserved2 = 1;
    idt[0x80].reserved1 = 1;
    idt[0x80].reserved0 = 0;
    idt[0x80].size = 1;
    idt[0x80].dpl = 3;
    idt[0x80].present = 1;
    SET_IDT_ENTRY(idt[0x80], syscall_handler_linkage);    

}

/*
 * idt_initialize
 *   DESCRIPTION: Initializes the Interrupt Descriptor Table (IDT) for handling exceptions, interrupts, and system calls.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Initializes the IDT with exception and interrupt handlers and sets up system call entries.
 */
void idt_initialize() {
    uint8_t i;

    /* Initialize exceptions an interrupts */
    for (i = 0; i <= 0x2F; i++) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].dpl = 0;
        idt[i].present = 1;
        if (i <= 0x1F)
            SET_IDT_ENTRY(idt[i], default_exception_handler_linkage); // Exceptions
        else
            // printf("%d",i);
            SET_IDT_ENTRY(idt[i], default_interrupt_handler_linkage); // Interrupts
    }

    /* Initialize system calls*/
    set_system_call_in_idt();

    /* Exceptions */
    SET_IDT_ENTRY(idt[0], division_error_handler_linkage);
    SET_IDT_ENTRY(idt[1], debug_exception_handler_linkage);
    SET_IDT_ENTRY(idt[2], nmi_handler_linkage);
    SET_IDT_ENTRY(idt[3], breakpoint_handler_linkage);
    SET_IDT_ENTRY(idt[4], overflow_handler_linkage);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_handler_linkage);
    SET_IDT_ENTRY(idt[6], invalid_opcode_handler_linkage);
    SET_IDT_ENTRY(idt[7], device_noa_handler_linkage);
    SET_IDT_ENTRY(idt[8], double_fault_handler_linkage);
    SET_IDT_ENTRY(idt[9], cso_handler_linkage);
    SET_IDT_ENTRY(idt[10], invalid_tss_handler_linkage);
    SET_IDT_ENTRY(idt[11], segment_np_handler_linkage);
    SET_IDT_ENTRY(idt[12], stack_seg_fault_handler_linkage);
    SET_IDT_ENTRY(idt[13], general_protection_fault_handler_linkage);
    SET_IDT_ENTRY(idt[14], page_fault_handler_linkage);

    /* Interrupts */
    SET_IDT_ENTRY(idt[0x20], pit_intr_handler_linkage);
    SET_IDT_ENTRY(idt[0x21], keyboard_intr_handler_linkage);
    SET_IDT_ENTRY(idt[0x28], rtc_intr_handler_linkage);
    SET_IDT_ENTRY(idt[0x2C], mouse_intr_handler_linkage);
    /* Load IDT */
    lidt(idt_desc_ptr);
}

