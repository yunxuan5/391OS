#include "IDT.h"
#include "keyboard.h"
#include "interrupt_linkage.h"
#include "lib.h"
#include "system_calls.h"
#include "system_calls_linkage.h"
/*
 * setup_idt
 *   DESCRIPTION: initialize IDT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: set attributes and descriptor entries for the IDT
 */  
void setup_idt () {
    int i;
    /* Set IDT entries */
    SET_IDT_ENTRY(idt[0x00], &exception_division_error);
    SET_IDT_ENTRY(idt[0x01], &exception_debug);
    SET_IDT_ENTRY(idt[0x02], &exception_NMI);
    SET_IDT_ENTRY(idt[0x03], &exception_breakpoint);
    SET_IDT_ENTRY(idt[0x04], &exception_overflow);
    SET_IDT_ENTRY(idt[0x05], &exception_bound_range_exceeded);
    SET_IDT_ENTRY(idt[0x06], &exception_invalid_opcode);
    SET_IDT_ENTRY(idt[0x07], &exception_device_not_available);
    SET_IDT_ENTRY(idt[0x08], &exception_double_fault);
    SET_IDT_ENTRY(idt[0x09], &exception_coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[0x0A], &exception_invalid_TSS);
    SET_IDT_ENTRY(idt[0x0B], &exception_segment_not_present);
    SET_IDT_ENTRY(idt[0x0C], &exception_stack_fault);
    SET_IDT_ENTRY(idt[0x0D], &exception_general_protection);
    SET_IDT_ENTRY(idt[0x0E], &exception_page_fault);
    /* no interrupt 15 defined */
    SET_IDT_ENTRY(idt[0x10], &exception_FPU_floating_point_error);
    SET_IDT_ENTRY(idt[0x11], &exception_alignment_check);
    SET_IDT_ENTRY(idt[0x12], &exception_machine_check);
    SET_IDT_ENTRY(idt[0x13], &exception_SIMD_floating_point_error);

    SET_IDT_ENTRY(idt[KEYBOARD], &keyboard_irq_handler_linkage); // call linkage function
    SET_IDT_ENTRY(idt[RTC], &RTC_linkage);  // call linkage function
    SET_IDT_ENTRY(idt[PIT], &PIT_linkage);  // call linkage function
    SET_IDT_ENTRY(idt[SYSTEM_CALL], &system_calls);
    /* go through first 20 exceptions first */
    for (i = 0; i < 20; i++) {
        idt[i].seg_selector = KERNEL_CS;    // all descriptors are in kernel code segment
        /* exceptions use trap gates, so the reserved bits are: 01110 according to Intel IA32 Manual */
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;    // our descriptors are always 32 bit
        idt[i].reserved0 = 0;
        idt[i].present = 1;
        idt[i].dpl = 0;     // ring 0 for kernel Descriptor Privilege Level
    }
    // go through interrupts
    for (i = 32; i < NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;    // all descriptors are in kernel code segment
        /* interrupts use interrupt gates, so the reserved bits are: 01100 according to Intel IA32 Manual */
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;    // our descriptors are always 32 bit
        idt[i].reserved0 = 0;
        idt[i].present = 1;
        idt[i].dpl = 0;
        if (i == SYSTEM_CALL) {
            idt[i].dpl = 3;     // system call is at Descriptor Privilege Level 3
        }
    }
    // for (i = 0; i < NUM_VEC; i++) {
    //     idt[i].seg_selector = KERNEL_CS;    // all descriptors are in kernel code segment
    //     idt[i].reserved4 = 0;
    //     if (i >= INTERRUPT_START && i <= INTERRUPT_END) {
    //         idt[i].reserved3 = 0;
    //     }
    //     else {
    //         idt[i].reserved3 = 1;
    //     }
    //     idt[i].reserved2 = 1;
    //     idt[i].reserved1 = 1;
    //     idt[i].size = 1;
    //     idt[i].reserved0 = 0;
    //     idt[i].present = 0;
    //     if (i == SYSTEM_CALL) {
    //         idt[i].dpl = 3;
    //     }
    //     else {
    //         idt[i].dpl = 0;
    //     }
    //     if (i < 20) {
    //         idt[i].present = 1;
    //     }
    //     if (i == 0x21 || i == 0x28) {
    //         idt[i].present = 1;
    //     }
    //     if (i == SYSTEM_CALL) {
    //         idt[i].present = 1;
    //     }

    // }
    lidt(idt_desc_ptr);     // load IDT
}

/*
 * exception handlers
 *   DESCRIPTION: For checkpoint 1, we only print out exception messages
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Reports exceptions
 */  
void exception_division_error() {
    printf("Division error \n");
    system_calls(halt(0x00));
    while(1);
}

void exception_debug() {
    printf("Debug exception \n");
    system_calls(halt(0x01));
    while(1);
}

void exception_NMI() {
    printf("Nonmaskable interrupt \n");
    system_calls(halt(0x02));
    while(1);
}

void exception_breakpoint() {
    printf("Breakpoint exception \n");
    system_calls(halt(0x03));
    while(1);
}

void exception_overflow() {
    printf("Overflow exception \n");
    system_calls(halt(0x04));
    while(1);
}

void exception_bound_range_exceeded() {
    printf("Bound range exceeded \n");
    system_calls(halt(0x05));
    while(1);
}

void exception_invalid_opcode() {
    printf("Invalid opcode \n");
    system_calls(halt(0x06));
    while(1);
}

void exception_device_not_available() {
    printf("Device not available \n");
    system_calls(halt(0x07));
    while(1);
}

void exception_double_fault() {
    printf("Double fault \n");
    system_calls(halt(0x08));
    while(1);
}

void exception_coprocessor_segment_overrun() {
    printf("Coprocessor segment overrun \n");
    system_calls(halt(0x09));
    while(1);
}

void exception_invalid_TSS() {
    printf("Invalid TSS \n");
    system_calls(halt(0x0A));
    while(1);
}

void exception_segment_not_present() {
    printf("Segment not present \n");
    system_calls(halt(0x0B));
    while(1);
}

void exception_stack_fault() {
    printf("Stack fault \n");
    system_calls(halt(0x0C));
    while(1);
}

void exception_general_protection() {
    printf("General protection \n");
    system_calls(halt(0x0D));
    while(1);
}

void exception_page_fault() {
    printf("Page fault \n");
    system_calls(halt(0x0E));
    while(1);
}

void exception_FPU_floating_point_error() {
    printf("FPU Floating point error \n");
    system_calls(halt(0x10));
    while(1);
}

void exception_alignment_check() {
    printf("Alignment check \n");
    system_calls(halt(0x11));
    while(1);
}

void exception_machine_check() {
    printf("Machine check \n");
    system_calls(halt(0x12));
    while(1);
}

void exception_SIMD_floating_point_error() {
    printf("SIMD floating point error \n");
    system_calls(halt(0x13));
    while(1);
}
