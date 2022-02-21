#ifndef LIBK_SEMAPHORE_H
#define LIBK_SEMAPHORE_H

struct semaphore {
    volatile int count;
};

void semaphore_init(struct semaphore* s);
void semaphore_up(struct semaphore* s);
void semaphore_down(struct semaphore* s);

#endif
