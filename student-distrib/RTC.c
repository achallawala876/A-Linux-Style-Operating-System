#include "RTC.h"
#include "systemcalls.h"

uint32_t count;
static uint8_t read;
volatile uint32_t rtc_counter;
uint32_t old_count;

syscall_func_t rtc_table[4];
// static terminal_rtc_t terminal1;
// static terminal_rtc_t terminal2;
// static terminal_rtc_t terminal3;
// static terminal_rtc_t* terminal_array [3] = {
//     &terminal1,
//     &terminal2,
//     &terminal3
// };


// void store_cur_rtc(int terminal_num){
//     terminal_array[terminal_num] -> count = count;
//     // terminal_array[terminal_num] -> rtc_counter = rtc_counter; 
//     // terminal_array[terminal_num] -> read = read;

// }

// void read_cur_rtc(int terminal_num){
//     count = terminal_array[terminal_num] -> count ;
//     // rtc_counter=terminal_array[terminal_num] -> rtc_counter ; 
//     // read=terminal_array[terminal_num] -> read;

// }








/*
 * rtc_init
 *
 * DESCRIPTION: Initializes the Real-Time Clock (RTC) hardware and configures it to generate periodic interrupts.
 *
 * INPUTS: None
 *
 * OUTPUTS: RTC hardware is configured to generate periodic interrupts.
 *
 * RETURN VALUE: None (void)
 *
 * SIDE EFFECTS:
 *   - Modifies the RTC configuration by setting Register B to enable periodic interrupts.
 *   - Calculates the 'count' variable based on rtc_max and rtc_min.
 *   - Enables interrupts for the RTC.
 *   - Sets the RTC update rate using rtc_change_rate function.
 */
void rtc_init(){
 unsigned char prev;                 // select register B, (disable NMI is unnecessary for interrupts are not enabled)

    outb(0x0B, 0x70);    // select register B, (disable NMI is unnecessary for interrupts are not enabled)
    prev = inb(0x71);               // read the current value of register B

    outb(0x0B,0x70);     // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);     //write the previous value ORed with 0x40. This turns on bit 6 of register B
    // printf("%d",prev);
    count=rtc_max/rtc_min;
    rtc_counter = rtc_max / rtc_min;
    // outb(0x8A,0x70);
    // prev= inb(0x71);
    enable_irq(8);       
    rtc_change_rate(rtc_max); 

    // Initialize rtc jump table
    rtc_table[0] = &rtc_open;
    rtc_table[1] = &rtc_read;
    rtc_table[2] = &rtc_write;
    rtc_table[3] = &rtc_close;
}

/*
 * rtc_change_rate
 *
 * DESCRIPTION: Changes the update rate of the Real-Time Clock (RTC).
 *
 * INPUTS:
 *   - frequency: The desired frequency (in Hz) for RTC updates.
 *
 * OUTPUTS:
 *   The RTC update rate is adjusted to the specified frequency.
 *
 * RETURN VALUE: None (void)
 *
 * SIDE EFFECTS:
 *   - Modifies the RTC update rate by writing to Register A.
 *   - Disables interrupts temporarily (CLI) during the rate change to ensure atomicity.
 *   - Enables interrupts (STI) after the rate change.
 *
 * NOTES:
 *   - The 'rate' is calculated from the 'frequency' parameter.
 *   - The function returns early if the 'rate' is less than 2, as the fastest rate is 3.
 */
void rtc_change_rate(int32_t frequency) {
    char rate = conv(frequency);       //rate=log2()
    rate &= 0x0F;

    if (rate < 2){                             // fastest rate selection is 3
        return;                                       // thus if rate is less than 3, return
    }
    cli();
    outb(0x0A, 0x70);                  // set index to register A, (disable NMI is unnecessary for interrupts are not enabled)
    char prev = inb(0x71);                        // get initial value of register A
    outb(0x0A, 0x70);                  // reset index to A
    outb((prev & 0xf0) | rate, 0x71);        // write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
}

/*
 * rtc_intr_handler
 *
 * DESCRIPTION: Interrupt handler for the Real-Time Clock (RTC).
 *
 * INPUTS: None
 *
 * OUTPUTS: Handles RTC interrupts.
 *
 * RETURN VALUE: None (void)
 *
 * SIDE EFFECTS:
 *   - Disables interrupts (CLI) to ensure atomicity.
 *   - Reads from the RTC and sets the 'read' flag to 1.
 *   - Handles RTC interrupt actions (e.g., printing, processing, or other operations).
 *   - Enables interrupts (STI) after processing the interrupt.
 *   - Sends an End of Interrupt (EOI) signal to acknowledge the interrupt.
 *   - May include additional functionality (commented out code) for handling RTC interrupts.
 */
void rtc_intr_handler(){
    cli();
    
    // test_interrupts();
    // unsigned char prev;char ascii;
    outb(0x0C,0x70);
    inb(0x71);
    
    // ascii=scancode_to_ascii_table[(prev%10)];
    // putc(ascii);
    rtc_counter++;
    
    if(rtc_counter >=count) {
        read=1;
        // if ((parent_last_scheduled)==2){
        rtc_counter=0;
        // }
        // else if ((parent_last_scheduled)==1){
        //     rtc_counter=count/(3-parent_last_scheduled);
        // }
        // else{
        //     rtc_counter=count;
        // }
    }

    // printf(" RTC caught\n");
    sti();
    send_eoi(8);
}

