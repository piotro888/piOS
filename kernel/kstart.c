/* 
 * piOS kernel entry point
 * (C) 2021 by Piotr Wegrzyn 
*/

#include <video/tty.h>
#include <libk/kprintf.h>
#include <irq/interrupt.h>

/* C entry point for kernel */
void _kstart() {
    init_tty();
    tty_set_color((char)0x02 | 0x08);
    tty_puts("pios");
    tty_set_color((char)0x0F);
    tty_puts(" kernel booting\n");
    tty_putc('\n');
    tty_putc('a');
    tty_putc('b');
    tty_putc('c');
    kprintf("cab");
    kprintf("int%db%db%db%db%db%dc", 1287, 33, 888, 3, 5, 9);
    kprintf("okaaaaaaaaaaaaaaaaaaa");
    int_enable();
}
