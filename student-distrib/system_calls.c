#include "paging.h"
#include "lib.h"
#include "system_calls.h"
#include "filesystem.h"
#include "terminal.h"
#include "RTC.h"
#include "x86_desc.h"
#include "scheduler.h"

/* global variables */
uint32_t pid_array[MAX_PROCESS] = {0, 0, 0, 0, 0, 0};   // initialize process array
uint32_t curr_pid;
uint32_t parent_pid;

/* file operation static tables */
static file_ops null_fop = {failed_calls, failed_calls, failed_calls, failed_calls};
static file_ops stdin_fop = {terminal_open, terminal_read, failed_calls, terminal_close};  // read
static file_ops stdout_fop = {terminal_open, failed_calls, terminal_write, terminal_close};    // write
static file_ops RTC_fop = {RTC_open, RTC_read, RTC_write, RTC_close};
static file_ops dir_fop = {dir_open, dir_read, dir_write, dir_close};
static file_ops file_fop = {file_open, file_read, file_write, file_close};

/*
 * int32_t halt (uint8_t status)
 * Description: halt system call
 * INPUT: status: process status
 * OUTPUT: none
 */
int32_t halt (uint8_t status) {
    int i;
    cli();
    /* -------------------------- Restore parent data -------------------------*/
    PCB* curr_pcb_ptr = get_curr_pcb();
    PCB* parent_pcb_ptr = get_pcb(curr_pcb_ptr->parent_process_ID);

    // return to shell if it is already shell
    if(curr_pcb_ptr->process_ID == 0){
        printf("Cannot exit base shell!\n");
        uint32_t eip_arg = curr_pcb_ptr->usr_eip;
        uint32_t esp_arg = curr_pcb_ptr->usr_esp;

        asm volatile ("\
            andl    $0x00FF, %%ebx  ;\
            movw    %%bx, %%ds      ;\
            pushl   %%ebx           ;\
            pushl   %%edx           ;\
            pushfl                  ;\
            popl    %%edx           ;\
            orl     $0x0200, %%edx  ;\
            pushl   %%edx           ;\
            pushl   %%ecx           ;\
            pushl   %%eax           ;\
            iret                    ;\
            "
            :
            : "a"(eip_arg), "b"(USER_DS), "c"(USER_CS), "d"(esp_arg)
            : "memory"
        );
    }
    // update pid
    curr_pid = curr_pcb_ptr->parent_process_ID;
    parent_pid = parent_pcb_ptr->parent_process_ID;
    pid_array[curr_pcb_ptr->process_ID] = 0;

    /* update terminal active process ID */
    terminals[curr_index].active_pid = curr_pid;

    /* -------------------------- Restore parent paging -------------------------*/
    int index = USR_ADDR >> 22; //shift 22 to get pde index
    set_pde_mb(index, 1);
    page_directory[index].MB.user_supervisor = 1;
    page_directory[index].MB.global = 0;
    page_directory[index].MB.table_address = 2 + curr_pid;   // skip first page

    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );

    /* -------------------------- Clear fd array -------------------------*/
    for(i = 0; i < MAX_FILES; i++){
        curr_pcb_ptr->fda[i].file_operation_ptr = &null_fop;
        curr_pcb_ptr->fda[i].inode = 0;
        curr_pcb_ptr->fda[i].file_position = 0;
        curr_pcb_ptr->fda[i].flag = 0;
    }

    /* -------------------------- Write Parent process' info back to TSS -------------------------*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = _8MB - (_8KB * parent_pcb_ptr->process_ID) - sizeof(int32_t);  // set to parent kernel stack

    uint16_t retval = (uint16_t) status;
    if(status == 0x0E) retval = 256;
    sti();
    /* -------------------------- Jump to execute return -------------------------*/
    // restore parent process's esp and ebp, then return
    asm volatile(
        "movl %0, %%esp     ;"
        "movl %1, %%ebp     ;"
        "xorl %%eax,%%eax   ;"
        "movw %2, %%ax      ;"
        "leave              ;"
        "ret                ;"
        : 
        : "r" (curr_pcb_ptr->old_esp), "r" (curr_pcb_ptr->old_ebp), "r"(retval)
        : "esp", "ebp", "eax"
    );
    return 0;
}

