#ifndef _IDT_LINKAGE_H
#define _IDT_LINKAGE_H

// void system_call_handler_linkage();
void default_exception_handler_linkage();
void default_interrupt_handler_linkage();

/* Exceptions */
void division_error_handler_linkage();
void debug_exception_handler_linkage();
void nmi_handler_linkage();
void breakpoint_handler_linkage();
void overflow_handler_linkage();
void bound_range_exceeded_handler_linkage();
void invalid_opcode_handler_linkage();
void device_noa_handler_linkage();
void double_fault_handler_linkage();
void cso_handler_linkage();
void invalid_tss_handler_linkage();
void segment_np_handler_linkage();
void stack_seg_fault_handler_linkage();
void general_protection_fault_handler_linkage();
void page_fault_handler_linkage();

/* Interrupts */
void keyboard_intr_handler_linkage();
void rtc_intr_handler_linkage();
void pit_intr_handler_linkage();
void mouse_intr_handler_linkage();


#endif /* _IDT_LINKAGE_H */
