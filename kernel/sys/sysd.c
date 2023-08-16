#include "sysd.h"

#include <driver/tty.h>
#include <proc/sched.h>
#include <proc/virtual.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

#include <libk/con/blockq.h>
#include <libk/log.h>
#include <libk/kmalloc.h>

int process_syscall(struct proc_state* state) {
    int sysno = state->regs[0];
    switch (sysno) {
        case SYS_DUMP: {
            log("PID: %d DUMP r0:0x%x r1:0x%x r2:0x%x r3:0x%x r4:0x%x r5:0x%x r6:0x%x r7:0x%x vpc:0x%x",
                current_proc->pid, state->regs[0], state->regs[1], state->regs[2], state->regs[3], state->regs[4], state->regs[5],
                state->regs[6], state->regs[7], state->pc);
            break;
        }
        case SYS_PRINT: {
            char* pb = kmalloc(state->regs[2]+1);
            memcpy_from_userspace(pb, current_proc, state->regs[1], state->regs[2]+1);
            log(pb);
            kfree(pb);
            break;
        }
        case SYS_OPEN: {
            int fd = proc_free_fd(current_proc);
            if (fd < 0) {
                state->regs[0] = fd;
                break;
            }

            char* path = kmalloc(state->regs[2]);
            // ehhh we need userspace struct here
            memcpy_from_userspace(path, current_proc, state->regs[1], state->regs[2]);
            struct inode* inode = vfs_find_inode(path);
            kfree(path);
            
            int rc = vfs_open(inode, &current_proc->open_files[fd]);

            if (rc < 0) {
                state->regs[0] = rc;
                break;
            }

            state->regs[0] = fd;
            break;
        }
        case SYS_CLOSE: {
            if (!validate_fd(state->regs[1])) {
                state->regs[0] = -EBADFD;
                break;
            }
            
            int r = vfs_close(&current_proc->open_files[state->regs[1]]);

            state->regs[0] = r;
            break;
        }
        case SYS_READ: {
            if (!validate_fd(state->regs[1])) {
                state->regs[0] = -EBADFD;
                break;
            }
                
            struct proc_file* file = &current_proc->open_files[state->regs[1]];
            
            int flags = file->inode->vnode->reg->flags;
            
            void* ks_buff = NULL;
            if (flags & VFS_REG_FLAG_KERNEL_BUFFER_ONLY) {
                ks_buff = kmalloc(state->regs[3]);
            }

            ssize_t r = vfs_read_blocking(file, 
                ks_buff ? 0 : current_proc->pid, 
                ks_buff ? ks_buff : (void*)state->regs[2], 
                state->regs[3]
            );

            if (ks_buff)
                kfree(ks_buff);
            state->regs[0] = r;
            break;
        }
        case SYS_WRITE: {
            if (!validate_fd(state->regs[1])) {
                state->regs[0] = -EBADFD;
                break;
            }
            struct proc_file* file = &current_proc->open_files[state->regs[1]];

            int flags = file->inode->vnode->reg->flags;
            
            void* ks_buff = NULL;
            if (flags & VFS_REG_FLAG_KERNEL_BUFFER_ONLY) {
                // TODO: validate size
                ks_buff = kmalloc(state->regs[3]);
                memcpy_from_userspace(ks_buff, current_proc, state->regs[2], state->regs[3]);
            }

            ssize_t r = vfs_write_blocking(file, 
                ks_buff ? 0 : current_proc->pid, 
                ks_buff ? ks_buff : (void*)state->regs[2], 
                state->regs[3]
            );

            if (ks_buff)
                kfree(ks_buff);
            state->regs[0] = r;
            // req not allocated
            break;
        }
        default: {
            log("PID: %d Illegal syscall (%d)", current_proc->pid, sysno);
            state->regs[0] = -EINVAL;
            break;
        }
    }
    return 1;
}
