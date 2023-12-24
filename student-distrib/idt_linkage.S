/*
 * INTR_LINK
 *   DESCRIPTION: A macro for defining interrupt linkage for assembly functions.
 *   INPUTS: name - The name of the interrupt handler.
 *           func - The function to be called when the interrupt occurs.
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Defines an interrupt handler with the specified name and function.
 */

#define INTR_LINK(name, func)   \
.globl name                ;\
name: pushal   ;\
    pushfl   ;\
    call func    ;\
    popfl    ;\
    popal    ;\
    iret     ;\

# supposedly iret does sti as well

# INTR_LINK(system_call_handler_linkage, system_call_handler);
INTR_LINK(default_exception_handler_linkage, default_exception_handler);
INTR_LINK(default_interrupt_handler_linkage, default_interrupt_handler);

INTR_LINK(division_error_handler_linkage, division_error_handler);
INTR_LINK(debug_exception_handler_linkage, debug_exception_handler);
INTR_LINK(nmi_handler_linkage, nmi_handler);
INTR_LINK(breakpoint_handler_linkage, breakpoint_handler);
INTR_LINK(overflow_handler_linkage, overflow_handler);
INTR_LINK(bound_range_exceeded_handler_linkage, bound_range_exceeded_handler);
INTR_LINK(invalid_opcode_handler_linkage, invalid_opcode_handler);
INTR_LINK(device_noa_handler_linkage, device_noa_handler);
INTR_LINK(double_fault_handler_linkage, double_fault_handler);
INTR_LINK(cso_handler_linkage, cso_handler);
INTR_LINK(invalid_tss_handler_linkage, invalid_tss_handler);
INTR_LINK(segment_np_handler_linkage, segment_np_handler);
INTR_LINK(stack_seg_fault_handler_linkage, stack_seg_fault_handler);
INTR_LINK(general_protection_fault_handler_linkage, general_protection_fault_handler);
INTR_LINK(page_fault_handler_linkage, page_fault_handler);

INTR_LINK(pit_intr_handler_linkage, pit_intr_handler);
INTR_LINK(keyboard_intr_handler_linkage, keyboard_intr_handler);
INTR_LINK(rtc_intr_handler_linkage, rtc_intr_handler);
INTR_LINK(mouse_intr_handler_linkage, mouse_intr_handler);





