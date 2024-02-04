#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "interrupt_linkage.h"

#define SYSTEM_CALL 0x80        // IDT port for system calls
#define INTERRUPT_START 0x20    // hardware interrupt starts
#define INTERRUPT_END 0x2F      // hardware interrupt ends
#define PIT 0x20                // IDT port for PIT
#define KEYBOARD 0x21           // IDT port for keyboard
#define RTC 0x28                // IDT port for RTC

void setup_idt ();
/* exceptions */
void exception_division_error();

void exception_debug();

void exception_NMI();

void exception_breakpoint();

void exception_overflow();

void exception_bound_range_exceeded();

void exception_invalid_opcode();

void exception_device_not_available();

void exception_double_fault();

void exception_coprocessor_segment_overrun();

void exception_invalid_TSS();

void exception_segment_not_present();

void exception_stack_fault();

void exception_general_protection();

void exception_page_fault();

void exception_FPU_floating_point_error();

void exception_alignment_check();

void exception_machine_check();

void exception_SIMD_floating_point_error();

// void system_calls();

#endif
