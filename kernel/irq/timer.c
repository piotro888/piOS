#include "timer.h"

#include <libk/types.h>
#include <irq/interrupt.h>

#define TIMER_VALUE (volatile u16*) 0x14
#define TIMER_CLK_DIV (volatile u16*) 0x15
#define TIMER_AUTO_RESET (volatile u16*) 0x16

unsigned int sys_ticks = 0;

void timer_init() {
    // Timer frequency 100Hz
    // cycles = (1e6/100) = 10000
    // Set timer for 65535-10000 = 55535

    const int timer_val = 55535;

    *TIMER_CLK_DIV = 0;
    *TIMER_AUTO_RESET = timer_val;
    *TIMER_VALUE = timer_val;

    IRQ_CLEAR(TIMER_IRQ_ID);
}


