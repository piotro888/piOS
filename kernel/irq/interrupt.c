#include "interrupt.h"

#include <libk/kprintf.h>
#include <driver/keyboard.h>
#include <libk/assert.h>
#include <libk/types.h>
#include <libk/log.h>
#include <proc/sched.h>
#include <proc/proc.h>
#include <proc/virtual.h>
#include <irq/timer.h>

/* Interrupt handler called from irq.s */
__attribute__((used))
void interrupt(const char* state) {
    // save thread state
    for(int i=0; i<8; i++)
        current_proc->regs[i] = *(int*)(state+16-(3+i));
    current_proc->pc =          *(int*)(state+16-11);
    current_proc->arith_flags = *(int*)(state+16-12);

    // Thread should be only switched on syscall and timer interrupts
    int should_switch_thread = 0;

    // check for syscall flag in sr
    int syscall_flag = (*(int*)(state+16-13)) & 0x8;

    if(syscall_flag && (current_proc->type == PROC_TYPE_USER || current_proc->type == PROC_TYPE_PRIV)) {
        log("syscall: %d", current_proc->regs[0]);
        should_switch_thread = 1;
    }

    /* syscall from thread is always just YIELD */
    if(syscall_flag && current_proc->type == PROC_TYPE_KERNEL) {
        log("yield");
        should_switch_thread = 1;
    }

    // interrupt request and syscall could happen at same time
    // clear interrupt pending from controller and process interrupts
    if(IRQ_PENDING(0)) {
        IRQ_CLEAR(0);
        uint8_t scancode = *((int*)3);
        print_scancode(scancode);
    }

    if(IRQ_PENDING(TIMER_IRQ_ID)) {
        IRQ_CLEAR(TIMER_IRQ_ID);
        sys_ticks++;
        should_switch_thread = 1;
    }

    if(scheduling_enabled) {
        if(should_switch_thread)
            sched_pick_next();

        ASSERT(current_proc->state != PROC_STATE_UNLOADED);
        switch_to_userspace(current_proc);
        ASSERT_NOT_REACHED();
    }

    /* only allow returning here with virtual memory disabled
     * i.e. before scheduling (when kernel is booting before threads)
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
    : "=r" (sr)
    );

    return (sr & 0x4) > 0;
}
