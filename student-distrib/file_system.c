/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "init_idt.h"
#include "paging.h"
#include "file_system.h"
#include "multiboot.h"
#include "systemcalls.h"

// int* fileSystemPtr;
static boot_block_t* boot_block; // create global boot block for accessing directory data
static inode_t* inodePtr; // create inodePtr for loading inode struct array
static uint32_t* dataBlockPtr; // create dataBlockPtr for easy access to data blocks
// static uint32_t byte_offset = 0; // records offset read file calls (for a specific file)
static uint8_t* prev_file = NULL; // used or checking whether same file is being read again
static int dir_index = 0; // records offset for read directory calls
// static int idx = 0; // records offset for read directory calls

syscall_func_t file_table[4];
syscall_func_t dir_table[4];

/*
 * file_sys_init
 *   DESCRIPTION: Initializes the file system structures for further file operations.
 *   INPUTS: 
 *     - fileSystemPtr: A pointer to the start of the file system structure.
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Populates data structures for file system access.
 */
void file_sys_init(uint32_t* fileSystemPtr){
    
    int i;

    if (fileSystemPtr == NULL) return; // extend with error code for future checks

    // filling boot_block struct
    boot_block = (boot_block_t*)(fileSystemPtr);

    // increments of 1024 are used as the 32 but pointer moves 4 bytes a time
    // filling inode structs
    for (i = 1; i < 64; i++){
        inodePtr = (inode_t *) (fileSystemPtr + 1024*i); 
        inodes[i-1] = inodePtr; // mane tf is this 
    }

    // initialize dataBlockPtr for easy access to data blocks
    dataBlockPtr = fileSystemPtr + 1024 + (1024 * boot_block->inode_count);


    // Initialize file jump table
    file_table[0] = &file_open;
    file_table[1] = &read_data;
    file_table[2] = &file_write;
    file_table[3] = &file_close;

    dir_table[0] = &dir_open;
    dir_table[1] = &dir_read;
    dir_table[2] = &dir_write;
    dir_table[3] = &dir_close;
}

/*
 * file_open
 *   DESCRIPTION: Opens a file by name and populates the dentry structure with information about the file.
 *   INPUTS:
 *     - filename: A pointer to a null-terminated string representing the name of the file to be opened.
 *     - dentry: A pointer to a dentry_t structure where information about the file will be stored if found.
 *   OUTPUTS: None
 *   RETURN VALUE:
 *     - 0: If the file is found, and the dentry structure is populated with file information.
 *     - -1: If the file is not found, and the dentry structure remains empty.
 *   SIDE EFFECTS: Populates the dentry structure with file information if the file is found.
 */
int file_open(int32_t filename_in, void* dentry_in, int32_t arg3){
    const uint8_t *filename = (uint8_t*)filename_in;
    dentry_t* dentry = (dentry_t*)dentry_in;
    if (!read_dentry_by_name(filename, dentry)) // if file dentry is read it will give you a 0, so this is a 0 (success) in scope of file_open
        return 0;

    return -1;
}

/*
 * file_close
 *   DESCRIPTION: "Closes" the currently open file and resets related state variables.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 (indicating success)
 *   SIDE EFFECTS: Resets state variables used for file access.
 */
int file_close(int32_t arg1, void* arg2, int32_t arg3){
    // byte_offset = 0; 
    prev_file = NULL;
    return 0;
}

/*
 * file_write
 *   DESCRIPTION: Placeholder function to indicate that writing to a file is not supported.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: -1 (indicating that writing to a file is not supported)
 *   SIDE EFFECTS: None
 */
int file_write(int32_t arg1, void* arg2, int32_t arg3){
    return -1;
}

/*
 * file_read
 *   DESCRIPTION: Reads data from a file with the specified filename.
 *   INPUTS:
 *     - filename: A pointer to a null-terminated string representing the name of the file to read.
 *     - byte_count: The number of bytes to read from the file.
 *     - buf: A pointer to a buffer where the read data will be stored.
 *   OUTPUTS: Data read from the file is stored in the 'buf' parameter.
 *   RETURN VALUE:
 *     - The number of bytes actually read from the file (could be less than 'byte_count').
 *     - -1: If the file is not found, the filename is empty, or 'buf' is NULL.
 *   SIDE EFFECTS: 
 *     - Updates the 'byte_offset' to track the current position within the file.
 *     - Alters buffer that is passed in.
 */
int file_read(const uint8_t *filename, uint32_t byte_count, uint8_t *buf){

    int i; // loop index for directories
    int bytes_written = 0;
    // byte_offset = 0;
    if (!strlen((int8_t*)filename) || buf == NULL) return -1;

    // MIGHT BE USEFUL LATER:
    // int this_offset
    // if(prev_file == filename)
    //     this_offset = byte_offset;

    for (i = 0; i < boot_block->dir_count; i++){
        if (!strncmp((int8_t*)boot_block->direntries[i].filename, (int8_t*)filename, 32)){ // comparing 32 bytes in 2 character arrays
            int32_t inode_idx = boot_block->direntries[i].inode_num; // accessing inode index for this file
            bytes_written = read_data(inode_idx, buf, byte_count); // populate buffer with file data
            // byte_offset += bytes_written; // adding # of bytes to current global offset
            return bytes_written; 
        } 
    }

    return -1;
}

