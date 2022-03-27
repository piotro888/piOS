#ifndef LIBK_SPINLOCK_H
#define LIBK_SPINLOCK_H

struct spinlock {
    volatile int locked;
    volatile int owner;
};

void spinlock_init(struct spinlock* sl);
int  spinlock_lock(struct spinlock* sl);
int  spinlock_unlock(struct spinlock* sl);

#endif
