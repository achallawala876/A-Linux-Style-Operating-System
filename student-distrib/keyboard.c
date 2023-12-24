#include "lib.h"
#include "i8259.h"
#include  "keyboard.h"
#include "systemcalls.h"

#define CAPS_LOCK 58
#define ENTER 28
#define LEFT_SHIFT_ON 42
#define LEFT_SHIFT_OFF 170
#define RIGHT_SHIFT_ON 54
#define RIGHT_SHIFT_OFF 182
#define CTRL_ON 29
#define CTRL_OFF 157
#define BACKSPACE 14

#define LEFT_ALT_ON 56 // 0x38 
#define LEFT_ALT_OFF 184 // 0xB8
#define F1_ON 59 // 0x3B
#define F2_ON 60 // 0x3C
#define F3_ON 61 // 0x3D

// #define BUFSIZE 128

static char scancode_to_ascii_table[] = {

    0,   0,  '1', '2', '3', '4', '5', '6', '7', '8',  // , esc, 0-9
    '9', '0', '-', '=', '\b',                         // Backspace
    '\t',                                             // Tab
    'q', 'w', 'e', 'r',                               // 10-19
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     // Enter key
    0,                                                // 29 - Control
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 30-39
    '\'', '`', 0,                                     // Left shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',               // 40-49
    'm', ',', '.', '/', 0,                            // Right shift
    '*',
    0,   // Alt
    ' ', // Space bar
    0,   // Caps lock
    0,   // 59 - F1 key ... >
    0, 0, 0, // F2 - F4
    0, 0, 0, 0, 0,
    0,   // < ... F10
    0,   // 69 - Num lock
    0,   // Scroll Lock
    0,   // Home key
    0,   // Up Arrow
    0,   // Page Up
    '-',
    0,   // Left Arrow
    0,
    0,   // Right Arrow
    '+',
    0,   // 79 - End key
    0,   // Down Arrow
    // 0,   // Page Down
    // 0,   // Insert Key
    // 0,   // Delete Key
    // 0, 0, 0,
    // 0,   // F11 Key
    // 0,   // F12 Key
    // 0    // All other keys are undefined

};

static char scancode_to_ascii_table_capital[] = {

    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',  // 0-9
    '(', ')', '_', '+', '\b',                         // Backspace
    0,                                             // Tab
    'Q', 'W', 'E', 'R',                               // 10-19
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',     // Enter key
    0,                                                // 29 - Control
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 30-39
    '\"', '~', 0,                                     // Left shift
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',               // 40-49
    'M', '<', '>', '?', 0,                            // Right shift 
};

char buffer[BUFSIZE]; // main buffer
// char buffer1[BUFSIZE];
// char buffer2[BUFSIZE];
// char buffer3[BUFSIZE];

static int index = -1; // print buffer till this index, if < 0 --> bufer empty
// static int index1  = -1;
// static int index2  = -1;
// static int index3  = -1;

static int caps_flag = 0; // 0 = not active, 1 = active
static int shift_flag = 0; // 0 = not active, 1 = active
static int read_flag = 0; // 0 = not active, 1 = active
static int ctrl_flag = 0; // 0 = not active, 1 = active
static int enter_flag = 0; // 0 = not active, 1 = active
static int alt_flag = 0; // 0 = not active, 1 = active
// static int f1_flag = 0; // 0 = not active, 1 = active
// static int f2_flag = 0; // 0 = not active, 1 = active
// static int f3_flag = 0; // 0 = not active, 1 = active

static terminal_info_t terminal1;
static terminal_info_t terminal2;
static terminal_info_t terminal3;

static terminal_info_t* terminal_array [3] = {
    &terminal1,
    &terminal2,
    &terminal3
};

syscall_func_t terminal_table[4];

bool keyboard_key(uint8_t scan_code) { return keyboard.keys[scan_code]; }
bool keyboard_char(unsigned char c) { return keyboard.chars[c]; }

