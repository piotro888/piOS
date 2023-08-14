#include "panic.h"
#include <libk/kprintf.h>
#include <irq/interrupt.h>

__attribute__((noreturn))
_Noreturn void panic(char* message) {
    int_disable();
    kprintf("\n\033[97;91m[!] KERNEL PANIC\033[97m\n"); //FIXME [97;101m
    kprintf("Caused by: %s\n", message);
    print_stacktrace();
    kprintf("*kernel halted*");
    __asm__ volatile (
        "__panic_loop:\n"
        "ldi r0, 0xAB\n"
        "jmp __panic_loop\n"
    );
    __builtin_unreachable();
}

void print_stacktrace() {
    u16 fp;
    asm volatile (
        "mov %0, r5\n"
        : "=r" (fp)
    );

    kprintf("Stack trace:\n");
    for(int st_lim=0; st_lim<10; st_lim++) {
        u16 prev_ra = *(u16 *) fp;
        u16 prev_fp = *(u16 *) (fp - 2);
        // TODO: Load symbols map here
        kprintf("ret=%x fp=%x\n", prev_ra, prev_fp);
        if(!prev_ra || fp == 0xfffe || fp == 0xeff0)
            return;
        fp = prev_fp;
    }
}