/*
 * int32_t execute (const uint8_t* command)
 * Description: system call execute
 * Input: command: command for system call
 * Output: 0 for success, -1 for failure
 */
int32_t execute (const uint8_t* command) {
    cli();
    int i;
    dentry_t temp_dentry;
    uint8_t elf[4]; //contains first four bytes of executable file

    uint32_t eip_arg;
    uint32_t esp_arg;

    /* -------------------------- Parse command and argument -------------------------*/
    uint8_t executable[FILENAME_LEN + 1];
    uint8_t argument[ARGS_MAX];
    parse_argument((uint8_t*)command, (uint8_t*)executable, (uint8_t*)argument);

    if(read_dentry_by_name((uint8_t*)executable, &temp_dentry) != 0){
        return -1;
    }

    read_data(temp_dentry.inode_num, 0, elf, 4);    //read first four bytes

    if(elf[0] != ELFMAG0 || elf[1] != ELFMAG1 || elf[2] != ELFMAG2 || elf[3] != ELFMAG3){
        return -1;  //check executable file
    }

    read_data(temp_dentry.inode_num, EIP_ENTRY, elf, 4);  // find the entry point for EIP from bytes 24-27 of the executable loaded

    /* -------------------------- Set up paging -------------------------*/
    int pid_flag = 0;
    for(i = 0; i < MAX_PROCESS; i++){
        if(pid_array[i] == 0){
            pid_array[i] = 1;
            curr_pid = i;
            pid_flag = 1;
            break;
        }
    }
    if(pid_flag == 0){
        printf("process full\n");
        return -1;
    }

    int index = USR_ADDR >> 22; //shift 22 to get pde index
    // int phys_mem_addr = USR_ADDR + curr_pid * _4MB; //start from 128mb, 4 mb each process

    uint32_t pt_index = 2 + curr_pid;  // skip first page
    // page_directory[USER_INDEX].table_addr_31_12 = phys_addr/ALIGN_4KB;//user virtual to physical

    set_pde_mb(index, 1);
    page_directory[index].MB.user_supervisor = 1;
    page_directory[index].MB.global = 0;
    page_directory[index].MB.table_address = pt_index;   //update page table index

    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );

    /* -------------------------- Load file -------------------------*/
    inode_t* temp_inode = (inode_t*)(inode_ptr + temp_dentry.inode_num);
    read_data(temp_dentry.inode_num, 0, (uint8_t*)PROGRAM_ADDR, temp_inode->length);  //copy file to program image address

    /* -------------------------- Create PCB -------------------------*/
    PCB* pcb_ptr = get_curr_pcb();
    pcb_ptr->process_ID = curr_pid;

    eip_arg = *((uint32_t*)elf);    // set EIP
    esp_arg = USR_ADDR + _4MB - sizeof(int32_t);  // 4 bits for data alignment

    pcb_ptr->usr_eip = eip_arg; //store eip and esp
    pcb_ptr->usr_esp = esp_arg;

    if(curr_pid != 0){
        pcb_ptr->parent_process_ID = parent_pid;
        parent_pid = curr_pid;
    }else{
        pcb_ptr->parent_process_ID = curr_pid;
    }

    if (terminals[curr_index].shell_on == 0) {
        terminals[curr_index].shell_on = 1;
        terminals[curr_index].active_pid = pcb_ptr->process_ID;
    }
    else {
        terminals[curr_index].active_pid = pcb_ptr->process_ID;
    }
    // curr_term()->active_pid = pcb_ptr->process_ID;
    //initialize fd array
    for(i = 0; i < MAX_FILES; i++){
        pcb_ptr->fda[i].file_operation_ptr = &null_fop;
        pcb_ptr->fda[i].inode = 0;
        pcb_ptr->fda[i].file_position = 0;
        pcb_ptr->fda[i].flag = 0;
    }

    // set up stdin and stdout
    pcb_ptr->fda[0].file_operation_ptr = &stdin_fop;
    pcb_ptr->fda[0].flag = 1;    // in use
    pcb_ptr->fda[1].file_operation_ptr = &stdout_fop;
    pcb_ptr->fda[1].flag = 1;    // in use

    // uint32_t curr_esp, curr_ebp;

    strncpy((int8_t*)pcb_ptr->arg, (int8_t*) argument, FILENAME_LEN);

    // save current process esp and ebp register
    asm volatile(
        "movl %%esp, %0 ;"
        "movl %%ebp, %1 ;"
        : "=r" (pcb_ptr->old_esp) ,"=r" (pcb_ptr->old_ebp) 
    );

    /* -------------------------- Contex switch -------------------------*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = _8MB - (_8KB * curr_pid) - sizeof(int32_t);
    /* -------------------------- Push IRET to stack -------------------------*/
    sti();
    asm volatile ("\
        andl    $0x00FF, %%ebx  ;\
        movw    %%bx, %%ds      ;\
        pushl   %%ebx           ;\
        pushl   %%edx           ;\
        pushfl                  ;\
        popl    %%edx           ;\
        orl     $0x0200, %%edx  ;\
        pushl   %%edx           ;\
        pushl   %%ecx           ;\
        pushl   %%eax           ;\
        iret                    ;\
        "
        :
        : "a"(eip_arg), "b"(USER_DS), "c"(USER_CS), "d"(esp_arg)
        : "memory"
    );
    
    return 0;
}