/*
 * dir_open
 *   DESCRIPTION: Opens a directory.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 (indicating success)
 *   SIDE EFFECTS: None
 */
int dir_open(int32_t arg1, void* arg2, int32_t arg3){ 
    return 0;
}

/*
 * dir_close
 *   DESCRIPTION: Closes a directory.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 (indicating success)
 *   SIDE EFFECTS: None
 */
int dir_close(int32_t arg1, void* arg2, int32_t arg3){
    return 0;
}

/*
 * dir_write
 *   DESCRIPTION: Placeholder function to indicate that writing to a directory is not supported.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: -1 (indicating that writing to a directory is not supported)
 *   SIDE EFFECTS: None
 */
int dir_write(int32_t arg1, void* arg2, int32_t arg3){
    return -1;
}

/*
 * dir_read
 *   DESCRIPTION: Reads directory entries and copies their names to the provided buffer.
 *   INPUTS:
 *     - str_buf: A pointer to a buffer where directory entry names will be stored.
 *   OUTPUTS: Directory entry names are copied into the 'str_buf' parameter.
 *   RETURN VALUE:
 *     - 0: If a directory entry name is successfully copied.
 *     - -1: If 'str_buf' is NULL.
 *   SIDE EFFECTS: Updates 'idx' and 'dir_index' to keep track of the current position within the directory.
 */
int dir_read(int32_t arg1, void* str_buf_in, int32_t arg2){
    uint8_t* str_buf = (uint8_t*)str_buf_in;
    int j;
    int8_t cur_file_name[32]; // 32 bytes needed

    if (str_buf == NULL) return -1;

    strncpy((int8_t*)cur_file_name, (int8_t*)boot_block->direntries[dir_index].filename, 32); // copying 32 bytes from filename in filesystem to our character array
    // cur_file_name[33] = '\0';
    // if (strlen((int8_t*)cur_file_name) > 32) {
    //     cur_file_name[32] = '\0';
    // }
    // IMP: PUTTING NULL IS BAD BAD BAD 
    for(j = 0; j < strlen((int8_t*)cur_file_name); j++){
        // str_buf[idx] = cur_file_name[j];
        // idx++;    
        if(j >= 32)
            break;
        str_buf[j] = cur_file_name[j];
    }

    str_buf[j] = '\n'; // mark the end of a name in the character array
    // idx++; 
    dir_index++;

    if(dir_index <= 17) {
        return j;
    }
    else {
        dir_index = 0;
        return 0;
    }
}

/*
 * read_data
 *   DESCRIPTION: Reads data from a file starting at a given offset and stores it in the buffer.
 *   INPUTS:
 *     - inode_idx: The index of the inode for the file to read.
 *     - byte_count: The number of bytes to read.
 *     - offset: The starting offset within the file.
 *     - buf: A pointer to the buffer where the read data will be stored.
 *   OUTPUTS: Data read from the file is stored in the 'buf' parameter.
 *   RETURN VALUE: The total number of bytes read and stored in 'buf'.
 *   SIDE EFFECTS: Alters buffer that is passed in.
 */
int32_t read_data(int32_t inode_idx,  void *buf_in, int32_t byte_count){
    uint8_t* buf = (uint8_t*)buf_in;
    int32_t data_block_actual_idx;
    uint8_t* cur_db_ptr;
    uint32_t i,j;
    uint32_t buf_idx = 0;
    uint32_t rem_byte_count;
    uint32_t block_end;
    uint32_t final_rem_bytes;
    uint32_t file_length;

    uint32_t offset = get_offset();

    // 4096 is the number of bytes in a single block of the filesystem data structure
    // Calculations: # of data blocks, etc. (centered around offset)
    uint32_t offset_block_num = offset/4096;  // first block num
    uint32_t start_byte_inside_block = offset % 4096;  // byte index inside data block
    uint32_t rem_bytes_inside_block = 4096 - start_byte_inside_block;
    uint32_t end;

    

    // Ensuring that buffer is valid
    if (buf == NULL) return -1;

    // Accessing file metadata
    file_length = inodes[inode_idx]->length;
    data_block_actual_idx = inodes[inode_idx]->data_block_num[offset_block_num];

    // Going to the data block from which we start readingd ata
    cur_db_ptr = (uint8_t*)(dataBlockPtr + (data_block_actual_idx * 1024)); // cur_db_ptr is 4 bytes, so it moves 4 bytes at a time... adding 1024 would lead to a 4096 byte block jump
    
    if(byte_count > rem_bytes_inside_block) // We to read data beyond one block
        end = 4096;
    else 
        end = start_byte_inside_block + byte_count; 

    // Populating buffer with data
    for(i = start_byte_inside_block; i < end; i++) {
        // if (buf_idx >= file_length)

        if (offset_block_num*4096 + i >= file_length)
        {
            return buf_idx;
        }
        buf[buf_idx] = cur_db_ptr[i];
        buf_idx++;
    }

    // Data reading done as we are done within the first block that was read
    if(end != 4096) {
        /* return now */
        return buf_idx;
    }

    // Remaining bytes
    rem_byte_count = byte_count - rem_bytes_inside_block;
    // New # of blocks left to be read
    block_end = rem_byte_count/4096;

    // Read data from what is remaining
    for (i = offset_block_num + 1; i < offset_block_num + 1 + block_end; i++) {
        data_block_actual_idx = inodes[inode_idx]->data_block_num[i];
        cur_db_ptr = (uint8_t*)(dataBlockPtr + (data_block_actual_idx*1024));

        for(j = 0; j < 4096; j++) {
            // if (buf_idx >= file_length)
            if (i*4096 + j >= file_length)
            {
                return buf_idx;
            }
            buf[buf_idx] = cur_db_ptr[j];
            buf_idx++;
        }

    }

    // Any data bytes left over (within a block)
    final_rem_bytes = rem_byte_count % 4096;
    if (final_rem_bytes == 0) {
        /* return now */
        return buf_idx;
    }

    data_block_actual_idx = inodes[inode_idx]->data_block_num[i];
    cur_db_ptr = (uint8_t *)(dataBlockPtr + (data_block_actual_idx*1024));
    for(j = 0; j < final_rem_bytes; j++) {
        if (i*4096 + j >= file_length)
        {
            return buf_idx;
        }
        buf[buf_idx] = cur_db_ptr[j];
        // buf[buf_idx] = 1;

        buf_idx++;
    }

    return buf_idx;

}

