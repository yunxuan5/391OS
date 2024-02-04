#ifndef _INTERRUPT_LINKAGE_H_
#define _INTERRUPT_LINKAGE_H_

#ifndef ASM
    extern void keyboard_irq_handler_linkage();
    extern void RTC_linkage();
    extern void PIT_linkage();
#endif

#endif
