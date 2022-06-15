#ifndef IRQ_INTERRUPT_H
#define IRQ_INTERRUPT_H

void int_enable();
void int_disable();
int int_get();

/* Macros for interrupt controller */
#define IRQ_CLEAR_ADDR 0x24
#define IRQ_STATUS_ADDR 0x20
#define IRQ_CLEAR(x)   (*(volatile u16*) IRQ_CLEAR_ADDR) = (1<<(x))
#define IRQ_PENDING(x) ((*(volatile u16*) IRQ_STATUS_ADDR) & (1<<(x)))

#endif
