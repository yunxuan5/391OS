#include "PIT.h"
#include "i8259.h"
#include "lib.h"
#include "scheduler.h"

/* all of implementations below are adapted from https://wiki.osdev.org/Pit and http://www.osdever.net/bkerndev/Docs/pit.htm */
/*
 * init_PIT()
 *   Description: initialize device PIT
 *   Inputs: none
 *   Outputs: none
 *   Side effects: set up PIT for scheduling
 */
void init_PIT() {
    printf("haha\n");
    // int count = 1193180 / PIT_FREQ;    // initialize count for having a PIT frequency of 100 Hz
    int count = 11932;
    /* set the PIT operating mode */
    outb(PIT_CMD, PIT_IO_CMD);
    /* set the reload value: send the low 8 bits followed by the high 8 bits */
    outb(count&0xFF, PIT_IO_DATA);		// Low byte
	outb((count&0xFF00)>>8, PIT_IO_DATA);	// High byte
    /* enable its IRQ on PIC */
    enable_irq(PIT_IRQ);
    return;
}

/*
 * PIT_handler()
 *   Description: calls scheduler to handle process switch 
 *   Inputs: none
 *   Outputs: none
 *   Side effects: send eoi
 */
void PIT_handler() {
    send_eoi(PIT_IRQ);

    /* To be done: context switch */
    //printf("before\n");
    scheduler();
    //printf("after\n");
    return;
}