#include "lib.h"
#include "i8259.h"
#include "mouse.h"
#include "systemcalls.h"

// char buffer[BUFSIZE];
// static int index = -1; // print buffer till this index, if < 0 --> bufer empty
// static int caps_flag = 0; // 0 = not active, 1 = active
// static int shift_flag = 0; // 0 = not active, 1 = active
// static int read_flag = 0; // 0 = not active, 1 = active
// static int ctrl_flag = 0; // 0 = not active, 1 = active
// static int enter_flag = 0; // 0 = not active, 1 = active
int x=0,y=0,l=0,r=0,m=0;
int32_t move_x;
int32_t move_y;
int mouse_used=0;
int checker;
syscall_func_t terminal_table[4];


/* keyboard_init()
 * 
 * Iniatilizes keyboard device
 * Inputs: None
 * Outputs: None
 * Side Effects: Enable keyboard interrupt (1) on PIC
 */
void mouse_init(){
    // x = 0; 
    // y = 0;
    move_x=0;
    move_y=0;
    checker='E';
    padhle();
    outb(0xa8,MOUSE_DATA_PORT);
    padhle();
    outb(0x20,MOUSE_DATA_PORT);
    
    likhle();
    uint8_t status = inb(KEYBOARD_DATA_PORT);
    status |= 2;
    status &= 0xdf;
    padhle();
    outb(0x60,MOUSE_DATA_PORT);
    padhle();
    // printf("%d",move_x);
    outb(status,KEYBOARD_DATA_PORT);

    likh_rahahu(0xf6);
    padh_rahahu();
    // printf("%d",move_x);
    likh_rahahu(0xf4);
    padh_rahahu();
    // printf("%d",move_x);
    padhle();
    likh_rahahu(0xf3);
    padh_rahahu();
    // likh_rahahu(100);
    // padh_rahahu();
    outb(200, KEYBOARD_DATA_PORT);

    enable_irq(12);
}




void mouse_intr_handler(){
    // printf("mouseasssssssssssssssssssss ma");
    cli();
    uint8_t read1=inb(KEYBOARD_DATA_PORT);
    int8_t x_move=inb(KEYBOARD_DATA_PORT);
    int8_t y_move=inb(KEYBOARD_DATA_PORT);
    uint8_t fixed;
    if ((read1 & 0x80) || (read1 & 0x40) || !(read1 & 0x8)){
        send_eoi(12);
        sti();
        return;
    };
    if (read1 & 16){x_move = 0xFFFFFF00 | x_move;}
    if (read1 & 32){y_move = 0xFFFFFF00 | y_move;}
    if (read1 & 4){update_colors();}
    if (checker!='+')
        {fixed=checker;}
    int old_x=move_x;
    int old_y=move_y;
    move_x=move_x+(x_move>>3);
    move_y=move_y-(y_move>>3);


    if(move_x < 0)move_x = 0;
    if(move_y < 0)move_y = 0;
    if(move_x > SCREEN_WIDTH-1)move_x = SCREEN_WIDTH-1;
    if(move_y > SCREEN_HEIGHT-1 )move_y = SCREEN_HEIGHT-1;

    // if (read1 & 16)
    checker=read_char(move_y,move_x);
    clear_mouse(old_y,old_x);
    update_mouse(move_y,move_x);
    if (fixed != ' ')
        update_char(fixed,old_y,old_x);
    // printf("%c \n",checker);

    sti();
    send_eoi(12);
}




void padhle() {
    int i = 100000;
    while(i-- && inb(MOUSE_DATA_PORT) & 2);
}

void likhle() {
    int i = 100;
    while(i-- && inb(MOUSE_DATA_PORT) & 1);
}


void likh_rahahu(uint8_t data){
    padhle();
    outb(0xd4,MOUSE_DATA_PORT);
    padhle();
    outb(data,KEYBOARD_DATA_PORT);
}

uint8_t padh_rahahu(){
    likhle();
    return inb(KEYBOARD_DATA_PORT);
}


int32_t khulja_simsim(int32_t* inode, char* filename) {
    cli();
    if(mouse_used) {
        sti();
        return 0;
    }
    mouse_used = 1;
    x = 0;
    y = 0;
    l = 0;
    r = 0;
    m=0;
    move_x = 0;
    move_y = 0;
    sti();
    return 1;
}


int32_t close(int32_t* inode) {
    mouse_used = 0;
    return 0;
}




