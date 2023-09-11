#ifndef PROC_PROC_H
#define PROC_PROC_H

#include <libk/types.h>
#include <libk/con/semaphore.h>
#include <libk/list.h>

#define PROC_MAX_FILES 16

struct proc_file {
    struct inode* inode;
    
    size_t offset;
    unsigned fcntl_flags;
};

struct proc_state {
    int regs[8];
    int pc;
    int arith_flags;

    // paging
    int mem_pages[16];
    int prog_pages[16];
};


struct proc {
    int pid;

    int state;
    int type;

    // process saved state
    struct proc_state proc_state;
    struct proc_state syscall_state;

    // short proc name
    char name[16];

    int kernel_stack_page;

    struct proc_file open_files[PROC_MAX_FILES];

    // blocking support
    struct semaphore* sema_blocked;

    // signals
    struct list signal_queue;
    int signals_hold;
    void* sighandler;
    struct semaphore signal_sema;

    void* load_brk;
    unsigned long alarm_ticks;
};

#define PROC_STATE_UNLOADED 0
#define PROC_STATE_RUNNABLE 1
#define PROC_STATE_BLOCKED 2
#define PROC_STATE_SYSCALL 3
#define PROC_STATE_SYSCALL_BLOCKED 4

// init process skips virtual memory (spins in kernel loop and handles interrupts, not selected by scheduler)
#define PROC_TYPE_INIT 0
#define PROC_TYPE_USER 1
#define PROC_TYPE_KERNEL 2
#define PROC_TYPE_PRIV 3

#endif
