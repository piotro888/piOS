#ifndef SYS_SYSD_H
#define SYS_SYSD_H

int sysd_submit(int pid);
int sysd_resubmit(int pid);

void sysd_init();

#define SYS_DUMP    0
#define SYS_PRINT   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_READ    4
#define SYS_WRITE   5
#define SYS_IOCTL   6
#define SYS_EXIT    7

#endif
