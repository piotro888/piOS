/* 
 * piOS kernel entry point
 * (C) 2021 by Piotr Wegrzyn 
*/

#include <driver/tty.h>
#include <irq/interrupt.h>
#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <libk/log.h>
#include <libk/assert.h>
#include <irq/timer.h>
#include <fs/tar.h>
#include <driver/sd.h>
#include <art.h>
#include <proc/virtual.h>
#include <proc/sched.h>

__attribute__((noreturn)) void test_kthread() {
    tty_putc('x');
    for(;;) {
        int_disable();
        log("from thread");
        int_enable();
     //   asm volatile ("sys");
        for(int i=0; i<30000; i++){}
    }
}

__attribute__((noreturn)) void test_kthread2() {
    tty_putc('x');
    for(;;) {
        int_disable();
        log("thread2");
        int_enable();
        //   asm volatile ("sys");
        for(int i=0; i<30000; i++){}
    }
}

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
    kprintf("initializing scheduler\n");
    timer_init();
    scheduler_init();
    kprintf("enabling interrupts\n");
    int_enable();
    kprintf("init done.\n");
    kprintf(BOOT_ART);
    //tar_make_dir_tree();

    make_kernel_thread("tt", test_kthread);
    make_kernel_thread("tt2", test_kthread2);
    sched_pick_next();
    scheduling_enabled = 1;
    switch_to_userspace(current_proc);
    ASSERT_NOT_REACHED();

    tty_set_color(0x09);
    kprintf("\nkernel halted.\n");
    tty_set_color(0x07);
}
