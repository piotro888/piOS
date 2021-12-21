/* 
 * piOS kernel entry point
 * (C) 2021 by Piotr Wegrzyn 
*/

#include <driver/tty.h>
#include <irq/interrupt.h>
#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <fs/tar.h>
#include <driver/sd.h>

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
    kprintf("initializing devices\n");
    sd_init();
    kprintf("enabling interrupts\n");
    int_enable();
    kprintf("init done.\n");

    uint8_t buff[512];
    for(int i=0; i<4; i++) {
        kprintf("reading sector %d\n", i);
        sd_read_block(buff, i);
        for(int i=0; i<512; i++){
            kprintf("%c", buff[i]);
        }
        kprintf("end\n");
    }
    
    
    tar_test();
}
