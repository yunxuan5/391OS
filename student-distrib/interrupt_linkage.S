#define ASM     1
#include "interrupt_linkage.h"

/*
 * INTR_LINK
 *   DESCRIPTION: assembly linkage function called when an interrupt arises 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: call a corresponding irq handler
 */  
#define INTR_LINK(name, function)    \
    .global name                    ;\
    name:                           ;\
        pushal                      ;\
        pushfl                      ;\
        call function               ;\
        popfl                       ;\
        popal                       ;\
        iret                        ;\

INTR_LINK(keyboard_irq_handler_linkage, keyboard_irq_handler)
INTR_LINK(RTC_linkage, RTC_handler)
INTR_LINK(PIT_linkage, PIT_handler)
