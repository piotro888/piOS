#ifndef SYS_SYSD_H
#define SYS_SYSD_H

#include <sys/syscodes.h>
#include <proc/proc.h>

int process_syscall(struct proc_state* state);

#endif
