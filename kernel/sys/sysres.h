#ifndef SYS_SYSRES_H
#define SYS_SYSRES_H

#include <proc/proc.h>

void sysres_init();

void sysres_notify();

// SYSCALL REGISTER BLOCK FUNCTIONS
void sysres_submit_read(struct proc* proc);
void sysres_submit_write(struct proc* proc);

#endif
