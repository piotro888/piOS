#ifndef __SYS_SYS_H
#define __SYS_SYS_H

#include <sys/syscodes.h>
#include <sys/systructs.h>
#include <stddef.h>

unsigned int syscall_raw(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4);

void __attribute__((always_inline)) sys_dump();
void sys_print(const char* str);
int sys_open(const char* path);
int sys_close(int fd);
int sys_read(int fd, void* buff, size_t size);
int sys_write(int fd, const void* buff, size_t size);
int sys_fcntl(int fd, unsigned flags);
int sys_procinfo(unsigned pid, struct sys_proc_info* proc_info);
int sys_pgmap(int page);
int sys_sigsend(int pid, unsigned type, unsigned number);
void* sys_sigaction(void (*handler)(struct signal*, int async));
#define sys_sighdlasyncret() __asm__ volatile ("ldi r0, 14\nsys");
int sys_sigwait(struct signal* result);
void sys_clockticks(unsigned long* time);
void sys_alarmset(unsigned long ticks);

#endif
