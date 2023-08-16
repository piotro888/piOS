/* 
 * piOS kernel entry point
 * (C) 2021-2023 by Piotr Wegrzyn
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <art.h>
#include <driver/sd.h>
#include <driver/spi.h>
#include <driver/tty.h>
#include <fs/kbd.h>
#include <fs/sio.h>
#include <fs/tar.h>
#include <fs/rootfs.h>
#include <fs/vfs.h>
#include <irq/timer.h>
#include <proc/elf.h>
#include <proc/sched.h>
#include <proc/virtual.h>

#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <libk/log.h>
#include <libk/assert.h>

void __attribute__((noreturn)) init_stage1();

/* C entry point for kernel */
__attribute__((used))
void _kstart() {
    enable_default_memory_paging();

    log_set_target(LOG_TARGET_SERIAL, 1);
    //tty_init_basic();
    //log_set_target(LOG_TARGET_TTY, 1);

    log_early_puts("\033[92mpios\033[97m");

    log_early_puts(" kernel booting\n");
    log_early_putc('\n');
    kprintf("initializing kernel heap\n");
    init_malloc();
    kprintf("%uB heap available\n", mem_free_size());
    kprintf("initializing vfs\n");
    vfs_init();
    kprintf("initializing scheduler\n");
    timer_init();
    scheduler_init();
    kprintf("initializing devices\n");
    spi_init();
    sd_init();
    kprintf("initializing full tty driver\n");
    tty_init_basic();
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
    log("creating rootfs");
    struct vnode* rootfs = vfs_mount(rootfs_get_vfs_reg(), vfs_get_root_inode());
    rootfs_create(rootfs);

    log("registering system threads");
    sd_register_thread();

    log("mounting SD card TAR filesystem");
    struct vnode* tarfs = vfs_mount(tar_get_vfs_reg(), rootfs_create_entry(rootfs, "", "sd/", INODE_TYPE_DIRECTORY));
    tar_init(tarfs);

    log("%uB kernel heap, %ukB paged memory free", mem_free_size(), 0);
    log("init stage 1 done");

    struct inode* load_inode = vfs_find_inode("/sd/bin/libe");
    ASSERT(load_inode);
    struct proc_file* elf_file = &current_proc->open_files[proc_free_fd(current_proc)];
    vfs_open(load_inode, elf_file);
    log("loading ELF %s", elf_file->inode->name);
    elf_load(elf_file);
    vfs_close(elf_file);

    semaphore_init(&sleep);
    semaphore_down(&sleep);
    for(;;); // FIXME: remove thread here
}
