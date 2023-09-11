#include "sysd.h"

#include <sys/systructs.h>

#include <driver/tty.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/virtual.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>
#include <irq/timer.h>

#include <libk/kmalloc.h>
#include <libk/assert.h>
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

            if (ks_buff) {
                if (r > 0)
                    memcpy_to_userspace(current_proc, state->regs[2], ks_buff, r);
                kfree(ks_buff);
            }

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
        case SYS_FCNTL: {
            if (!validate_fd(state->regs[1])) {
                state->regs[0] = -EBADFD;
                break;
            }
            struct proc_file* file = &current_proc->open_files[state->regs[1]];

            if(state->regs[2] != (unsigned) -1) {
                file->fcntl_flags = state->regs[2];
                log("fcntl %x", state->regs[2]);
            }

            state->regs[0] = file->fcntl_flags;
            break;
        }
        case SYS_PROCINFO: {
            struct sys_proc_info proc_info;
            
            struct proc* proc = current_proc;
            if (state->regs[1])
                proc = proc_by_pid(state->regs[1]);
            if (!proc) {
                state->regs[0] = -ENOTFOUND;
                break;
            }

            proc_info.pid = proc->pid;
            proc_info.type = proc->type;
            proc_info.state = proc->state;
            proc_info.mem_pages_mapped = 0;
            proc_info.prog_pages_mapped = 0;
            for (int i=0; i<16; i++) {
                if (proc->syscall_state.mem_pages[i] && proc->syscall_state.mem_pages[i] != 0xff)
                    proc_info.mem_pages_mapped |= (1u<<i);
                if (proc->syscall_state.prog_pages[i])
                    proc_info.prog_pages_mapped |= (1u<<i);
            }
            proc_info.load_brk = proc->load_brk;
            
            memcpy_to_userspace(current_proc, state->regs[2], &proc_info, sizeof(proc_info));

            state->regs[0] = 0;
            break;
        }
        case SYS_PGMAP: {
            if (state->regs[1] < 0 || state->regs[1] >= 16) {
                state->regs[0] = -EINVAL;
                break;
            }
            if(current_proc->syscall_state.mem_pages[state->regs[1]] != 0xff && current_proc->syscall_state.mem_pages[state->regs[1]] != 0) {
                state->regs[0] = -ENOTFOUND;
                break;
            }

            // TODO: thread safety on first free page
            current_proc->syscall_state.mem_pages[state->regs[1]] = first_free_page++;
            state->regs[0] = 0;
            break;
        }
        case SYS_SIGACTION: {
            state->regs[0] = (unsigned) current_proc->sighandler;
            current_proc->sighandler = (void*)state->regs[1];
            break;
        }
        case SYS_SIGSEND: {
            struct proc* target = current_proc;
            if ((int) state->regs[1] >= 0) {
                target = proc_by_pid(state->regs[1]);
                if (!target) {
                    state->regs[0] = -EINVAL;
                    break;
                }
            } 
            // todo: queue lock
            struct signal signal = {
                state->regs[2],
                state->regs[3]
            };
            signal_send(target, &signal);
            break;
        }
        case SYS_SIGHDLRRET: {
            signal_handler_return(current_proc);
            break;
        }
        case SYS_SIGWAIT: {
            if (current_proc->sighandler && !current_proc->signals_hold) {
                state->regs[0] = -EINVAL;
                break;
            }
            semaphore_down(&current_proc->signal_sema);
            ASSERT(current_proc->signal_queue.first);
            memcpy_to_userspace(current_proc, state->regs[1], current_proc->signal_queue.first->val, sizeof(struct signal));
            kfree(current_proc->signal_queue.first->val);
            list_remove(&current_proc->signal_queue, current_proc->signal_queue.first);
            state->regs[0] = 0;
            break;
        }
        case SYS_CLOCKTICKS: {
            // TODO: atomic, or check if high changed in this case
            state->regs[0] = (unsigned int) sys_ticks;
            state->regs[1] = (unsigned int) (sys_ticks << 16u);
            break;
        }
        case SYS_ALARMSET: {
            unsigned long alarm_val = ((unsigned long) state->regs[2] << 16u) | (unsigned long) state->regs[1];
            // TODO: atomic set
            current_proc->alarm_ticks = alarm_val;
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
