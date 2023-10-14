#ifndef PROC_SIGNAL_H
#define PROC_SIGNAL_H

#include <proc/proc.h>
#include <sys/systructs.h>

#define SIG_TYPE_CONTROL 1
#define SIG_TYPE_ALARM 2
#define SIG_TYPE_MQ 4
#define SIG_TYPE_AIO 8
#define SIG_TYPE_USER 16

#define SIG_CONTROL_CHLD 1

void signal_send(struct proc* proc, struct signal* signal);

void signal_handler_enter(struct proc* proc, struct signal* signal);
void signal_handler_return(struct proc* proc);

void signal_send_sigchld(struct proc* chld);

#endif
