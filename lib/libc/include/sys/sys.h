#ifndef __SYS_SYS_H
#define __SYS_SYS_H

#include <sys/syscodes.h>
#include <stddef.h>

unsigned int syscall_raw(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4);

void __attribute__((always_inline)) sys_dump();
void sys_print(char* str);
int sys_open(char* path);
int sys_close(int fd);
int sys_read(int fd, void* buff, size_t size);
int sys_write(int fd, void* buff, size_t size);

#endif
