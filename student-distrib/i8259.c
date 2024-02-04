/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/*
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Set ports and send data for initialization sequence
 */  
void i8259_init(void) {
    /* save masks */
    outb(master_mask, PIC1_DATA);
    outb(slave_mask, PIC2_DATA);    

    /* send ICW1 */
	outb(ICW1, PIC1_COMMAND);       // starts the intialization sequence
	outb(ICW1, PIC2_COMMAND);

	/* send ICW2 */
	outb(OFFSET1, PIC1_DATA);	/* remap */
	outb(OFFSET2, PIC2_DATA);	/*  pics */

	/* send ICW3 */
	outb(4, PIC1_DATA);	// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(2, PIC2_DATA); // ICW3: tell Slave PIC its cascade identity (0000 0010)

	/* send ICW4 */
	outb(ICW4, PIC1_DATA);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(ICW4, PIC2_DATA);

    /* restore masks */
    outb(master_mask, PIC1_DATA);
    outb(slave_mask, PIC2_DATA);

    enable_irq(2);      // enable irq for PIC at 2
}

/*
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: N/A
 */  
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port); 
}

/*
 * disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: N/A
 */  
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;

    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) | (1 << irq_num);
    outb(value, port);  
}

/*
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: N/A
 */  
void send_eoi(uint32_t irq_num) {
    if (irq_num >= 15) {
        return;
    }
    if(irq_num >= 8) {
		outb((PIC_EOI | (irq_num-8)), PIC2_COMMAND);
        outb((PIC_EOI | 2), PIC1_COMMAND);
        return;
    }
    else {
        outb(PIC_EOI | irq_num, PIC1_COMMAND);
        return;
    }
}
