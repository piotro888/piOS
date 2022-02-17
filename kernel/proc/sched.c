#include "sched.h"

#include <libk/kmalloc.h>
#include <libk/log.h>
#include <libk/string.h>
#include <libk/list.h>
#include <libk/assert.h>

/*
 * Very simple scheduler, just to keep going on threads
 * Use pick_next to set globally accessible current proc
 */

struct proc* current_proc;

struct list proc_list;
struct list_node* last_element = NULL;

int scheduling_enabled = 0;

int pid_now = 0;

// FIXME: Request pages from virtual memory manager
int first_free_page = 17;

void scheduler_init() {
    list_init(&proc_list);
}

/* Set current_proc to next process */
void sched_pick_next() {
    ASSERT(proc_list.first != NULL);

    // if reached end of list return to beginning
    if(last_element == NULL || last_element->next == NULL)
        last_element = proc_list.first;
    else
        last_element = last_element->next;

    current_proc = ((struct proc*) last_element->val);
}

/* Prepare new kernel thread object and return its pid */
int make_kernel_thread(char* name, void (*entry)()) {
    struct proc* p = kmalloc(sizeof(struct proc));

    p->pid = ++pid_now;
    p->type = PROC_TYPE_KERNEL;

    for(int i=0; i<8; i++)
        p->regs[i] = 0;
    p->arith_flags = 0;

    // set virtual pages to kernel mapping
    for(int i=0; i<16; i++) {
        p->mem_pages[i] = i;
        p->prog_pages[i] = i;
    }

    // set stack page to new page (thread)
    p->mem_pages[15] = first_free_page++;
    // setup sp and fp
    p->regs[7] = 0xffff;
    p->regs[5] = 0xffff;

    /* assembly BUG to resolve */
    p->pc = (int)entry+1;

    // thread is ready to execute now
    p->state = PROC_STATE_LOADED;

    strcpy(p->name, name);

    list_append(&proc_list, p);

    return p->pid;
}
