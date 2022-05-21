#ifndef PROC_SCHED_H
#define PROC_SCHED_H

#include <proc/proc.h>

extern struct proc* current_proc;

extern int scheduling_enabled;
extern int first_free_page, first_free_prog_page;

void scheduler_init();

void sched_pick_next();

int make_kernel_thread(char* name, void __attribute__((noreturn)) (*entry)());
struct proc* sched_init_user_thread();

struct proc* proc_by_pid(int pid);

#define YIELD() asm volatile ("sys")

#endif
