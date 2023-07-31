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
#include <sys/sysd.h>

#define SFLAG_SYSCALL 0x2
#define SFLAG_SEGFAULT 0x8

static int processing_interrupt = 0;

/* Interrupt handler called from irq.s */
__attribute__((used))
void interrupt(const int state) {
    if (processing_interrupt) {
        // detect unexpected faults in interrupt processing
        __asm__ volatile (
            "__double_fault:\n"
            "ldi r0, 0xAA\n"
            "jmp __double_fault\n"
        );
        __builtin_unreachable();
    }
    processing_interrupt = 1;

    if(!scheduling_enabled) {
        enable_default_memory_paging();
        if (state & SFLAG_SYSCALL)
            kprintf("EARLY KERNEL SEGFAULT!");
        kprintf("pc: %x\n",  *(int*)(state+28-20));
        panic("Kernel interupted before scheduling enabled");
    }

    // save thread state
    for(int i=0; i<8; i++)
        current_proc->regs[i] = *(int*)(state+28-(4+2*i));
    current_proc->pc =          *(int*)(state+28-20);
    current_proc->arith_flags = *(int*)(state+28-22);

    u16 status_flag = (*(int*)(state+28-24));

    enable_default_memory_paging();
    handling_interrupt = 1;

    // Thread should be only switched on syscall and timer interrupts
    int should_switch_thread = 0;
 
    if((status_flag & SFLAG_SYSCALL) && (current_proc->type == PROC_TYPE_USER || current_proc->type == PROC_TYPE_PRIV)) {
        log_irq("syscall: r0 %d pc 0x%x", current_proc->regs[0], current_proc->pc);
        sysd_submit(current_proc->pid);
        should_switch_thread = 1;
    }

    if(status_flag & SFLAG_SEGFAULT) {
        log_irq("SEGFAULT! pc: %x", current_proc->pc);
        log_irq("PID: %d DUMP r0:0x%x r1:0x%x r2:0x%x r3:0x%x r4:0x%x r5:0x%x r6:0x%x r7:0x%x vpc:0x%x",
                current_proc->pid, current_proc->regs[0], current_proc->regs[1], current_proc->regs[2], current_proc->regs[3], current_proc->regs[4], current_proc->regs[5],
                current_proc->regs[6], current_proc->regs[7], current_proc->pc);
        if (current_proc->type != PROC_TYPE_KERNEL)
            panic("SEGFAULT INSIDE KERNEL");
    }

    /* syscall from thread is always just YIELD */
    if((status_flag & SFLAG_SYSCALL) && current_proc->type == PROC_TYPE_KERNEL) {
        should_switch_thread = 1;
    }

    if(current_proc->type == PROC_TYPE_INIT)
        should_switch_thread = 1;

    // interrupt request and syscall could happen at same time
    // clear interrupt pending from controller and process interrupts
    if(irq_pending(KEYBOARD_IRQ_ID)) {
        irq_clear(KEYBOARD_IRQ_ID);
        u8 scancode = *(SCANCODE_ADDR);
        print_scancode(scancode);
    }

    if(irq_pending(TIMER_IRQ_ID)) {
        irq_clear(TIMER_IRQ_ID);
        sys_ticks++;
        should_switch_thread = 1;
    }

    processing_interrupt = 0;

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

#define IRQC_PAGE 0x4
#define IRQ_STATUS_ADDR 0x18
#define IRQ_CLEAR_ADDR 0x1a
#define IRQ_MASK_ADDR 0x1c

void irq_clear(int id) {
    map_page_zero(IRQC_PAGE);
    (*(volatile u16*) IRQ_CLEAR_ADDR) = (1<<(id));
    map_page_zero(ILLEGAL_PAGE);
}

int irq_pending(int id) {
    map_page_zero(IRQC_PAGE);
    int en = ((*(volatile u16*) IRQ_STATUS_ADDR) & (1<<(id)));
    map_page_zero(ILLEGAL_PAGE);
    return en;
}

void irq_mask(int id, int en) {
    map_page_zero(IRQC_PAGE);
    if (en)
        *((volatile u16*)IRQ_MASK_ADDR) |= (1<<id);
    else
        *((volatile u16*)IRQ_MASK_ADDR) &= ~(1<<id);
    map_page_zero(ILLEGAL_PAGE);
}
