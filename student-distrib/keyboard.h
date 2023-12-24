#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define BUFSIZE 100
#define bool uint8_t
#define true 1
#define false 0

// extern int select_t1;
// extern int select_t2;
// extern int select_t3;

typedef struct terminal_info {
    char terminal_buffer[BUFSIZE];
    int terminal_index;
    int terminal_caps_flag; // 0 = not active, 1 = active
    int terminal_shift_flag; // 0 = not active, 1 = active
    int terminal_read_flag; // 0 = not active, 1 = active
    int terminal_ctrl_flag; // 0 = not active, 1 = active
    int terminal_enter_flag; // 0 = not active, 1 = active
    int terminal_alt_flag; // 0 = not active, 1 = active
} terminal_info_t;

//////////////////////////////////////////////////////////// for 2048 ////////////////////////////////////////////////////////////

#define KEY_NULL 0
#define KEY_ESC 27
#define KEY_BACKSPACE '\b'
#define KEY_TAB '\t'
#define KEY_ENTER '\n'
#define KEY_RETURN '\r'

#define KEY_INSERT 0x90
#define KEY_DELETE 0x91
#define KEY_HOME 0x92
#define KEY_END 0x93
#define KEY_PAGE_UP 0x94
#define KEY_PAGE_DOWN 0x95
#define KEY_LEFT 0x4B
#define KEY_UP 0x48
#define KEY_RIGHT 0x4D
#define KEY_DOWN 0x50

#define KEY_F1 0x80
#define KEY_F2 (KEY_F1 + 1)
#define KEY_F3 (KEY_F1 + 2)
#define KEY_F4 (KEY_F1 + 3)
#define KEY_F5 (KEY_F1 + 4)
#define KEY_F6 (KEY_F1 + 5)
#define KEY_F7 (KEY_F1 + 6)
#define KEY_F8 (KEY_F1 + 7)
#define KEY_F9 (KEY_F1 + 8)
#define KEY_F10 (KEY_F1 + 9)
#define KEY_F11 (KEY_F1 + 10)
#define KEY_F12 (KEY_F1 + 11)

typedef struct keyboard {
  bool keys[128];
  bool chars[128];
}key;
key keyboard;


///////////////////////////////////////////////////////////////////////exit/////////////////////////////////////////////////////

void store_cur_buffer (int terminal_num);
void select_terminal_buffer(int terminal_num);

void store_enter_flag(int terminal_num);
void load_enter_flag(int terminal_num);


char get_ascii(uint8_t key_in);
int32_t read_terminal(int32_t buf_in, void* nbytes_in, int32_t arg3);
int32_t write_terminal(int32_t fd, void* buf_in, int32_t nbytes);
int32_t terminal_open (int32_t filename, void* arg2, int32_t arg3);
int32_t terminal_close (int32_t fd, void* arrg2, int32_t arg3);
void keyboard_init();

extern char buffer[BUFSIZE];


#endif
