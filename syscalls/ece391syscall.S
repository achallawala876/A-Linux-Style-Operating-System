#include "ece391sysnum.h"

/* 
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */
#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\ // caller save ecx and eax??

	MOVL	$number,%EAX  ;\ // PUSH ARGUMENTS ON USER STACK TO REGISTERS
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	
	// dont mess with eax as it will have return value here
	// come back here after iret in syscall_linkage.STACK
	POPL	%EBX          ;\
	RET // go back to user program (this assembly is in userspace)

// FROM MAIN OF USER, THESE ARE CALLED WHICH INVOKES THIS ASSEMBLY LINKAGE TO INT X80
/* the system call library wrappers */
DO_CALL(ece391_halt,SYS_HALT)
DO_CALL(ece391_execute,SYS_EXECUTE)
DO_CALL(ece391_read,SYS_READ)
DO_CALL(ece391_write,SYS_WRITE)
DO_CALL(ece391_open,SYS_OPEN)
DO_CALL(ece391_close,SYS_CLOSE)
DO_CALL(ece391_getargs,SYS_GETARGS)
DO_CALL(ece391_vidmap,SYS_VIDMAP)
DO_CALL(ece391_set_handler,SYS_SET_HANDLER)
DO_CALL(ece391_sigreturn,SYS_SIGRETURN)


/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
    PUSHL   $0
    PUSHL   $0
	PUSHL	%EAX
	CALL	ece391_halt

