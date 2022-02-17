#include <libk/kprintf.h>
#include <driver/keyboard.h>
#include <libk/assert.h>
#include <libk/types.h>
#include <proc/sched.h>
#include <proc/proc.h>
#include <proc/virtual.h>

#define IRQ_CLEAR(x)   (*(volatile u16*) 0x12) = (1<<(x))
#define IRQ_PENDING(x) (*(volatile u16*) 0x10) & (1<<(x)) 

/* Interrupt handler called from irq.s */
__attribute__((used))
void interrupt(const char* state) {
    kprintf("[interrupted]\n");

    // save thread state
    for(int i=0; i<8; i++)
        current_proc->regs[i] = *(int*)(state+16-(3+i));
    current_proc->pc =          *(int*)(state+16-11);
    current_proc->arith_flags = *(int*)(state+16-12);


    // check for syscall flag in sr
    if((*(int*)(state+16-13)) & 0x8) {
        kprintf("SYSCALL ");
    }

    // interrupt request and syscall could happen at same time
    // clear interrupt pending from controller and process interrupts
    if(IRQ_PENDING(0)) {
        IRQ_CLEAR(0);
        uint8_t scancode = *((int*)3);
        print_scancode(scancode);
    }

    if(scheduling_enabled) {
        // resume execution from current thread
        kprintf("[ie to thread %d]", current_proc->pid);
        sched_pick_next();
        ASSERT(current_proc->state != PROC_STATE_UNLOADED);
        switch_to_userspace(current_proc);
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
