#include <libk/kprintf.h>
#include <driver/keyboard.h>
#include <libk/assert.h>
#include <libk/types.h>
#include <proc/sched.h>
#include <proc/proc.h>
#include <proc/virtual.h>

/* Interrupt handler called from irq.s */
__attribute__((used))
void interrupt(char* state) {
    kprintf("[interrupted] ptr=%x pc=%x r0=%d r1=%d r2=%d pc+1=%d pc-1=%d  ", (int)state+16, *(int*)((int)state+16-11), *(int*)(state+16-3), *(int*)(state+16-4), *(int*)(state+16-5), *(int*)(state+16-10), *(int*)(state+16-12));

    // save thread state
    current_proc.regs[0] = *(int*)(state+16-3);
    current_proc.regs[1] = *(int*)(state+16-4);
    current_proc.regs[2] = *(int*)(state+16-5);
    current_proc.regs[3] = *(int*)(state+16-6);
    current_proc.regs[4] = *(int*)(state+16-7);
    current_proc.regs[5] = *(int*)(state+16-8);
    current_proc.regs[6] = *(int*)(state+16-9);
    current_proc.regs[7] = *(int*)(state+16-10);
    current_proc.pc =      *(int*)(state+16-11);
    current_proc.arith_flags = *(int*)(state+16-12);

    kprintf("\nState saved regs = %d %d %d %d pc = %d\n", current_proc.regs[0], current_proc.regs[1], current_proc.regs[2], current_proc.regs[3], current_proc.pc);

    uint8_t scancode = *((int*)3);
    //kprintf("irq scan:%d\n", scancode);
    print_scancode(scancode);

    if(scheduling_enabled) {
        // resume execution from current thread
        kprintf("[ie to thread %d]", current_proc.pid);
        ASSERT(current_proc.state != PROC_STATE_UNLOADED);
        switch_to_userspace(&current_proc);
    }

    kprintf("[ie]  ");
    /*  allow only returning here with virtual memory disabled
        i.e. before scheduling (when kernel is booting before threads and initializing)
    */ 
    ASSERT(!scheduling_enabled);
}

/* Enable interrupts in CPU */
void int_enable() {
    asm volatile (
        "srl r0, 1\n"
        "ori r0, r0, 0x4\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}

void int_disable() {
    asm volatile (
    "srl r0, 1\n"
    "ani r0, r0, 0xfb\n"
    "srs r0, 1\n"
    ::: "r0"
    );
}

int int_get() {
    u16 sr;
    asm volatile (
    "srl %0, 1\n"
    "ori %0, %0, 0x4\n"
    "srs %0, 1\n"
    : "=r" (sr)
    );

    return (sr & 0x4) > 0;
}
