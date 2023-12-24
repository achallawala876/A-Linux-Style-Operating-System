/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
unsigned char a1, a2;

/* Initialize the 8259 PIC */
void i8259_init(void) {

	// a1 = inb(PIC1_DATA);                        // save masks
	// a2 = inb(PIC2_DATA);
    outb(0xff,PIC1_DATA);
    outb(0xff,PIC2_DATA);
 
	outb(ICW1,MASTER_8259_PORT);  // starts the initialization sequence (in cascade mode)
	// io_wait();
	outb(ICW2_MASTER,PIC1_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();
	outb(ICW3_MASTER,PIC1_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();
	outb(ICW4,PIC1_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();

    ////////////////////////////////////////////
    outb(ICW1,SLAVE_8259_PORT);   // starts the initialization sequence (in cascade mode)
	// io_wait();
	outb(ICW2_SLAVE,PIC2_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();
	outb(ICW3_SLAVE,PIC2_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();
	outb(ICW4,PIC2_DATA);                 // ICW2: Master PIC vector offset
	// io_wait();

    ///////////////////////////////////////////
	// outb(PIC1_DATA, 0x01);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	// // io_wait();
	// outb(PIC2_DATA, 0x01);
	// // io_wait();
 
	// outb(PIC1_DATA, a1);   // restore saved masks.
	// outb(PIC2_DATA, a2);

    // outb(a1, PIC1_DATA);   // restore saved masks.
	// outb(a2, PIC2_DATA);
    // outb(master_mask, PIC1_DATA);   // restore saved masks.
	// outb(slave_mask, PIC2_DATA);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    // uint16_t x;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
        // value = inb(PIC1_DATA) & ~(1 << irq_num);
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
        // value = a2 & ~(1 << irq_num);
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value,port);  
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    // uint16_t x;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
        // value = inb(port) | (1 << irq_num);
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
        // value = a2 | (1 << irq_num);
    }
    value = inb(port) | (1 << irq_num);
    
    outb(value,port); 
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num < 8)
		outb(EOI | irq_num,PIC1_COMMAND);
    else{
	outb(EOI | (irq_num-8),PIC2_COMMAND);
    outb(EOI | 2,PIC1_COMMAND);}

}