/*
 * rtc_open
 *
 * DESCRIPTION: Opens the Real-Time Clock (RTC) for use.
 *
 * INPUTS:
 *   - filename: A pointer to the filename (not used in this implementation).
 *
 * OUTPUTS: Configures the RTC update rate for use.
 *
 * RETURN VALUE: Always returns 0 to indicate success.
 *
 * SIDE EFFECTS:
 *   - Disables interrupts (CLI) to ensure atomicity.
 *   - Calls rtc_change_rate to set the RTC update rate (e.g., 2 Hz).
 *   - Enables interrupts (STI) after configuring the RTC.
 */
int32_t rtc_open (int32_t filename, void* arg2, int32_t arg3){
    // count=rtc_max/rtc_min;
    // cli();
    count=rtc_max/rtc_min;
    // rtc_change_rate(2);
    // sti();
    return 0;
}

/*
 * rtc_close
 *
 * DESCRIPTION: Closes the Real-Time Clock (RTC) for use.
 *
 * INPUTS:
 *   - fd: The file descriptor for the RTC (not used in this implementation).
 *
 * RETURN VALUE: Always returns 0 to indicate success.
 *
 * SIDE EFFECTS: None
 */
int32_t rtc_close (int32_t fd, void* arg2, int32_t arg3){
    count=old_count;
    return 0;
}

/*
 * rtc_write
 *
 * DESCRIPTION: Writes to the Real-Time Clock (RTC) to set the update frequency.
 *
 * INPUTS:
 *   - fd: The file descriptor for the RTC (not used in this implementation).
 *   - buf: A pointer to the frequency value to set for the RTC.
 *   - nbytes: The number of bytes to write (should be sizeof(uint32_t)).
 *
 * RETURN VALUE:
 *   - 0: If the write operation is successful and the frequency is valid.
 *   - -1: If the write operation fails due to invalid frequency or buffer size.
 *
 * SIDE EFFECTS:
 *   - Disables interrupts (CLI) to ensure atomicity.
 *   - Calls rtc_change_rate to set the RTC update rate based on the provided frequency.
 *   - Enables interrupts (STI) after configuring the RTC.
 */
int32_t rtc_write (int32_t fd, void* buf, int32_t nbytes){
    uint32_t freq= *((int*)buf);
    if (((freq&(freq-1))==0)&&(freq<rtc_max||freq>rtc_min)&&(buf!=NULL)&&(nbytes==sizeof(uint32_t))){
        // cli();
        if (parent_last_scheduled!=0){
            old_count=count;
            count=rtc_max/(freq*(parent_last_scheduled+1));}
        else 
            count=rtc_max/(freq);
        // rtc_change_rate(freq);
        // sti();
        return 0;
    }
    return -1;
}

/*
 * rtc_read
 *
 * DESCRIPTION: Reads from the Real-Time Clock (RTC) to wait for the next RTC interrupt.
 *
 * INPUTS:
 *   - fd: The file descriptor for the RTC (not used in this implementation).
 *   - buf: A pointer to the buffer where the read data will be stored (not used in this implementation).
 *   - nbytes: The number of bytes to read (not used in this implementation).
 *
 * RETURN VALUE:
 *   - 0: If the read operation is successful and has waited for the next RTC interrupt.
 *
 * SIDE EFFECTS:
 *   - Resets the 'read' flag to 0 and enters a loop to wait for the next RTC interrupt.
 *   - The loop continues until 'read' is set to 1 by the RTC interrupt handler.
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){
    read=0;
    while(!read);
    return 0;
}

/*
 * conv
 *
 * DESCRIPTION: Converts a frequency value to the corresponding rate for configuring the Real-Time Clock (RTC).
 *
 * INPUTS:
 *   - frequency: The desired frequency in Hz.
 *
 * RETURN VALUE:
 *   - The rate value that corresponds to the provided frequency.
 *   - -1 if the provided frequency is not in the supported list.
 *
 * SIDE EFFECTS: None
 */
int conv(int32_t frequency){
    int rate;
    rate = -1;

    switch ((int)frequency) {
        case 16384:
            rate = 6;
            break;
        case 8192:
            rate = 6;
            break;
        case 4096:
            rate = 4;
            break;
        case 2048:
            rate = 5;
            break;
        case 1024:
            rate = 6;
            break;
        case 512:
            rate = 7;
            break;
        case 256:
            rate = 8;
            break;
        case 128:
            rate = 9;
            break;
        case 64:
            rate = 10;
            break;
        case 32:
            rate = 11;
            break;
        case 16:
            rate = 12;
            break;
        case 8:
            rate = 13;
            break;
        case 4:
            rate = 14;
            break;
        case 2:
            rate = 15;
            break;
        case 1:
            rate = 16;
            break;
    }

    return rate;
}

