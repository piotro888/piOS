#include "spinlock.h"
#include <libk/atomic.h>
#include <proc/sched.h>

void spinlock_init(struct spinlock* sl) {
    sl->locked  = 0;
    sl->owner = -1;
}

int spinlock_lock(struct spinlock* sl) {
    while(!atomic_compare_and_swap_int(&sl->locked, 0, 1)) {
        YIELD(); // on single core CPU we can yield to save runtime
    }

    sl->owner = current_proc->pid;
    return 0;
}

int spinlock_unlock(struct spinlock* sl) {
    if(current_proc->pid != sl->owner || !sl->locked)
        return -1;

    sl->locked = 0;

    return 0;
}
