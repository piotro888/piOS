#include "sysres.h"

#include <proc/sched.h>
#include <sys/sysd.h>

#include <libk/con/semaphore.h>
#include <libk/list.h>
#include <libk/log.h>
#include <panic.h>

struct semaphore sysres_sig;
struct list sysres_blocked_read, sysres_blocked_write;

// How it works
// * sysd executes vfs_read_nonblock
// * File is empty
// * Turn on sysres_notify flag on file
// * Submit waiting syscall via sysres_submit_read(pid)
// * sysres puts it to list
// === SLEEP ===
// * file write from different proc
// * file notifies sysres and resets sysres_notif
// * (option 1 file leaves its id (requires queue or list it) or (opt1 1.2 process has volatile bool fakse_sema)
// * sysrei thread checks syscalls (opt2 call vfs_is_unblocked()) or (opt1 checks if req file in queue)
// * sysrei pushes unblocked syscalls

void reissue_read() {
    list_foreach(&sysres_blocked_read) {
        struct proc* sys_p = LIST_FOREACH_VAL(struct proc*);
        if(vfs_read_nonblock(sys_p->regs[0], NULL, 0) != -EWOULDBLOCK) {
            log("reissue_read %d", sys_p->pid);
            sysd_resubmit(sys_p->pid);
            list_remove(&sysres_blocked_read, LIST_FOREACH_NODE);
        }
    }
}

void reissue_write() {
    list_foreach(&sysres_blocked_write) {
        struct proc* sys_p = LIST_FOREACH_VAL(struct proc*);
        if(vfs_write_nonblock(sys_p->regs[0], NULL, 0) != -EWOULDBLOCK) {
            log("reissue_write %d", sys_p->pid);
            sysd_resubmit(sys_p->pid);
            list_remove(&sysres_blocked_read, LIST_FOREACH_NODE);
        }
    }
}

__attribute__((noreturn)) void syscall_reissuer() {
    for(;;) {
        semaphore_down(&sysres_sig);
        log("wakeup");
        reissue_read();
        reissue_write();
    }
}

void sysres_notify() {
    semaphore_binary_up(&sysres_sig);
}

void sysres_submit_read(struct proc* proc) {
    // LATER_FIXME: Clear killed processes from list
    log("sysres_submit_read %d", proc->pid);
    list_append(&sysres_blocked_read, proc);
}

void sysres_submit_write(struct proc* proc) {
    log("sysres_submit_write %d", proc->pid);
    list_append(&sysres_blocked_write, proc);
}

void sysres_init() {
    semaphore_init(&sysres_sig);
    list_init(&sysres_blocked_read);
    list_init(&sysres_blocked_write);
    make_kernel_thread("sysres", syscall_reissuer);
}
