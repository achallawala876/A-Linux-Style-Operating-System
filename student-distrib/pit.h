#ifndef pit
#define pit

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "init_idt.h"



extern int ticks;
void pit_init();
void pit_intr_handler();
uint32_t pit_get_ticks();

#endif
