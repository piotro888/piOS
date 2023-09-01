#include "interrupt.h"

#include <string.h>
#include <libk/kprintf.h>
#include <driver/keyboard.h>
#include <libk/assert.h>
#include <libk/types.h>
#include <libk/log.h>
#include <libk/kmalloc.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/proc.h>
#include <proc/virtual.h>
#include <irq/timer.h>
#include <sys/sysd.h>

#define SFLAG_SYSCALL 0x2
#define SFLAG_SEGFAULT 0x8

static int in_interrupt_handler = 0;

extern void int_sys_init(int stack_page);

#define IN_KERNEL(proc) ((proc)->type == PROC_TYPE_KERNEL  \
    || (proc)->state == PROC_STATE_SYSCALL || (proc)->state == PROC_STATE_SYSCALL_BLOCKED)


struct int_handler_state {
    unsigned regs[8];
    unsigned pc;
    unsigned arith_flags;
    unsigned irq_flags;
};

/* Interrupt handler called from irq.s */
__attribute__((used))
void interrupt(struct int_handler_state* state) {
    if (in_interrupt_handler) {
        // detect unexpected faults in interrupt processing
        __asm__ volatile (
            "__double_fault:\n"
            "ldi r0, 0xAA\n"
            "jmp __double_fault\n"
        );
        __builtin_unreachable();
    }
    in_interrupt_handler = 1;

    if(!scheduling_enabled) {
        enable_default_memory_paging();
        if (state->irq_flags & SFLAG_SEGFAULT)
            kprintf("EARLY KERNEL SEGFAULT!");
        kprintf("pc: %x\n",  *(int*)(state+28-20));
        panic("Kernel interupted before scheduling enabled");
    }

    // default kernel stack pointer is used
    enable_default_memory_paging();
    int_no_proc_modify = 1;

    // save state in current proc
    for(int i=0; i<8; i++)
        current_proc->proc_state.regs[i] = state->regs[i];
    
    current_proc->proc_state.pc = state->pc;
    current_proc->proc_state.arith_flags = state->arith_flags;

    // Thread should be only switched on syscall and timer interrupts
    int should_switch_thread = 0;

    if(state->irq_flags & SFLAG_SEGFAULT) {
        log_irq("SEGFAULT! pc: %x", current_proc->proc_state.pc);
        log_irq("PID: %d DUMP r0:0x%x r1:0x%x r2:0x%x r3:0x%x r4:0x%x r5:0x%x r6:0x%x r7:0x%x vpc:0x%x",
                current_proc->pid, current_proc->proc_state.regs[0], current_proc->proc_state.regs[1], current_proc->proc_state.regs[2],
                current_proc->proc_state.regs[3], current_proc->proc_state.regs[4], current_proc->proc_state.regs[5],
                current_proc->proc_state.regs[6], current_proc->proc_state.regs[7], current_proc->proc_state.pc);
        if (IN_KERNEL(current_proc))
            panic("SEGFAULT INSIDE KERNEL");
    }

    /* syscall from thread is always just YIELD */
    if((state->irq_flags & SFLAG_SYSCALL) && IN_KERNEL(current_proc)) {
        should_switch_thread = 1;
    }

    if(current_proc->type == PROC_TYPE_INIT)
        should_switch_thread = 1;

    // interrupt request and syscall could happen at same time
    // clear interrupt pending from controller and process interrupts
    if(irq_pending(KEYBOARD_IRQ_ID)) {
        map_page_zero(SCANCODE_PAGE);
        u8 scancode = *(SCANCODE_ADDR);
        map_page_zero(ILLEGAL_PAGE);
        irq_clear(KEYBOARD_IRQ_ID); // clear after read
        print_scancode(scancode);
    }

    if(irq_pending(TIMER_IRQ_ID)) {
        irq_clear(TIMER_IRQ_ID);
        sys_ticks++;
        should_switch_thread = 1;
    }

    if((state->irq_flags & SFLAG_SYSCALL) && !IN_KERNEL(current_proc)) {
        // set up syscall processing thread in kernel with new stack
        int_sys_init(current_proc->kernel_stack_page);
        ASSERT_NOT_REACHED();
    }

    in_interrupt_handler = 0;

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

// called from int_sys_init with new clear stack
_Noreturn void int_sys_call() {
    in_interrupt_handler = 0;

    // store syscall state
    memcpy(&current_proc->syscall_state, &current_proc->proc_state, sizeof(struct proc_state));
    
    for (int i=0; i<16; i++) {
        current_proc->proc_state.mem_pages[i] = 0x200+i;
        current_proc->proc_state.prog_pages[i] = i;
    }
    current_proc->proc_state.mem_pages[0] = ILLEGAL_PAGE;
    current_proc->proc_state.mem_pages[15] = current_proc->kernel_stack_page;

    // enable process as kernel thread
    current_proc->state = PROC_STATE_SYSCALL;

    int_no_proc_modify = 0;
    int_enable();

    log("syscall %x", current_proc->syscall_state.regs[0]);
    process_syscall(&current_proc->syscall_state);
    log("end %x", current_proc->syscall_state.regs[0]);    

    // recover user state and return
    int_disable();
    int_no_proc_modify = 1;
    memcpy(&current_proc->proc_state, &current_proc->syscall_state, sizeof(struct proc_state));

    if (current_proc->signal_queue.first != NULL && !current_proc->signals_hold && current_proc->sighandler) {
        // signall pending - switch to handler
        signal_handler_enter(current_proc, current_proc->signal_queue.first->val);
        kfree(current_proc->signal_queue.first->val);
        list_remove(&current_proc->signal_queue, current_proc->signal_queue.first);
        ASSERT(current_proc->signal_sema.count > 0);
        current_proc->signal_sema.count--;
    }

    current_proc->state = PROC_STATE_RUNNABLE;

    // switch_to_userspace expects calls from interrupt handler.
    // syscall kernel process, when resumed from sheduler, has program paging set
    // and executing switch from running process breaks it
    disable_program_paging();
    switch_to_userspace(current_proc);
    ASSERT_NOT_REACHED();
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
