#ifndef IRQ_INTERRUPT_H
#define IRQ_INTERRUPT_H

#include <libk/types.h>

void int_enable();
void int_disable();
int int_get();

/* Interrupt controller functions */

void irq_clear(int id);
int irq_pending(int id);
void irq_mask(int id, int en);

#endif