/*
 * read_dentry_by_name -> HELPER FUNCTION
 *   DESCRIPTION: Searches for a directory entry by name and populates the 'dentry' structure with the entry's information.
 *   INPUTS:
 *     - filename: A pointer to a null-terminated string representing the name of the directory entry to search for.
 *     - dentry: A pointer to a 'dentry_t' structure where the entry's information will be stored if found.
 *   OUTPUTS: Information about the directory entry is stored in the 'dentry' parameter if found.
 *   RETURN VALUE:
 *     - 0: If the directory entry is found, and 'dentry' is populated.
 *     - -1: If the directory entry is not found, or 'dentry' is NULL, or the filename is empty.
 *   SIDE EFFECTS: None
 */
int read_dentry_by_name(const uint8_t *filename, dentry_t* dentry){
    int i;

    if (dentry == NULL || !strlen((int8_t*)filename)) return -1; // invalid dentry or filename

    // iterating through all directories to find the specified filename
    for(i = 0; i < 63; i++){ 

        if(!strncmp((int8_t*)boot_block->direntries[i].filename, (int8_t*)filename, FILENAME_LEN)){
            // index constrained by this for loop and dentry NULL already done, so you can never have a -1 returned from this specific call
            read_dentry_by_index(i, dentry); 
            return 0;
        }
    }

    return -1;
}

/*
 * read_dentry_by_index -> HELPER FUNCTION
 *   DESCRIPTION: Reads a directory entry by its index and populates the 'dentry' structure with the entry's information.
 *   INPUTS:
 *     - dir_idx: The index of the directory entry to read.
 *     - dentry: A pointer to a 'dentry_t' structure where the entry's information will be stored.
 *   OUTPUTS: Information about the directory entry is stored in the 'dentry' parameter.
 *   RETURN VALUE:
 *     - 0: If the directory entry is found, and 'dentry' is populated.
 *     - -1: If 'dentry' is NULL, or the provided index is out of bounds.
 *   SIDE EFFECTS: None
 */
int read_dentry_by_index(int32_t dir_idx, dentry_t* dentry){

    if(boot_block->dir_count > 63 || boot_block->dir_count < 0 || dentry == NULL){ // to prevent breaking strncpy and memory
        return -1;
    } // directory index should be between 0 and 63

    // accessing file metadata through the directory index and filling in the attributes of the dentry struct passed in 
    strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->direntries[dir_idx].filename, 32); // copying 32 bytes from filesystem filename to the filename attribute of the dentry struct passed in
    dentry->filetype = boot_block->direntries[dir_idx].filetype;
    dentry->inode_num = boot_block->direntries[dir_idx].inode_num;

    return 0;

}


void set_dir_index(){
    dir_index = 0;
}

// OLDER IMPLEMENTATION - We abstracted the offset functionality away from the scope of the test cases.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// int file_read(const uint8_t *filename, uint32_t byte_count, uint32_t offset, uint8_t *buf){ // uses read_data, reads count bytes of data from file into buf
//     int i; // loop index for directories
//     if (!strlen((int8_t*)filename) || buf == NULL) return -1;

//     for (i = 0; i < boot_block->dir_count; i++){
//         if (!strncmp((int8_t*)boot_block->direntries[i].filename, (int8_t*)filename, 32)){
//             int32_t inode_idx = boot_block->direntries[i].inode_num;
//             byte_offset = read_data(inode_idx, byte_count, byte_offset, buf);  //changed offset to global_offset
//             return 0;
//         } 
//     }

//     return -1;
// }




