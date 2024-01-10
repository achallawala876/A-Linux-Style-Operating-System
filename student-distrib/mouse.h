#ifndef _MOUSE_H
#define _MOUSE_H

#define KEYBOARD_DATA_PORT 0x60
#define  MOUSE_DATA_PORT 0x64
#define SCREEN_WIDTH 80
#define  SCREEN_HEIGHT 25


// char get_ascii(uint8_t key_in);
// int32_t read_terminal(int32_t buf_in, void* nbytes_in, int32_t arg3);
// int32_t write_terminal(int32_t fd, void* buf_in, int32_t nbytes);
// int32_t terminal_open (int32_t filename, void* arg2, int32_t arg3);
// int32_t terminal_close (int32_t fd, void* arrg2, int32_t arg3);
void mouse_init();
void padhle();
void likhle();
void likh_rahahu(uint8_t data);
uint8_t padh_rahahu();
int32_t khulja_simsim(int32_t* inode, char* filename);
int32_t bhag(int32_t* inode);
void mouse_intr_handler();



#endif
