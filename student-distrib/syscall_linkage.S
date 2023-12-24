.data
    /*  Define jump table for sys calls */
    jump_table:
        .long halt_handler
        .long execute_handler
        .long read_handler
        .long write_handler
        .long open_handler
        .long close_handler
        .long getargs
        .long vidmap
        .long set_handler
        .long sigreturn

.globl syscall_handler_linkage

// FROM ece391syscall.S file (through int 0x80) in KERNEL

syscall_handler_linkage:

    cmpl $1, %eax
    jl return_invalid

    cmpl $10, %eax
    jg return_invalid
    
    // check sys call validity 

    decl %eax

    /* save all regs ? */

    // why not all? of 7, 3 contain arguments...?

    pushl %ebp
    pushl %esi
    pushl %edi
    pushfl

    /* push args */
    pushl %edx
    pushl %ecx
    pushl %ebx

    call *jump_table(, %eax, 4) // correct sys call handler


    /* Remove args */
    addl $12, %esp

    popfl
    popl %edi
    popl %esi
    popl %ebp

    iret // take ME BACKKKKK TO ece391syscall.S


return_invalid:
		movl $-1, %eax
		iret
