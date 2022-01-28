#ifndef PROC_PROC_H
#define PROC_PROC_H

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
};

#define PROC_STATE_NULL 0
#define PROC_STATE_LOADED 1

#define PROC_TYPE_INIT 0
#define PROC_TYPE_USER 1
#define PROC_TYPE_KERNEL 2
#define PROC_TYPE_PRIV 3

#endif