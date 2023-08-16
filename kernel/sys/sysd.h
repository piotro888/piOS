#ifndef SYS_SYSD_H
#define SYS_SYSD_H

#include <proc/proc.h>

int process_syscall(struct proc_state* state);

#define SYS_DUMP    0
#define SYS_PRINT   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_READ    4
#define SYS_WRITE   5
#define SYS_FCNTL   6
#define SYS_EXIT    7

#endif
