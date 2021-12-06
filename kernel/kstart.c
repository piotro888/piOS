/* 
 * piOS kernel entry point
 * (C) 2021 by Piotr Wegrzyn 
*/

#include <driver/tty.h>
#include <irq/interrupt.h>
#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <fs/tar.h>

/* C entry point for kernel */
void _kstart() {
    init_tty();
    tty_set_color((char)0x02 | 0x08);
    tty_puts("pios");
    tty_set_color((char)0x0F);
    tty_puts(" kernel booting\n");
    tty_putc('\n');
    kprintf("initializing kernel heap\n");
    init_malloc();
    kprintf("enabling interrupts\n");
    int_enable();
    kprintf("init done.\n");
    
    tar_test();
}