/*
 * int32_t read (int32_t fd, void* buf, int32_t nbytes)
 * Description: system call read
 * Input: fd: file descriptor
 *        buf: buffer used to write to terminal
 *        nbytes: number of bytes to write
 * Output: 0 for success, -1 for failure
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    sti();
    if(fd < 0 || fd > (MAX_FILES-1) || buf == NULL || nbytes <= 0){
        return -1;
    }
    PCB* curr_process = get_pcb(terminals[curr_index].active_pid);

    if (curr_process->fda[fd].flag == 0) {
        return -1;
    }
    fd_table file_to_read = curr_process->fda[fd];
    return file_to_read.file_operation_ptr->read(fd, buf, nbytes);
}

/*
 * int32_t write (int32_t fd, void* buf, int32_t nbytes)
 * Description: system call write
 * Input: fd: file descriptor
 *        buf: buffer used to write to terminal
 *        nbytes: number of bytes to write
 * Output: 0 for success, -1 for failure
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    if(fd < 0 || fd > (MAX_FILES-1) || buf == NULL || nbytes <= 0){
        return -1;
    }
    PCB* curr_process = get_pcb(terminals[curr_index].active_pid);

    if (curr_process->fda[fd].flag == 0) {
        return -1;
    }
    fd_table file_to_write = curr_process->fda[fd];
    return file_to_write.file_operation_ptr->write(fd, buf, nbytes);
}

/*
 * int32_t open (const uint8_t* filename)
 * Description: system call open
 * Input: filename: name of file to be opened
 * Output: 0 for success, -1 for failure
 */
int32_t open (const uint8_t* filename) {
    int fd_, i;
    int read_check;
    int files_full;
    dentry_t dentry_;
    read_check = read_dentry_by_name(filename, &dentry_);
    if (read_check == -1) {
        return -1;  // read failed
    }
    PCB* curr_process = get_pcb(terminals[curr_index].active_pid);
    // the first two fd entries are taken by stdin and stdout, so start from the third
    files_full = 1;     // assume all files opened
    for (i = 2; i < MAX_FILES; i++) {
        if (curr_process->fda[i].flag == 0) {
            curr_process->fda[i].flag = 1;
            curr_process->fda[i].file_position = 0;
            fd_ = i;
            files_full = 0;     // not full
            break;
        }
    }
    if (files_full) {
        return -1;
    }
    /* set up fop table according to dentry */
    switch (dentry_.file_type)
    {
    case RTC_TYPE:
        /* check if we can open the file */
        if (RTC_open(filename) == -1) {
            return -1;
        }
        curr_process->fda[fd_].file_operation_ptr = &RTC_fop;
        curr_process->fda[fd_].inode = 0;   // 0 for directories and RTC
        break;
    case DIR_TYPE:
        /* check if we can open the file */
        if (dir_open(filename) == -1) {
            return -1;
        }
        curr_process->fda[fd_].file_operation_ptr = &dir_fop;
        curr_process->fda[fd_].inode = 0;   // 0 for directories and RTC
        break;
    case REGULAR_TYPE:
        /* check if we can open the file */
        if (file_open(filename) == -1) {
            return -1;
        }
        curr_process->fda[fd_].file_operation_ptr = &file_fop;
        curr_process->fda[fd_].inode = dentry_.inode_num;
        break;
    default:
        break;
    }
    return fd_;
}

