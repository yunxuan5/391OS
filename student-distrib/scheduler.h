#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "system_calls.h"
#include "types.h"

#define _8MB            0x800000
#define _4MB            0x400000
#define _8KB            0x2000

extern volatile int32_t curr_index;

void scheduler();

#endif
