#ifndef _SYSTEM_CALLS_H_
#define _SYSTEM_CALLS_H_

#include "types.h"
#include "filesystem.h"

#define MAX_FILES       8
#define ARGS_MAX        100

#define MAX_PROCESS     6
#define USR_ADDR        0x08000000
#define PROGRAM_ADDR    0x08048000
#define PMEM_START      0x8
#define EIP_ENTRY       24
#define _8MB            0x800000
#define _4MB            0x400000
#define _8KB            0x2000
#define RTC_TYPE        0
#define DIR_TYPE        1
#define REGULAR_TYPE    2

#define ELFMAG0		    0x7F
#define ELFMAG1		    0x45    //E
#define ELFMAG2		    0x4C    //L
#define ELFMAG3		    0x46    //F
#define PCB_SIZE        0x2000     // PCB size: 8kB
#define PCB_START       0x800000    // where PCB starts: 8MB
#define PCB_ADDR_MASK   0xFFFFE000  // bit mask to get the starting address of the current PCB


typedef struct file_operation_jump_table {
    int32_t (*open) (const uint8_t* filename);
    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close) (int32_t fd);
} file_ops;

typedef struct file_descriptor_table {
    file_ops* file_operation_ptr;
    int inode;
    int file_position;
    int flag;
} fd_table;

typedef struct process_control_block {
    fd_table fda[MAX_FILES];
    uint32_t process_ID;
    uint32_t parent_process_ID;
    uint32_t K_ESP_REG;
    uint32_t K_EBP_REG;
    uint32_t U_ESP_REG;
    uint32_t U_EBP_REG;
    uint32_t usr_eip;
    uint32_t usr_esp;
    uint32_t old_esp;   //esp of parent process
    uint32_t old_ebp;   //ebp of parent process
    uint32_t saved_esp;   //esp of current process
    uint32_t saved_ebp;   //ebp of current process
    uint8_t arg[FILENAME_LEN];
    uint32_t term_ID;
    /* more to be added... */
} PCB;

/* system calls */
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t get_args(uint8_t* buff, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);

/* system call helper functions */
void parse_argument(uint8_t* command, uint8_t* executable, uint8_t* argument);
PCB* get_pcb(uint32_t process_num);
PCB* get_curr_pcb();
int32_t failed_calls();

#endif