void store_cur_buffer (int terminal_num) {
    memcpy(terminal_array[terminal_num] -> terminal_buffer, buffer, BUFSIZE);
    terminal_array[terminal_num] -> terminal_index = index;
    terminal_array[terminal_num] -> terminal_caps_flag = caps_flag; 
    terminal_array[terminal_num] -> terminal_shift_flag = shift_flag; 
    terminal_array[terminal_num] -> terminal_read_flag = read_flag;
    terminal_array[terminal_num] -> terminal_ctrl_flag = ctrl_flag; 
    terminal_array[terminal_num] -> terminal_enter_flag = enter_flag; 
    terminal_array[terminal_num] -> terminal_alt_flag = alt_flag;

    // switch (terminal_num) {
    //     case 1:
    //         memcpy(buffer1, buffer, BUFSIZE);
    //         index1 = index;
    //         break;

    //     case 2:
    //         memcpy(buffer2, buffer, BUFSIZE);
    //         index2 = index;
    //         break;

    //     case 3:
    //         memcpy(buffer3, buffer, BUFSIZE);
    //         index3 = index;
    //         break;

    //     default:
    //         memcpy(buffer1, buffer, BUFSIZE);
    //         index1 = index;
    // }
}

/*
 * select_terminal_buffer
 *   DESCRIPTION: Switches the active terminal's buffer and updates keyboard state flags.
 *   INPUTS:
 *     - terminal_num: Terminal number to switch to.
 *   OUTPUTS: Updates global variables for keyboard state and buffer.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies global variables.
 */

void select_terminal_buffer(int terminal_num) {


    memcpy(buffer, terminal_array[terminal_num] -> terminal_buffer, BUFSIZE);
    index = terminal_array[terminal_num] -> terminal_index; 
    caps_flag = terminal_array[terminal_num] -> terminal_caps_flag; 
    shift_flag = terminal_array[terminal_num] -> terminal_shift_flag; 
    read_flag = terminal_array[terminal_num] -> terminal_read_flag; 
    ctrl_flag = terminal_array[terminal_num] -> terminal_ctrl_flag; 
    enter_flag = terminal_array[terminal_num] -> terminal_enter_flag; 
    alt_flag = terminal_array[terminal_num] -> terminal_alt_flag;
}

/*
 * store_enter_flag
 *   DESCRIPTION: Stores the enter flag for a specific terminal.
 *   INPUTS:
 *     - terminal_num: Terminal number for which to store the enter flag.
 *   OUTPUTS: Updates the enter flag for the specified terminal.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies the terminal's enter flag.
 */
void store_enter_flag(int terminal_num) {
    terminal_array[terminal_num] -> terminal_enter_flag = enter_flag;
    // terminal_array[terminal_num] -> terminal_read_flag = read_flag;
}

/*
 * load_enter_flag
 *   DESCRIPTION: Loads the enter flag for a specific terminal.
 *   INPUTS:
 *     - terminal_num: Terminal number for which to load the enter flag.
 *   OUTPUTS: Updates the global enter flag based on the specified terminal.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies the global enter flag.
 */

void load_enter_flag(int terminal_num) {
    enter_flag = terminal_array[terminal_num] -> terminal_enter_flag; 
    // read_flag = terminal_array[terminal_num] -> terminal_read_flag;
}

/*
 * get_ascii
 *   DESCRIPTION: Converts a key code (scancode) into ASCII characters based on the keyboard state.
 *   INPUTS:
 *     - key_in: Scancode of the pressed key.
 *   OUTPUTS: Returns the corresponding ASCII character or 0 if not applicable.
 *   RETURN VALUE: ASCII character or 0.
 *   SIDE EFFECTS: Modifies global flags and buffer.
 */

