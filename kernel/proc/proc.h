#ifndef PROC_PROC_H
#define PROC_PROC_H

#include <libk/con/semaphore.h>

struct proc {
    int pid;

    int state;
    int type;

    // process saved state
    int regs[8];
    int pc;
    int arith_flags;
    // paging
    int mem_pages[16];
    int prog_pages[16];

    // short proc name
    char name[16];

    // blocking support
    struct semaphore* sema_blocked;
};

#define PROC_STATE_UNLOADED 0
#define PROC_STATE_RUNNABLE 1
#define PROC_STATE_BLOCKED 2

// init process skips virtual memory (spins in kernel loop and handles interrutps, not selected by scheduler)
#define PROC_TYPE_INIT 0
#define PROC_TYPE_USER 1
#define PROC_TYPE_KERNEL 2
#define PROC_TYPE_PRIV 3

#endif