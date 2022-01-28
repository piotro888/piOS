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
#include <proc/proc.h>
#include <proc/sched.h>

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
    // kprintf("initializing devices\n");
    // sd_init();
    kprintf("enabling interrupts\n");
    int_enable();
    kprintf("init done.\n");
    kprintf("Happy new year!\n");
    kprintf(BOOT_ART);
    //tar_make_dir_tree();

    kprintf("loading program\n");
    int program_buff[10];
    program_buff[0] = 0x4;
    program_buff[1] = 0x5;
    //program_buff[2] = 0xe;
    //program_buff[3] = 0x1; // looping at addr 1 previously broke irq
    program_buff[2] = 0x0;
    program_buff[3] = 0x0; // looping at addr 1 previously broke irq
    //program_buff[2] = 0x12; // interrupt!
    //program_buff[3] = 0x0;
    program_buff[4] = 0x12; // interrupt!
    program_buff[5] = 0x0;
    //program_buff[4] = 0x0;
    //program_buff[5] = 0x0;
    program_buff[6] = 0x4; 
    program_buff[7] = 0x3; // load 3 to r0 to see if next insn is executed
    program_buff[8] = 0xe;
    program_buff[9] = 0x3;
    load_into_userspace_program(16, program_buff);

    kprintf("switching to userspace\n");
    scheduling_enabled = 1;
    current_proc.pid = 1;
    current_proc.state = PROC_STATE_LOADED;
    current_proc.pc = 0;
    current_proc.regs[0] = 1;
    current_proc.regs[1] = 11;
    current_proc.regs[2] = 12;
    current_proc.regs[3] = 13;
    current_proc.regs[4] = 14;
    current_proc.regs[5] = 15;
    current_proc.regs[6] = 16;
    current_proc.regs[7] = 17;
    switch_to_userspace(&current_proc);

    tty_set_color(0x09);
    kprintf("\nkernel halted.\n");
    tty_set_color(0x07);
}