char get_ascii(uint8_t key_in) {
    
    char ascii;

    // select_terminal_buffer(current_terminal);
    if (index >= BUFSIZE - 1 && key_in != ENTER)
        return 0;
    
    // key within array or LEFT_SHIFT_OFF or RIGHT_SHIFT_OFF or CTRL_OFF
    if((key_in < sizeof(scancode_to_ascii_table)) || (key_in == LEFT_SHIFT_OFF) || (key_in == RIGHT_SHIFT_OFF) || (key_in == CTRL_OFF) || (key_in == LEFT_ALT_OFF)) {
        switch(key_in) {

            /* Caps Lock*/
            case CAPS_LOCK :
                caps_flag = (caps_flag + 1) % 2;
                return 0;
            
            /* Enter */
            case ENTER:
                if (read_flag && index < BUFSIZE-1) {
                    buffer[++index] = '\n';
                }
                /*  reset buffer index to 0 */
                    index = -1;
                    enter_flag = 1;
                
                return '\n';
            
            /* Left or right shift is pressed */
            case LEFT_SHIFT_ON:
            case RIGHT_SHIFT_ON:
                shift_flag = 1;
                return 0;
            
            /* Left or right shift is released */
            case LEFT_SHIFT_OFF:
            case RIGHT_SHIFT_OFF:
                shift_flag = 0;
                return 0;
            
            /* Backspace */
            case BACKSPACE:
                if (read_flag) {
                    if (index >= 0) {
                        index--;
                        return '\b';
                    }
                }
                else
                    return 0;
                // return '\b';
            
            /* Ctrl is pressed */
            case CTRL_ON:
                ctrl_flag = 1;
                return 0;
            
            /* Ctrl is released */
            case CTRL_OFF:
                ctrl_flag = 0;
                return 0;

            /* Left Alt is pressed */
            case LEFT_ALT_ON:
                alt_flag = 1;
                return 0;
            /* Left Alt is released */
            case LEFT_ALT_OFF:
                alt_flag = 0;
                return 0;

            case F1_ON:
                // if (alt_flag) clear_screen();
                if (alt_flag){
                    // send_eoi(1);
                    select_terminal(0);
                    return 0;
                }
                return 0;

            // case F1_OFF:
            //     f1_flag = 0;
            //     return 0;

            case F2_ON:
                // if (alt_flag) clear_screen();
                if (alt_flag){
                    // send_eoi(1);
                    select_terminal(1);
                    return 0;
                }
                return 0;

            // case F2_OFF:
            //     f2_flag = 0;
            //     return 0;

            case F3_ON:
                // if (alt_flag) clear_screen();
                if (alt_flag){
                    // send_eoi(1);
                    select_terminal(2);
                    return 0;
                }
                return 0;

            // case F3_OFF:
            //     f3_flag = 0;
            //     return 0;

            /* alphabets, numbers, and symbols */
            default:
                ascii = scancode_to_ascii_table[key_in];

                /* alphabets */
                if( (int)(ascii) >= 97 && (int)(ascii) <= 122) {
                    if (ctrl_flag && ascii == 'k') { 
                        clear_screen();
                        return 0;
                    }
                    else if (caps_flag && shift_flag)
                        ascii = scancode_to_ascii_table[key_in];
                    else if (caps_flag || shift_flag)
                        ascii = scancode_to_ascii_table_capital[key_in];
                    else
                        ascii = scancode_to_ascii_table[key_in];
                    
                    if (read_flag) {
                        if (index < BUFSIZE - 1)
                            buffer[++index] = ascii;
                        else
                            ++index;
                    }
                    return ascii;
                }

                /* number or symbol */
                else {

                    if(shift_flag)
                        ascii = scancode_to_ascii_table_capital[key_in];
                    else
                        ascii = scancode_to_ascii_table[key_in];

                    /////////////////////////////// what is this?

                    if (read_flag) {
                        if (index < BUFSIZE - 1)
                            buffer[++index] = ascii;
                        else
                            ++index;
                    }

                    ////////////////////////////// terminal swapping

                    // if (alt_flag && ascii == '1'){
                    //     send_eoi(1);
                    //     select_terminal(1);
                    //     // select_t1 = 1;
                    //     // select_t2 = 0;
                    //     // select_t3 = 0;
                    //     return 0;
                    // }

                    // else if (alt_flag && ascii == '2'){
                    //     send_eoi(1);
                    //     select_terminal(2);
                    //     // select_t1 = 0;
                    //     // select_t2 = 1;
                    //     // select_t3 = 0;
                    //     return 0;
                    // }

                    // else if (alt_flag && ascii == '3'){
                    //     send_eoi(1);
                    //     select_terminal(3);
                    //     // select_t1 = 0;
                    //     // select_t2 = 0;
                    //     // select_t3 = 1;
                    //     return 0;
                    // }

                    //////////////////////////// skip over if we aren't doing anything with the alt key
                    
                    return ascii;
                }
        }   
    }

    /* Outside of valid range */
    else {
        return 0;
    }
}

