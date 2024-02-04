#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define IRQ_NUM 8   // designated IRQ port on PIC
#define IO_PORT1 0x70   // IO port 1, used to specify an index
#define IO_PORT2 0x71   // IO port 2, used to read/write
#define STATUS_REG_A 0x8A   // RTC status register A
#define STATUS_REG_B 0x8B   // RTC status register B
#define STATUS_REG_C 0x8C   // RTC status register C
/* Periodic Interrupt Rate and Square-Wave Output Frequency */
#define MAX_RATE 0x06
#define MAX_FREQ 1024
#define MIN_FREQ 2
#define REG_A_MASK 0xF0
#define REG_B_MASK 0x40

extern void init_RTC ();    
extern void RTC_handler ();
extern void reenable_NMI ();
int32_t RTC_open (const uint8_t* fname);
int32_t RTC_close (int32_t fd);
int32_t RTC_read (int32_t fd, void* buf, int32_t nbytes);
int32_t RTC_write (int32_t fd, const void* buf, int32_t nbytes);

#endif
