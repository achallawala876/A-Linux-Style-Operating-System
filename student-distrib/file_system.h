#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#define FILENAME_LEN 32
#define INODE_SIZE 63
#define NUM_DATA_BLOCKS 1023

// STRUCT DENTRY
typedef struct dentry { 
    uint8_t filename[FILENAME_LEN];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[24]; // 24 reserved bytes
} dentry_t;

// STRUCT BOOT BLOCK
typedef struct boot_block {
   uint32_t dir_count;
   uint32_t inode_count;
   uint32_t data_count;
   uint8_t reserved[52]; // 52 reserved bytes
   dentry_t direntries[63];
} boot_block_t;

// STRUCT INODE
typedef struct inode { 
    uint32_t length;
    uint32_t data_block_num[NUM_DATA_BLOCKS];
} inode_t;

// create array of inode structs for easy access to inode blocks
inode_t* inodes [INODE_SIZE]; 

// function prototypes, described in file_system.c
void file_sys_init(uint32_t* fileSystemPtr);
int file_open(int32_t filename_in, void* dentry_in, int32_t arg3);
int file_close(int32_t arg1, void* arg2, int32_t arg3);
int file_write(int32_t arg1, void* arg2, int32_t arg3);
int file_read(const uint8_t *filename, uint32_t byte_count, uint8_t *buf);

int dir_open(int32_t arg1, void* arg2, int32_t arg3);
int dir_close(int32_t arg1, void* arg2, int32_t arg3);
int dir_write(int32_t arg1, void* arg2, int32_t arg3);
int dir_read(int32_t arg1, void* str_buf_in, int32_t arg2);

int32_t read_data(int32_t inode_idx,  void *buf, int32_t byte_count);
int read_dentry_by_name(const uint8_t *filename, dentry_t* dentry);
int read_dentry_by_index(int32_t dir_idx, dentry_t* dentry);
// int file_read(const uint8_t *filename, uint32_t byte_count, uint32_t offset, uint8_t *buf);
void set_dir_index();


#endif
