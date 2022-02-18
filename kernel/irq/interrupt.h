#ifndef IRQ_INTERRUPT_H
#define IRQ_INTERRUPT_H

void int_enable();
void int_disable();
int int_get();

/* Macros for interrupt controller */
#define IRQ_CLEAR(x)   (*(volatile u16*) 0x12) = (1<<(x))
#define IRQ_PENDING(x) (*(volatile u16*) 0x10) & (1<<(x))

#endif