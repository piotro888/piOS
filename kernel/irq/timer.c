#include "timer.h"

#include <libk/types.h>
#include <irq/interrupt.h>
#include <proc/virtual.h>

#define TIMER_PAGE 0x4
#define TIMER_VALUE (volatile u16*) 0x10
#define TIMER_CLK_DIV (volatile u16*) 0x12
#define TIMER_AUTO_RESET (volatile u16*) 0x14

unsigned long sys_ticks = 0;

void timer_init() {
    map_page_zero(TIMER_PAGE);

    // 100 interrupts per second
    const unsigned int timer_val = 34285;

    *TIMER_CLK_DIV = 3; // 3.125 MHz
    *TIMER_AUTO_RESET = timer_val;
    *TIMER_VALUE = timer_val;

    irq_clear(TIMER_IRQ_ID);
    irq_mask(TIMER_IRQ_ID, 1);

    map_page_zero(ILLEGAL_PAGE);
}
