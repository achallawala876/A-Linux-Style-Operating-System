#ifndef _PAGING_H
#define _PAGING_H
// syntax for preventing multiple accesses of the data structures defined in this header file

#include "types.h"

// page directory entry struct - can be manipulated to either encode information for a 4 MB page or a page table
typedef struct page_directory_entry_t {
        uint32_t P          : 1;  
        uint32_t R_W        : 1;
        uint32_t U_S        : 1;
        uint32_t PWT        : 1;
        uint32_t PCD        : 1;
        uint32_t A          : 1;
        uint32_t AVL        : 1;
        uint32_t PS         : 1;
        uint32_t AVL_L      : 4;
        uint32_t base_level_addr : 20;
    } page_directory_entry;  

// page table entry struct
typedef struct page_table_t {
        uint32_t P          : 1;  
        uint32_t R_W        : 1;
        uint32_t U_S        : 1;
        uint32_t PWT        : 1;
        uint32_t PCD        : 1;
        uint32_t A          : 1;
        uint32_t D          : 1;
        uint32_t PAT        : 1;
        uint32_t G          : 1;
        uint32_t AVL_LT        : 3;
        uint32_t base_level_addr : 20; // HW expects these 20 bits at the top, so can't keep it at top of the struct
        } page_table;

page_directory_entry page_directory[1024] __attribute__((aligned(4096))); // array of 1024 entries for representing the page directory (4 KB aligned for memory)
page_table page_tables[1024] __attribute__((aligned(4096))); // array of 1024 page table entries, 4 MB aligned


page_table page_tables2[1024] __attribute__((aligned(4096))); // array of 1024 page table entries, 4 MB aligned


/*
 * loadPageDirectory
 *   DESCRIPTION: Loads a new page directory into the processor's control registers CR4, CR0, and CR3.
 *   INPUTS: page_directory - A pointer to the page directory to be loaded.
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Modifies control registers CR4, CR0, and CR3 to enable the new page directory.
 */
void loadPageDirectory(page_directory_entry *page_directory);

// description in paging_file.c
void page_init(); 


// union of two structs was discarded while debugging as manipulating a single struct was deemed more intuitive

// struct page_4MB { 
//         uint32_t P          : 1;  
//         uint32_t R_W        : 1;
//         uint32_t U_S        : 1;
//         uint32_t PWT        : 1;
//         uint32_t PCD        : 1;
//         uint32_t A          : 1;
//         uint32_t D          : 1;
//         uint32_t PS         : 1;
//         uint32_t G          : 1;
//         uint32_t AVL_LT     : 3;
//         uint32_t PAT        : 1;
//         uint32_t firstBits_level_addr : 8;
//         uint32_t RSVD        : 1;
//         uint32_t base_level_addr : 10;
//         } __attribute__ ((packed, aligned(4 * 1024 * 1024)));

int add_page_entry(int entry);
// void videoMap();
void remap_videoMap(int32_t new_terminal_pid);
void remap_fish(int32_t num);
#endif
