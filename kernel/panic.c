#include <libk/kprintf.h>

void panic(char* message) {
    kprintf("kernel panic  %s", message);
    while(1);
}