#ifndef _PIT_H
#define _PIT_H

#define PIT_IRQ     0x0     // The output from PIT channel 0 is connected to the PIC chip, so that it generates an "IRQ 0"
#define PIT_IO_DATA 0x40    // used for data reading and writing
#define PIT_IO_CMD  0x43    // used for mode/command (write only)
#define PIT_CMD     0x37    // channel 0: 00 (bits 6 and 7); lobyte/hibyte access mode: 11 (bits 4 and 5); square wave operating mode: 011 (bits 1 to 3); binary mode: 0 (bit 0)
#define PIT_FREQ    100     // use OS Dev suggested frequency: 100 Hz

void init_PIT ();    
void PIT_handler ();

#endif
