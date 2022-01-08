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
#include <art.h>
#include <proc/virtual.h>

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
    kprintf("Happy new year!\n");
    kprintf(BOOT_ART);
    tar_make_dir_tree();

    kprintf("loading program");
    int program_buff[10];
    program_buff[0] = 0x4;
    program_buff[1] = 0x5;
    program_buff[2] = 0xe;
    program_buff[3] = 0x0;
    load_into_userspace_program(16, program_buff);
    kprintf("switching to userspace");
    switch_to_userspace();

    tty_set_color(0x09);
    kprintf("\nkernel halted.\n");
    tty_set_color(0x07);
}
