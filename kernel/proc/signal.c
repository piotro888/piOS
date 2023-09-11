#include "signal.h"
#include <string.h>
#include <irq/interrupt.h>
#include <proc/virtual.h>
#include <libk/assert.h>
#include <libk/kmalloc.h>
#include <libk/log.h>

// TODO: cancel blocking i/o opreations from fs queues at call


void signal_send(struct proc* proc, struct signal* signal) {
    struct signal* gsignal = kmalloc(sizeof(struct signal));
    memcpy(gsignal, signal, sizeof(struct signal));
    list_append(&proc->signal_queue, gsignal);
    semaphore_up(&proc->signal_sema);
}

void signal_handler_enter(struct proc* proc, struct signal* signal) {
    // process must be set to userspace state here, execute just before cswitch
    log_irq("sighaenter %x %x >%x", proc->proc_state.regs[7], proc->proc_state.pc, proc->sighandler);
    proc->signals_hold = 1;
    unsigned sp = proc->proc_state.regs[7];
    sp -= 4; // prologue safe
    sp -= sizeof(struct proc_state);
    memcpy_to_userspace(proc, sp+2, &proc->proc_state, sizeof(struct proc_state));
    sp -= sizeof(struct signal);
    unsigned sigptr = sp+2;
    memcpy_to_userspace(proc, sp+2, signal, sizeof(struct signal));
    ASSERT(sp >= 0xf000u);
    proc->proc_state.regs[0] = sigptr;
    proc->proc_state.regs[1] = 1;
    proc->proc_state.regs[5] = sp;
    proc->proc_state.regs[6] = 0;
    proc->proc_state.regs[7] = sp;
    ASSERT(proc->sighandler);
    proc->proc_state.pc = (unsigned) proc->sighandler;
}

void signal_handler_return(struct proc* proc) {
    // to be called normally from sysd, fills userspace state to switch
    unsigned sp = proc->syscall_state.regs[5]; // frame pointer of called function is stack pointer of entry
    sp += sizeof(struct signal);
    memcpy_from_userspace(&proc->syscall_state, proc, sp + 2, sizeof(struct proc_state));
    sp += sizeof(struct proc_state) + 4;
    ASSERT(sp == proc->syscall_state.regs[7]); // verify return sp equals saved state
    proc->signals_hold = 0;
}