/*
 * int32_t close (int32_t fd)
 * Description: system call close
 * Input: fd: file descriptor number
 * Output: 0 for success, -1 for failure
 */
int32_t close (int32_t fd) {
    int check_close;
    if(fd < 2 || fd > (MAX_FILES-1)){
        return -1;
    }
    PCB* curr_process = get_pcb(terminals[curr_index].active_pid);

    if (curr_process->fda[fd].flag == 0) {
        return -1;
    }
    curr_process->fda[fd].flag = 0;     // mark as available
    check_close = curr_process->fda[fd].file_operation_ptr->close(fd);
    return check_close;
}

/*
 * int32_t getargs(uint8_t* buf, int32_t nbytes)
 * Description: system call getargs
 * Input: buf: file descriptor number
 *        nbytes: bytes to read
 * Output: 0 for success, -1 for failure
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
    if(buf == NULL){
        return -1;
    }
    PCB* cur_pcb = get_pcb(terminals[curr_index].active_pid);
    if(cur_pcb->arg[0] == NULL){
        return -1;
    }
    strncpy((int8_t*)buf, (int8_t*)(cur_pcb->arg), nbytes);
    return 0;
}

/*
 * int32_t vidmap(uint8_t ** screen_start)
 * Description: Map video map page directory 
 * Input: screen_start: screen starting address
 * Output: 0 for success, -1 for failure
 */
int32_t vidmap(uint8_t** screen_start){
    if(screen_start < (uint8_t**)USR_ADDR || screen_start > (uint8_t**)(USR_ADDR + _4MB)) {
        return -1;
    }

    set_pte_video_mem(VIDEO_ADDR >> 12, 1);

    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );
    
    *screen_start = (uint8_t*)(USR_ADDR + _4MB + VIDEO_ADDR);
    return 0;
}

/* system call helper functions */

/*
 * void parse_argument(uint8_t* command, uint8_t* executable, uint8_t* argument)
 * Description: parse executable and argument from command
 * Input: command: command to execute
 *        executable: executable files
 *        argument: argument files
 * Output: none
 */
void parse_argument(uint8_t* command, uint8_t* executable, uint8_t* argument){
    int i = 0;
    int j = 0;

    //copy executable name
    while(command[i] == ' '){
        i++;    //skip leading space
    }
    while(command[i] != ' ' && command[i] != '\0' && i < FILENAME_LEN){
        executable[j] = command[i];
        j++;
        i++;
    }
    executable[j] = '\0';   //set null terninator

    //copy arguments
    while(command[i] == ' '){
        i++;    //skip space between excutable and argument
    }
    j = 0;
    while(command[i] != ' ' && command[i] != '\0' && i < ARGS_MAX && j < FILENAME_LEN){
        argument[j] = command[i];
        j++;
        i++;
    }
    argument[j] = '\0'; //set null terminator
}

/*
 * PCB* get_pcb (uint32_t process_num)
 * Description: get PCB based on process nuumber
 * Input: process_num
 * Output: corresponding pcb pointer
 */
PCB* get_pcb(uint32_t process_num) {
    return (PCB* ) (PCB_START-(process_num+1)*PCB_SIZE);
}

/*
 * PCB* get_curr_pcb()
 * Description: get current PCB
 * Input: none
 * Output: current pcb pointer
 */
PCB* get_curr_pcb() {
    return (PCB* ) (PCB_START-(curr_pid+1)*PCB_SIZE);
}

/*
 * failed_calls()
 * Description: used for fops
 * Input: none
 * Output: none
 */
int32_t failed_calls() {
    return -1;
}
