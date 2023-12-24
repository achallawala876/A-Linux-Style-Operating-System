#include "paging.h"
#include "systemcalls.h"


// static page_directory_entry page_directory[1024] __attribute__((aligned(4096)));

/*
 * page_init
 *   DESCRIPTION: Initializes the page directory and page tables for memory management.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Initializes page directory and page tables.
 */
 
void page_init(){
    int i; 

    // TOP 10 bits from VA into PD

    // setting page directory index 0 for a page table of 1024 entries
    page_directory[0].P = 1; // tells us that an entry is present   
    page_directory[0].R_W = 1; // sets to rw mode
    page_directory[0].U_S = 0;
    page_directory[0].PWT = 0;
    page_directory[0].PCD = 0;
    page_directory[0].A   = 0;
    page_directory[0].AVL = 0;
    page_directory[0].PS  = 0; // indicates that this page directory index contains information that allows us to access page table indices
    page_directory[0].AVL_L = 0;
    page_directory[0].base_level_addr = ((unsigned int)page_tables >> 12); // we want the upper 20 bits to set the base level address of our 4 KB page

    // setting page directory index 1 for 4 MB kernel page
    page_directory[1].P = 1; // tells us that an entry is present  
    page_directory[1].R_W  = 1; // sets to rw mode
    page_directory[1].U_S  = 0;
    page_directory[1].PWT  = 0;
    page_directory[1].PCD  = 0;
    page_directory[1].A    = 0;
    page_directory[1].AVL  = 0;
    page_directory[1].PS   = 1; // indicates that this page directory index contains information that allows us to access the kernel page
    page_directory[1].AVL_L = 1;
    page_directory[1].base_level_addr = 1024;// this is done because this is hex 400, and the TLB appends three hex 0s to the end of it so you get x400 000 which is 4 MB

    // 400 00000
    // 100 0000 000/22 
    // which byte in the jumbo page through 22 bits
    
    // this loop is for the 4 KB pages in the page table (including video memory addressing)
    for(i = 0; i < 1024; i++){

        page_tables[i].P        = 0;
        page_tables[i].R_W      = 1;
        page_tables[i].U_S      = 0;
        page_tables[i].PWT      = 0;
        page_tables[i].PCD      = 0;
        page_tables[i].A        = 0;
        page_tables[i].D        = 0;
        page_tables[i].PAT      = 0;
        page_tables[i].G        = 0;
        page_tables[i].AVL_LT   = 0;
        page_tables[i].base_level_addr = i; // x01000 is 4096, x2000 is 8192, etc... remember the appending and the alignment stuff

        // 12 0s below

        // at index 184, the video memory begins (xb8000/4096)
        if(i <= 187 && i >= 184){ // accounting for video memory of multiple terminals
                
            page_tables[i].P        = 1; // set P to 1 for indicating video memory
            page_tables[i].R_W      = 1;
            page_tables[i].U_S      = 0;
            page_tables[i].PWT      = 0;
            page_tables[i].PCD      = 0;
            page_tables[i].A        = 0;
            page_tables[i].D        = 0;
            page_tables[i].PAT      = 0;
            page_tables[i].G        = 0;
            page_tables[i].AVL_LT      = 0;
            page_tables[i].base_level_addr = i;

        }
            
    }


    page_directory[34].P = 1; // tells us that an entry is present   
    page_directory[34].R_W = 1; // sets to rw mode
    page_directory[34].U_S = 1;
    page_directory[34].PWT = 0;
    page_directory[34].PCD = 0;
    page_directory[34].A   = 0;
    page_directory[34].AVL = 0;
    page_directory[34].PS  = 0; // indicates that this page directory index contains information that allows us to access page table indices
    page_directory[34].AVL_L = 0;
    page_directory[34].base_level_addr = ((unsigned int)page_tables2 >> 12);


    page_tables2[0].P        = 1; // set P to 1 for indicating video memory
    page_tables2[0].R_W      = 1;
    page_tables2[0].U_S      = 1;
    page_tables2[0].PWT      = 0;
    page_tables2[0].PCD      = 0;
    page_tables2[0].A        = 0;
    page_tables2[0].D        = 0;
    page_tables2[0].PAT      = 0;
    page_tables2[0].G        = 0;
    page_tables2[0].AVL_LT      = 0;
    page_tables2[0].base_level_addr = 184;

    loadPageDirectory(page_directory); // call assembly function that does all 3 steps in 1 go: enabling page directory, loading 4MiB kernel page, and loading page table

}

/* TODO:
*
* Fix paging: create union for 4mb and pt
*/

int add_page_entry(int entry) {

    if (entry > 5) 
        return -1; // failure

    page_directory[32].P = 1; // tells us that an entry is present  
    page_directory[32].R_W  = 1; // sets to rw  mode    
    page_directory[32].U_S  = 1; // set to user mode
    page_directory[32].PWT  = 0; // caching enabled
    page_directory[32].PCD  = 0;
    page_directory[32].A    = 0;
    page_directory[32].AVL  = 0;
    page_directory[32].PS   = 1; // indicates that this page directory index contains information that allows us to access the shell page
    page_directory[32].AVL_L = 1;
    page_directory[32].base_level_addr = 1024 * (entry + 2);

    flush_tlb();
    
    /* Success */
    return 0;

}


/*
 * remap_videoMap
 *   DESCRIPTION: Remaps video memory in the page table for the specified terminal.
 *   INPUTS:
 *     - new_terminal_pid: Process ID or terminal identifier for the new terminal.
 *   OUTPUTS: Updates the page table entry for video memory.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies page table entries and flushes the Translation Lookaside Buffer (TLB).
 */

void remap_videoMap(int32_t new_terminal_pid) {
    page_tables[184].P        = 1; // set P to 1 for indicating video memory
    page_tables[184].R_W      = 1;
    page_tables[184].U_S      = 0;
    page_tables[184].PWT      = 0;
    page_tables[184].PCD      = 0;
    page_tables[184].A        = 0;
    page_tables[184].D        = 0;
    page_tables[184].PAT      = 0;
    page_tables[184].G        = 0;
    page_tables[184].AVL_LT      = 0;
    page_tables[184].base_level_addr = 185 + new_terminal_pid;

    flush_tlb();
}


/*
 * remap_fish
 *   DESCRIPTION: Remaps video memory for the specified terminal in the page table.
 *   INPUTS:
 *     - num: Terminal number for which to remap video memory.
 *   OUTPUTS: Updates the page table entry for video memory.
 *   RETURN VALUE: None.
 *   SIDE EFFECTS: Modifies page table entries and flushes the Translation Lookaside Buffer (TLB).
 */

void remap_fish(int32_t num){

    page_tables2[0].P        = 1; // set P to 1 for indicating video memory
    page_tables2[0].R_W      = 1;
    page_tables2[0].U_S      = 1;
    page_tables2[0].PWT      = 0;
    page_tables2[0].PCD      = 0;
    page_tables2[0].A        = 0;
    page_tables2[0].D        = 0;
    page_tables2[0].PAT      = 0;
    page_tables2[0].G        = 0;
    page_tables2[0].AVL_LT      = 0;
    page_tables2[0].base_level_addr = 185 + num;


    flush_tlb();


}
