#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "init_idt.h"
// #include <math.h>
// #include <stdio.h>

#define rtc_max 512
#define rtc_min 2

#ifndef rtc
#define rtc

// typedef struct terminal_rtc {
//     uint32_t count;
//     uint8_t read;
//     volatile uint32_t rtc_counter;

// } terminal_rtc_t;
void rtc_init();
void rtc_change_rate(int32_t frequency);
int conv(int32_t frequency);
// void store_cur_rtc(int terminal_num);
// void read_cur_rtc(int terminal_num);
int32_t rtc_write (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_open (int32_t filename, void* arg2, int32_t arg3);
int32_t rtc_close (int32_t fd, void* arg2, int32_t arg3);
void rtc_intr_handler();

#endif
