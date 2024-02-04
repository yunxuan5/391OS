#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "lib.h"

#define FILENAME_LEN            32
#define DENTRY_RESERVED         24
#define BOOT_BLOCK_RESERVED     52
#define DENTRY_NUM              64 
#define BLOCK_SIZE              4096
#define DATA_BLOCK_NUM          1023

typedef struct dentry{
    int8_t filename[FILENAME_LEN];
    int32_t file_type;
    int32_t inode_num;
    int8_t reserved[DENTRY_RESERVED];
}dentry_t;

typedef struct boot_block
{
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[BOOT_BLOCK_RESERVED];
    dentry_t direntries[DENTRY_NUM - 1];
}boot_block_t;

typedef struct inode {
    int32_t length;
    int32_t data_block_num [DATA_BLOCK_NUM];
} inode_t;

dentry_t* dentry_ptr;
boot_block_t* boot_block_ptr;
inode_t* inode_ptr;
uint8_t* block_ptr;
int32_t inode_count;
dentry_t curr_dentry;

extern void init_filesystem(unsigned int base_address);

extern int32_t file_open(const uint8_t* fname);

extern int32_t file_close(int32_t fd);

extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t dir_open (const uint8_t* filename);

extern int32_t dir_close (int32_t fd);

extern int32_t dir_write (int32_t fd,const void* buf, int32_t nbytes);

extern int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
