#include <libk/kprintf.h>

/* Interrupt handler called from irq.s */
void interrupt() {
    kprintf("irq\n");
}

/* Enable interrupts in CPU */
void int_enable() {
    asm volatile (
        "srl r0, 1\n"
        "ani r0, r0, 0x4\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}