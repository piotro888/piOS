#include "sys/sys.h"

#include <string.h>

unsigned int syscall_raw(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4) {
    unsigned int ret;
    
    __asm__ volatile (
        " ldo r0, %1 \n"
        " ldo r1, %2 \n"
        " ldo r2, %3 \n"
        " ldo r3, %4 \n"
        " ldo r6, %5 \n"
        " sys \n"
        " mov %0, r0 \n"
        : "=r"(ret)
        : "m"(p0), "m"(p1), "m"(p2), "m"(p3), "m"(p4)
        : "r0", "r1", "r2", "r3", "r6"
    );
    return ret;
}

void __attribute__((always_inline)) sys_dump() {
    __asm__ volatile (
        "\tldi r0, 0 \n"
        "\tsys"
        ::: "r0"
    );
}

void sys_print(const char* str) {
    syscall_raw(SYS_PRINT, (int)str, strlen(str), 0, 0);
}

int sys_open(const char* path) {
    return syscall_raw(SYS_OPEN, (int)path, strlen(path), 0, 0);
}

int sys_close(int fd) {
    return syscall_raw(SYS_CLOSE, fd, 0, 0, 0);
}

int sys_read(int fd, void* buff, size_t size) {
    return syscall_raw(SYS_READ, fd, (int)buff, size, 0);
}

int sys_write(int fd, const void* buff, size_t size) {
    return syscall_raw(SYS_WRITE, fd, (int)buff, size, 0);
}

int sys_fcntl(int fd, unsigned flags) {
    return syscall_raw(SYS_FCNTL, fd, flags, 0, 0);
}

int sys_procinfo(unsigned pid, struct sys_proc_info* proc_info) {
    return syscall_raw(SYS_PROCINFO, pid, (int)proc_info, 0, 0);
}

int sys_pgmap(int page) {
    return syscall_raw(SYS_PGMAP, page, 0, 0, 0);
}
