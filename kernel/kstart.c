/* 
 * piOS kernel entry point
 * (C) 2021-2022 by Piotr Wegrzyn
*/

#include <driver/tty.h>
#include <irq/interrupt.h>
#include <libk/kmalloc.h>
#include <libk/kprintf.h>
#include <libk/log.h>
#include <libk/assert.h>
#include <libk/con/semaphore.h>
#include <irq/timer.h>
#include <fs/tar.h>
#include <driver/sd.h>
#include <art.h>
#include <proc/virtual.h>
#include <proc/sched.h>


/* kthreads testing example */

__attribute__((noreturn)) void test_kthread() {
    tty_putc('z');
    for(;;) {
       int_disable();
       log("from thread");
       int_enable();
     //   asm volatile ("sys");
        for(int i=0; i<10000; i++){}
    }
}

int cntr = 0, cntr2 = 0;
__attribute__((noreturn)) void test_kthread2() {
    tty_putc('x');
    *((int*)0xf005) = 888;

    for(int z=0; z<100; z++) {
        cntr = 0; cntr2 = 0;
        for (int i = 0; i < 30000; i++) {
            cntr++, cntr2++;
//              ASSERT(*((volatile int*)0xf008) != 854);
//            int_disable();
//            int r = sr_get();
//            if(r != 0b1001) {
//                log("failed %d", r);
//                ASSERT_NOT_REACHED();
//                ASSERT_NOT_REACHED();
//                ASSERT_ NOT_REACHED();
//            }
//            *((int*)0xf005) = 888; // working fine
//            int_enable();
//            *((int*)0xf005) = 888; // crash
        }

        log("cnt: %u %u %x", cntr, cntr2, current_proc->regs[7]);
        ASSERT(cntr == 30000);
    }
    for(;;) {
        log("thread2!");
    }
}

int shactr = 0;
struct semaphore sha_sem;
struct semaphore cnt1_end;
struct semaphore cnt2_end;

__attribute__((noreturn)) void test_kthread3d0() {
    for(int i=0; i<10000; i++) {
//        atomic_add_int(&shactr, 1);
        semaphore_down(&sha_sem);
        shactr++;
        semaphore_up(&sha_sem);
    }

    log("shactr1 ended: %d", shactr);
    semaphore_up(&cnt1_end);
    semaphore_down(&cnt2_end);
    log("2end");

    for(;;);
}

__attribute__((noreturn)) void test_kthread3d1() {
    for(int i=0; i<10000; i++) {
        semaphore_down(&sha_sem);
        shactr++;
        semaphore_up(&sha_sem);
    }

    log("shactr2 ended: %d", shactr);

    semaphore_up(&cnt2_end);
    semaphore_down(&cnt1_end);
    log("2end");

    for(;;);
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
    kprintf("init done.\n");
    kprintf(BOOT_ART);
    //tar_make_dir_tree();

    //make_kernel_thread("tt", test_kthread);
    //make_kernel_thread("tt2", test_kthread2);
    //make_kernel_thread("tt2", test_kthread3);

    // SEMAPHORES!
    semaphore_init(&sha_sem);
    semaphore_up(&sha_sem);
    semaphore_init(&cnt1_end);
    semaphore_init(&cnt2_end);
    make_kernel_thread("1", test_kthread3d0);
    make_kernel_thread("2", test_kthread3d1);
    sched_pick_next();
    scheduling_enabled = 1;

    kprintf("[[ starting multithreading mode ]]\n");
    // interrupts are enabled in last switch_to_userspace instruction
    switch_to_userspace(current_proc);

    tty_set_color(0x09);
    kprintf("\nkernel halted.\n");
    tty_set_color(0x07);
}