/* keyboard_init()
 * 
 * Iniatilizes keyboard device
 * Inputs: None
 * Outputs: None
 * Side Effects: Enable keyboard interrupt (1) on PIC
 */
void keyboard_init(){
    caps_flag = 0; 
    shift_flag = 0;
    read_flag = 0; 
    ctrl_flag = 0; 
    enter_flag = 0;
    index = -1;
    terminal_table[0] = &terminal_open;
    terminal_table[1] = &read_terminal;
    terminal_table[2] = &write_terminal;
    terminal_table[3] = &terminal_close;
    enable_irq(1);
}

/* read_keyboard(uint8_t* buf, int32_t nbytes)
 * 
 * Reads terminal into buffer
 * Inputs: buf: pointer to input buffer
 *         nbytes: number of bytes to read into buffer
 * Outputs: Number of bytes read into buffer
 * Side Effects: Resets read_flag, enter_flag to 0, and index to -1
 */
int32_t read_terminal(int32_t buf_in, void* nbytes_in, int32_t arg3){
    // if(!buf)
    //     return -1;

    // while(1);
    uint8_t* buf = (uint8_t*)buf_in;
    int32_t nbytes = (int32_t)nbytes_in;
    
    read_flag = 1;
    ctrl_flag = 0;
    int32_t i= 0;

    while(enter_flag == 0){}; // shell sits here waiting until enter is hit
    
    for(i = 0; i < BUFSIZE; i++) {

        // if (i > index) {
        //     buf[i] = '\n';
        //     enter_flag = 0;
        //     read_flag = 0;
        //     index = -1;
        //     return i;
        // }
        if(i >= nbytes)
        {
            buf[nbytes - 1] = '\n';
            enter_flag = 0;
            read_flag = 0;
            index = -1;
            return nbytes;
        }

        buf[i] = buffer[i];
        if (buffer[i] == '\n') {
            enter_flag = 0;
            read_flag = 0;
            index = -1;
            return i+1;
        }
    }

    buf[BUFSIZE - 1] = '\n';
    enter_flag = 0;
    read_flag = 0;
    index = -1;
    return BUFSIZE;
}

/* write_keyboard(int32_t fd, const uint8_t* buf, int32_t nbytes)
 * 
 * Writes to terminal
 * Inputs: buf: pointer to buffer containing data to print
 *         nbytes: Number of bytes to write to terminal
 * Outputs: Number of bytes written to terminal
 * Side Effects: Resets read_flag, enter_flag to 0, and index to -1
 */
int32_t write_terminal(int32_t fd, void* buf_in, int32_t nbytes) {
    int32_t i = 0;
    const uint8_t* buf = (uint8_t*)buf_in;

    if(!buf)
        return -1;

    for (i = 0; i < nbytes; i++) {
        if(buf[i] == '\0')
            continue;
        putc(buf[i]);
    }
    return nbytes;
}

/*
 * terminal_open
 *   DESCRIPTION: Opens the terminal (placeholder function).
 *   INPUTS:
 *     - filename: Ignored (placeholder).
 *     - arg2: Ignored (placeholder).
 *     - arg3: Ignored (placeholder).
 *   OUTPUTS: None.
 *   RETURN VALUE: Always returns 0.
 *   SIDE EFFECTS: None.
 */

int32_t terminal_open (int32_t filename, void* arg2, int32_t arg3) {
    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: Closes the terminal (placeholder function).
 *   INPUTS:
 *     - fd: Ignored (placeholder).
 *     - arg2: Ignored (placeholder).
 *     - arg3: Ignored (placeholder).
 *   OUTPUTS: None.
 *   RETURN VALUE: Always returns 0.
 *   SIDE EFFECTS: None.
 */

int32_t terminal_close (int32_t fd, void* arrg2, int32_t arg3) {
return 0;
}


