#include "RTC.h"
#include "lib.h"
#include "i8259.h"
/* Global variables */
int num_interrupts;
int int_count;
volatile int rtc_interrupt_occurred;
/* all information below are adopted from https://wiki.osdev.org/RTC */

/*
 * init_RTC
 *   DESCRIPTION: initialize RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: also sets virtualization for RTC
 */
void init_RTC () {
    outb(STATUS_REG_B, IO_PORT1);   // select register B
    char prev1 = inb(IO_PORT2);     // read register B
    outb(STATUS_REG_B, IO_PORT1);   // select register B
    outb(prev1 | REG_B_MASK, IO_PORT2);   // ORed previous value with 0x40, which turns on bit 6 of reg B
    /* RTC virtualization */
    outb(STATUS_REG_A, IO_PORT1);		// set register A
    char prev2=inb(IO_PORT2);	        // read register A
    outb(STATUS_REG_A, IO_PORT1);		// reset to A
    outb((prev2 & REG_A_MASK) | MAX_RATE, IO_PORT2); // write max rate to A (rate is the bottom 4 bits)
    enable_irq(IRQ_NUM);                // enable irq for RTC
}
/*
 * RTC_handler
 *   DESCRIPTION: handles irq requests from PIC 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: call RTC test function for checkpoint 1
 */  
void RTC_handler () {
    outb(STATUS_REG_C, IO_PORT1);   // set register C
    inb(IO_PORT2);	                // throw away contents
    int_count--;                    // decrement int_count
    if (int_count == 0) {           
        rtc_interrupt_occurred = 1; // set the interrupt flag
        int_count = num_interrupts; // reset the count
    }
    send_eoi(IRQ_NUM);              // signal PIC that interrupt is done
}
/*
 * reenable_NMI
 *   DESCRIPTION: re-enable Non-Maskable-Interrupts 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: re-enable Non-Maskable-Interrupts
 */   
void reenable_NMI () {
    // set bit 7 on IO port 0x70
    outb(inb(IO_PORT1) & 0x7f, IO_PORT1);
    // The CMOS RTC expects a read or write to port 0x71 after any write to port 0x70, or it may go into an undefined state
    inb(IO_PORT2);
}

/*
 * RTC_open
 *   DESCRIPTION: file open function for RTC
 *   INPUTS: const uint8_t* fname
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: set RTC frequency to 2 HZ as the default RTC frequency
 */ 
int32_t RTC_open (const uint8_t* fname) {
    num_interrupts = MAX_FREQ/MIN_FREQ;     // set RTC frequency to 2 HZ (default rate)
    int_count = num_interrupts;         // set the count
    return 0;
}

/*
 * RTC_close
 *   DESCRIPTION: file close function for RTC
 *   INPUTS: int32_t fd
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t RTC_close (int32_t fd) {
    // do nothing
    return 0;
}

/*
 * RTC_write
 *   DESCRIPTION: file write function for RTC
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: set RTC frequency according to argument buf
 */
int32_t RTC_write (int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL) {
        printf("Invalid buffer!\n");
        return -1;  // failure on invalid pointer
    }
    if (nbytes != sizeof(int32_t)) {
        printf("Invalid size!\n");
        return -1;  // incorrect size
    }
    int RTC_freq;
    RTC_freq = *((int*)buf);
    if (RTC_freq > 1024 || RTC_freq < 2) {
        printf("Not within the supported frequency range!\n");
        return -1;  // incorrect range
    }
    if ((RTC_freq) & ((RTC_freq-1) != 0)) {
        printf("Frequency is not a power of 2!\n");
        return -1;  // not a power of 2
    }
    num_interrupts = MAX_FREQ/RTC_freq;     // calculate the number of interrupts 
    int_count = num_interrupts;
    return 0;
}

/*
 * RTC_read
 *   DESCRIPTION: file read function for RTC
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 
 *   SIDE EFFECTS: wait for RTC handler
 */
int32_t RTC_read (int32_t fd, void* buf, int32_t nbytes) {
    // wait for the RTC handler
    rtc_interrupt_occurred = 0;
    while(rtc_interrupt_occurred == 0) {
    }
    return 0;
}
