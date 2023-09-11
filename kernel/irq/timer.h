#ifndef IRQ_TIMER_H
#define IRQ_TIMER_H

extern unsigned long sys_ticks;

void timer_init();

#define TIMER_IRQ_ID 0

#endif
