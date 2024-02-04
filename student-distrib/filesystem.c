#include "filesystem.h"
#include "system_calls.h"

/* init_filesystem
 * Description: Initialize the file system pointers
 * Input: base_address: file system address in memory
 * Output: none
 * Side effect: initialize the file_sys_addr and where boot_block starts
*/
void init_filesystem(unsigned int base_address){
    boot_block_ptr = (boot_block_t*)(base_address);
    dentry_ptr = boot_block_ptr->direntries;
    inode_ptr = (inode_t*)(boot_block_ptr + 1);
    inode_count = boot_block_ptr->inode_count;  //total inode count
    block_ptr = (uint8_t*)(inode_ptr + inode_count);
}

/* read_dentry_by_name
 * Description: load the coresponding file's name, type, inode into dentry
 * Input: file_name: name of the file
 *        dentry: dentry to copy data into
 * Output: 0 for success, -1 for fail
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){

    int fname_len = strlen((int8_t*) fname);
    int dname_len;
    int i;
    for(i = 0; i < boot_block_ptr->dir_count; i++){    //traverse dentries to find matched file name
        dname_len = strlen((int8_t*)dentry_ptr[i].filename);

        if(dname_len > FILENAME_LEN){   //check file name length
            dname_len = FILENAME_LEN;
        }

        if(dname_len == fname_len){ //if file name matched, copy to the dentry 
            if(strncmp((int8_t*)fname,(int8_t*) dentry_ptr[i].filename, FILENAME_LEN) == 0){
                read_dentry_by_index(i, dentry);
                return 0;
            }
        }
    }
    return -1;
}

/* read_dentry_by_index
 * Description: copy the target dentry into input dentry
 * Input: index: index of the dentry to copy from
 *        dentry: dentry to copy data into
 * Output: 0 for sucess, -1 for fail
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if(index < 64 - 1) {
        *dentry = boot_block_ptr->direntries[index];
        return 0;
    }
    return -1;
}

/* read_data
 * Description: get the offset from inode, and write buff length into buf
 * Input: inode: inode number that points to inode block
 *        offset: offset from beginnging to start reading  
 *        buf: buffer to copy data into
 *        length: the length of data to read
 * Output: bytes_read
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t block_num;
    uint8_t* curr_block_ptr;
    uint32_t bytes_read = 0;
    uint32_t block_index = offset / BLOCK_SIZE;
    uint32_t block_offset = offset % BLOCK_SIZE;
    inode_t* curr_inode_ptr = (inode_t*)(inode_ptr + inode);
    uint32_t N = boot_block_ptr->inode_count;

    if(inode >= N || inode < 0){
        return -1;  //inalid inode
    }

    while ((bytes_read < length && bytes_read + offset < curr_inode_ptr->length))
    {
        block_num = curr_inode_ptr->data_block_num[block_index];    //get corresponding block number
        curr_block_ptr = (uint8_t*)(block_ptr + BLOCK_SIZE * block_num + block_offset);
        uint32_t bytes_to_copy = BLOCK_SIZE - block_offset; // bytes remaining in this block

        //do not read more than requested
        if(bytes_to_copy > length - bytes_read){
            bytes_to_copy = length - bytes_read;
        }
        if(bytes_to_copy > curr_inode_ptr->length - offset - bytes_read){
            bytes_to_copy = curr_inode_ptr->length - offset - bytes_read;
        }
        memcpy(buf + bytes_read, curr_block_ptr, bytes_to_copy);
        bytes_read += bytes_to_copy;
        block_index++;
        block_offset = 0;
    }
    
    return bytes_read;
}

/* file_open
 * Description: open the file and set up file descriptor (only for check point 2)
 * Input: fname: name of file to open
 * Output: 0 for sucess, -1 for fail
*/
int32_t file_open(const uint8_t* fname){
    /*Invalid file name if length > 32*/
	// if(strlen((int8_t*)fname) > FILENAME_LEN){
    //     return -1;
    // }
    uint8_t truncated_fname[FILENAME_LEN + 1];
    strncpy((int8_t*)truncated_fname, (int8_t*)fname, FILENAME_LEN);
    truncated_fname[FILENAME_LEN] = '\0';

    if(read_dentry_by_name (truncated_fname, &curr_dentry) || curr_dentry.file_type != 2){
        return -1;
    }
	
    printf("file name:%s",  curr_dentry.filename);
    printf(" file type:%d\n",  curr_dentry.file_type);
	return 0; 
}

/* file_close
 * Description: close the file 
 * Input: fd
 * Output: 0
*/
int32_t file_close(int32_t fd){
    return 0;
}

/* file_write
 * Description: write file
 * Input: fd 
 *        buf 
 *        nbytes 
 * Output: -1
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* file_read
 * Description: read the content of the file
 * Input: fd: index of file to read in file descriptor array
 *        buf: buffer to read data into
 *        nbytes: number of bytes to read
 * Output: number of bytes read if success, -1 if failed
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    int32_t bytes_read;

    if(buf == NULL){
        return -1;
    }
    
    PCB* pcb_ptr = get_curr_pcb();

    bytes_read = read_data(pcb_ptr->fda[fd].inode, pcb_ptr->fda[fd].file_position, (uint8_t*)buf, nbytes);
    
    if(bytes_read == -1){
        return -1;
    }else{
        pcb_ptr->fda[fd].file_position += bytes_read;
    }

    return bytes_read;
}

/* dir_open
 * Description: open directory
 * Input: filename
 * Output: 0
*/
int32_t dir_open (const uint8_t* filename){
    return 0;
}

/* dir_close
 * Description: close directory
 * Input: fd
 * Output: 0
*/
int32_t dir_close (int32_t fd){
    return 0;
}

/* dir_write
 * Description: write to directory 
 * Input: fd
 * 		  buf
 * 		  nbytes
 * Output: -1
*/
int32_t dir_write (int32_t fd,const void* buf, int32_t nbytes){
    return -1;
}

/* dir_read
 * Description: read from directory
 * Input: fd
 * 		  buf
 * 		  nbytes
 * Output: 0
 * Size effect: will print a list of file name, type, and size in the directory
*/
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes){
    int i;
    dentry_t dt;
    char temp_filename[FILENAME_LEN + 1]; // +1 for the null terminator
    PCB* curr_pcb = get_curr_pcb();
    int32_t dir_location = curr_pcb->fda[fd].file_position; // get the position of the starting file

    if(read_dentry_by_index(dir_location, &dt) == -1){  // check for invalid read
        return 0;
    }

    for (i = 0; i < FILENAME_LEN; i ++){
        temp_filename[i] = dt.filename[i];
    }

    temp_filename[FILENAME_LEN] = '\0';

    uint32_t filename_len = strlen((int8_t*)(temp_filename));

    strncpy((int8_t*)buf, (int8_t*)temp_filename, filename_len);

    dir_location ++;
    curr_pcb->fda[fd].file_position = dir_location; // update file position to the next file

    return filename_len;
}
