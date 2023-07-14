#ifndef LIBPIOS_SYS
#define LIBPIOS_SYS

/* include syscall numbers shared with kernel */
#include "shared/syslist.h"

unsigned int syscall_raw(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4) {
    unsigned int ret;
    asm volatile (
        " ldo r0, %1 \n"
        " ldo r1, %2 \n"
        " ldo r2, %3 \n"
        " ldo r3, %4 \n"
        " ldo r6, %5 \n"
        " sys \n"
        " mov %0, r0 \n"
        : "=r"(ret)
        : "m"(p0), "m"(p1), "m"(p2), "m"(p3), "m"(p4)
        : "r0", "r1", "r2", "r3", "r6");
    return ret;
}

#endif