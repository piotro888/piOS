#include "sysd.h"

#include <driver/tty.h>
#include <proc/sched.h>
#include <proc/virtual.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

#include <libk/con/blockq.h>
#include <libk/log.h>
#include <libk/kmalloc.h>

struct blockq syscall_q;

void async_finished_callback(struct vfs_async_req_t* req);

int process_syscall(struct proc* proc) {
    int sysno = proc->regs[0];
    switch (sysno) {
        case SYS_DUMP: {
            log("PID: %d DUMP r0:0x%x r1:0x%x r2:0x%x r3:0x%x r4:0x%x r5:0x%x r6:0x%x r7:0x%x vpc:0x%x",
                proc->pid, proc->regs[0], proc->regs[1], proc->regs[2], proc->regs[3], proc->regs[4], proc->regs[5],
                proc->regs[6], proc->regs[7], proc->pc);
            break;
        }
        case SYS_PRINT: {
            char* pb = kmalloc(proc->regs[2]);
            memcpy_from_userspace(pb, proc, proc->regs[1], proc->regs[2]);
            log(pb);
            kfree(pb);
            break;
        }
        case SYS_OPEN: {
            char* path = kmalloc(sizeof proc->regs[2]);
            memcpy_from_userspace(path, proc, proc->regs[1], proc->regs[2]);
            int r = vfs_open(path);
            kfree(path);
            proc->regs[0] = r;
            break;
        }
        case SYS_CLOSE: {
            int r = vfs_close(proc->regs[1]);
            proc->regs[0] = r;
            break;
        }
        case SYS_READ: {
            int flags = vfs_get_vnode_flags(proc->regs[1]);
            if (flags < 0) {
                proc->regs[0] = flags;
                return 1;
            }
            
            void* ks_buff = NULL;
            if (flags & VFS_REG_FLAG_KERNEL_BUFFER_ONLY) {
                ks_buff = kmalloc(sizeof proc->regs[3]);
            }

            ssize_t r = vfs_read_async(proc->regs[1], 
                ks_buff ? 0 : proc->pid, 
                ks_buff ? ks_buff : (void*)proc->regs[2], 
                proc->regs[3],
                async_finished_callback,
                proc->pid
            );

            if (r < 0) {
                if (ks_buff)
                    kfree(ks_buff);
                proc->regs[0] = r;
                return 1;
            }
            return 0;
        }
        case SYS_WRITE: {
            int flags = vfs_get_vnode_flags(proc->regs[1]);
            if (flags < 0) {
                proc->regs[0] = flags;
                return 1;
            }
            
            void* ks_buff = NULL;
            if (flags & VFS_REG_FLAG_KERNEL_BUFFER_ONLY) {
                // TODO: validate size
                ks_buff = kmalloc(sizeof proc->regs[3]);
                memcpy_from_userspace(ks_buff, proc, proc->regs[2], proc->regs[3]);
            }

            ssize_t r = vfs_write_async(proc->regs[1], 
                ks_buff ? 0 : proc->pid, 
                ks_buff ? ks_buff : (void*)proc->regs[2], 
                proc->regs[3],
                async_finished_callback,
                proc->pid
            );

            if (r < 0) {
                if (ks_buff)
                    kfree(ks_buff);
                proc->regs[0] = r;
                // req not allocated
                return 1;
            }
            return 0;
        }
        default: {
            log("PID: %d Illegal syscall (%d)", proc->pid, sysno);
            proc->regs[0] = -EINVAL;
            break;
        }
    }
    return 1;
}

void async_finished_callback(struct vfs_async_req_t* req) {
    // we have only blocking syscalls for now, so that's easy to identify
    struct proc* proc = proc_by_pid(req->req_id);
    proc->regs[0] = req->res;

    log_dbg("PID: %d syscall callback: %x", req->req_id, req->res);

    if (req->type == VFS_ASYNC_TYPE_READ && req->res && !req->pid)
        memcpy_to_userspace(proc, proc->regs[2], req->vbuff, req->res);

    if (!req->pid) // kernel allocated buffer
        kfree(req->vbuff);

    kfree(req);
    proc->state = PROC_STATE_RUNNABLE;
}

__attribute__((noreturn)) void syscall_dispatcher() {
    for(;;) {
        int proc_pid;
        blockq_pop(&syscall_q, &proc_pid);
        struct proc* proc = proc_by_pid(proc_pid);

        int sys_ret = process_syscall(proc);
        if(sys_ret) {
            log_dbg("PID: %d resuming from syscall (ret %d)", proc_pid, proc->regs[0]);
            proc->state = PROC_STATE_RUNNABLE;
        }
    }
}

int sysd_submit(int pid) {
    if(syscall_q.size == syscall_q.max_size) {
        log_irq("[E] SYSQ is full. Dropping syscall");
        return -1;
    }

    blockq_push_nonblock(&syscall_q, &pid);
    current_proc->state = PROC_STATE_SYSCALL;
    return 0;
}

int sysd_resubmit(int pid) {
    blockq_push(&syscall_q, &pid);
    return 0;
}

void sysd_init() {
    blockq_init(&syscall_q, 128, sizeof(int));
    make_kernel_thread("sysd", syscall_dispatcher);
}
