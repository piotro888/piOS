#ifndef PROC_SCHED_H
#define PROC_SCHED_H

#include <proc/proc.h>
// supports only one thread for now
extern struct proc current_proc;

extern int scheduling_enabled;

#endif