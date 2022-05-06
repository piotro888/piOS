#ifndef PROC_SCHED_H
#define PROC_SCHED_H

#include <proc/proc.h>

extern struct proc* current_proc;

extern int scheduling_enabled;

void scheduler_init();

void sched_pick_next();

int make_kernel_thread(char* name, void __attribute__((noreturn)) (*entry)());

#define YIELD() asm volatile ("sys")

#endif
