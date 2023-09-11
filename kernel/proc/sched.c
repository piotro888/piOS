#include "sched.h"

#include <string.h>
#include <libk/kmalloc.h>
#include <libk/list.h>
#include <libk/assert.h>
#include <irq/timer.h>
#include <proc/signal.h>
#include <proc/virtual.h>

/*
 * Very simple scheduler, just to keep going on threads
 * Use pick_next to set globally accessible current proc
 */

struct proc* current_proc;

struct list proc_list;
struct list_node* last_element = NULL;

int scheduling_enabled = 0;
int int_no_proc_modify = 0;

int pid_now = 0;

// FIXME: Request pages from virtual memory manager
int first_free_page = 0x211; // skip devices region!
int first_free_prog_page = 0x11; // in prog page - MSB high (0x811 in practice)

extern void __attribute__((noreturn)) idle_task();
struct proc idle_struct;

void scheduler_init() {
    list_init(&proc_list);
    scheduling_enabled = 0;
    pid_now = 0;

    // init idle task
    idle_struct.pid = 0;
    idle_struct.state = PROC_STATE_RUNNABLE;
    idle_struct.type = PROC_TYPE_INIT;
    strcpy(idle_struct.name, "idle");
    idle_struct.proc_state.pc = (int)idle_task+1;
    for(int i=0; i<16; i++) {
        idle_struct.proc_state.prog_pages[i] = i;
        idle_struct.proc_state.mem_pages[i] = ILLEGAL_PAGE;
    }
}

/* Set current_proc to next process */
void sched_pick_next() {
    ASSERT(proc_list.first != NULL);

    struct list_node* first_element = last_element;
    for(;;) {
        // if reached end of list return to beginning
        if (last_element == NULL || last_element->next == NULL)
            last_element = proc_list.first;
        else
            last_element = last_element->next;

        // check if valid to run
        struct proc* lproc = (struct proc*)last_element->val;

        if(lproc->alarm_ticks && (unsigned int)lproc->alarm_ticks <= (unsigned int)sys_ticks) {
            struct signal alarm_sig = {SIG_TYPE_ALARM, (unsigned int) lproc->alarm_ticks};
            signal_send(lproc, &alarm_sig);
            lproc->alarm_ticks = 0;
        }

        // this process is valid to run
        if(lproc->state == PROC_STATE_RUNNABLE || lproc->state == PROC_STATE_SYSCALL)
            break;

        if(lproc->state == PROC_STATE_BLOCKED || lproc->state == PROC_STATE_SYSCALL_BLOCKED) {
            // check if blocked process is unblocked now
            if(lproc->sema_blocked && lproc->sema_blocked->count > 0) {
                lproc->state = lproc->state == PROC_STATE_BLOCKED ? PROC_STATE_RUNNABLE : PROC_STATE_SYSCALL;
                lproc->sema_blocked = NULL;
                break;
            }
        }

        if(last_element == first_element) {
            // if no process is currently available, set kernel idle task to allow device irq
            current_proc = &idle_struct;
            return;
        }
    }

    current_proc = ((struct proc*) last_element->val);
    
    #ifdef DEBUG
        kprintf("sched pick %s\n", current_proc->name);
        kprintf("pc=%x ppage0=>%x stack=%x\n", current_proc->proc_state.pc, current_proc->proc_state.prog_pages[0], current_proc->proc_state.mem_pages[15]);
    #endif
}

/* Prepare new kernel thread object and return its pid */
int make_kernel_thread(char* name, void __attribute__((noreturn)) (*entry)()) {
    struct proc* p = kmalloc(sizeof(struct proc));

    p->pid = ++pid_now;
    p->type = PROC_TYPE_KERNEL;

    for(int i=0; i<8; i++)
        p->proc_state.regs[i] = 0;
    p->proc_state.arith_flags = 0;

    // set virtual pages to kernel mapping
    for(int i=0; i<16; i++) {
        p->proc_state.mem_pages[i] = 0x200 + i;
        p->proc_state.prog_pages[i] = i;
    }
    p->proc_state.mem_pages[0] = 0; // illegal page
    // set stack page to new page (thread)
    p->proc_state.mem_pages[15] = first_free_page++;

    // setup sp and fp (fe - aligned to 2)
    p->proc_state.regs[7] = 0xfffe;
    p->proc_state.regs[5] = 0xfffe;

    p->proc_state.pc = (int)entry;

    // reset blocked status
    p->sema_blocked = NULL;

    // initialize fd table as free
    for(int i=0; i<PROC_MAX_FILES; i++)
        p->open_files[i].inode = NULL;

    // thread is ready to execute now
    p->state = PROC_STATE_RUNNABLE;

    list_init(&p->signal_queue); // not supported
    semaphore_init(&p->signal_sema);
    p->sighandler = NULL;

    strcpy(p->name, name);

    list_append(&proc_list, p);

    return p->pid;
}

struct proc* sched_init_user_thread() {
    struct proc* p = kmalloc(sizeof(struct proc));

    p->pid = ++pid_now;
    p->type = PROC_TYPE_USER;

    for(int i=0; i<8; i++)
        p->proc_state.regs[i] = 0;
    p->proc_state.arith_flags = 0;
    p->proc_state.pc = 0;

    // set invalid virtual pages
    for(int i=0; i<16; i++) {
        p->proc_state.mem_pages[i] = 0xff; //emm?
        p->proc_state.prog_pages[i] = 0xff;
    }

    // allocate stack page
    p->proc_state.mem_pages[15] = first_free_page++;
    p->kernel_stack_page = first_free_page++;

    // initialize fd table as free
    for(int i=0; i<PROC_MAX_FILES; i++)
        p->open_files[i].inode = NULL;
    p->sema_blocked = NULL;

    p->state = PROC_STATE_UNLOADED;
    p->name[0] = '\0';

    list_init(&p->signal_queue);
    semaphore_init(&p->signal_sema);
    p->signals_hold = 0;

    list_append(&proc_list, p);
    return p;
}

struct proc* proc_by_pid(int pid) {
    list_foreach(&proc_list) {
        struct proc* p = LIST_FOREACH_VAL(struct proc*);
        if(p->pid == pid)
            return p;
    }
    return NULL;
}
