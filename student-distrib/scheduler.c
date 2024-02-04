#include "scheduler.h"
#include "x86_desc.h"
#include "terminal.h"
#include "paging.h"

volatile int32_t curr_index = 0;
/*
 * scheduler()
 *   Description: do the process switch
 *   Inputs: none
 *   Outputs: none
 *   Side effects: saves current kernel stack, remap video page, restore TSS, and load the scheduled kernel stack 
 */
void scheduler() {
    /* create a new page for the next proecess */
    PCB* curr_PCB;
    PCB* next_PCB;
    /* Get the current active PCB */
    curr_PCB = get_pcb(terminals[curr_index].active_pid); // need to be the current active process at the terminals that we switching into
    // save current kernel stack 
    asm volatile(
        "movl %%esp, %0 ;"
        "movl %%ebp, %1 ;"
        : "=r" (curr_PCB->saved_esp) ,"=r" (curr_PCB->saved_ebp) 
    );
    /* if shell has not been opened, open shell first*/
        /* remap the video memory */
        vidmap_switch(curr_index);
    if (terminals[curr_index].shell_on == 0) {
        execute((const uint8_t*)"shell");
    }
    /* terminals switching from left to right: term0 -> term1 -> term2 -> term0 -> term1 ... */
    curr_index = (curr_index+1)%3;
    /* if shell has not been opened, open shell first*/
    if (terminals[curr_index].shell_on == 0) {
        /* remap the video memory */
        vidmap_switch(curr_index);      /* remap video memory */
        printf("Try to open shell on term %d\n", curr_index);
        execute((const uint8_t*)"shell\n");
    }
    /* get the PCB of the next process */
    next_PCB = get_pcb(terminals[curr_index].active_pid);

    /* set up paging */
    int index = USR_ADDR >> 22; //shift 22 to get pde index
    set_pde_mb(index, 1);
    page_directory[index].MB.user_supervisor = 1;
    page_directory[index].MB.global = 0;
    page_directory[index].MB.table_address = 2 + terminals[curr_index].active_pid;   // skip first page

    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );

    /* save tss */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = _8MB - (_8KB * terminals[curr_index].active_pid) - sizeof(int32_t);

    vidmap_switch(curr_index);
    sti();
    /* do context switch, swapping ESP and EBP */
    asm volatile(
        "movl %%eax, %%esp ;"
        "movl %%ebx, %%ebp ;"
        "leave             ;"
        "ret               ;"
        :
        : "a" (next_PCB->saved_esp) ,"b" (next_PCB->saved_ebp) 
    );

    return;
}
