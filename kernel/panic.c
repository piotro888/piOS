#include "panic.h"
#include <libk/kprintf.h>
#include <driver/tty.h>

void panic(char* message) {
    tty_set_color(0x9F);
    kprintf("\n[!] KERNEL PANIC\n");
    tty_set_color(0xF);
    kprintf("Caused by: %s\n", message);
    kprintf("*kernel halted*");
    while(1);
}