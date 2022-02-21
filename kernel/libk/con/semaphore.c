#include "semaphore.h"
#include "libk/log.h"

#include <libk/atomic.h>
#include <libk/assert.h>
#include <irq/interrupt.h>
#include <proc/sched.h>

void semaphore_init(struct semaphore* s) {
    atomic_write_int(&s->count, 0);
}

void semaphore_up(struct semaphore* s) {
    atomic_add_int(&s->count, 1);
}

void semaphore_down(struct semaphore* s) {
    int_disable();

    int count = atomic_read_int(&s->count);

    ASSERT(count >= 0);

    if(count == 0) {
        current_proc->state = PROC_STATE_BLOCKED;
        current_proc->sema_blocked = s;

        int itc = 0;
        do {
            // yield needs interrupts enabled
            int_enable();

            // yield in blocked state to sleep
            YIELD();

            // we return from sleep only if semaphore is ready to be acquired,
            // but we can't guarantee that context switch didn't happen before int_disable,
            // therefore confirmation of semaphore still >0 is needed
            int_disable();
        } while(atomic_read_int(&s->count) == 0);

        // interrupts disabled, count > 0
    }

    atomic_add_int(&s->count, -1);
    int_enable();
}
