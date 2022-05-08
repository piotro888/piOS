/* 
 * piOS kernel entry point
 * (C) 2021-2022 by Piotr Wegrzyn
*/

#include <art.h>
#include <driver/sd.h>
#include <driver/tty.h>
#include <fs/kbd.h>
#include <fs/tar.h>
#include <fs/vfs.h>
#include <irq/timer.h>
#include <proc/sched.h>
#include <proc/virtual.h>

#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <libk/log.h>

void __attribute__((noreturn)) init_stage1();

/* C entry point for kernel */
__attribute__((used))
void _kstart() {
    tty_init_basic();
    tty_puts("\033[92mpios\033[97m");
    tty_puts(" kernel booting\n");
    tty_putc('\n');
    kprintf("initializing kernel heap\n");
    init_malloc();
    kprintf("%uB heap available\n", mem_free_size());
    kprintf("initializing vfs\n");
    vfs_init();
    kprintf("initializing scheduler\n");
    timer_init();
    scheduler_init();
    kprintf("initializing devices\n");
    sd_init();
    kprintf("initializing full tty driver\n");
    tty_init_driver();
    tty_register_thread();
    kprintf("init done.\n");
    kprintf("happy 100th commit! :)\n");
    kprintf(BOOT_ART);

    kprintf("registering kernel init thread\n");
    make_kernel_thread("kernel:init1", init_stage1);

    sched_pick_next();
    scheduling_enabled = 1;
    kprintf("[[ starting multithreading mode ]]\n");
    // interrupts are enabled in last switch_to_userspace instruction
    switch_to_userspace(current_proc);

    kprintf("\n\033[91mkernel halted.\033[97m\n");
}

struct semaphore sleep;

void __attribute__((noreturn)) init_stage1() {
    // Continue kernel boot in threaded environment
    log("mounting devices in vfs");
    kbd_vfs_init();
    tty_mnt_vfs();

    log("registering system threads");
    sd_register_thread();

    log("mounting SD card TAR filesystem");
    tar_make_dir_tree();
    tar_mount_sd();

    log("%uB kernel heap, %ukB paged memory free", mem_free_size(), 0);
    log("init stage 1 done");

    semaphore_init(&sleep);
    semaphore_down(&sleep);
    for(;;); // FIXME: remove thread here
}
