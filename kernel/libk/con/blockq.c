#include "blockq.h"

#include <libk/log.h>
#include <libk/assert.h>

void blockq_init(struct blockq* s, size_t size, size_t base_size) {
    s->max_size = size;
    s->base_size = base_size;

    ringbuff_init(&s->rbuff, s->max_size*s->base_size);

    semaphore_init(&s->not_empty);
    semaphore_init(&s->not_full);
    s->not_full.count = s->max_size;

    spinlock_init(&s->read_lock);
    spinlock_init(&s->write_lock);

    s->size = 0;
}

void blockq_pop(struct blockq* s, void* buff) {
    semaphore_down(&s->not_empty);

    spinlock_lock(&s->read_lock);
    ASSERT(s->size > 0);
    size_t rb = ringbuff_read(&s->rbuff, buff, s->base_size);
    ASSERT(rb == s->base_size);
    s->size--;
    spinlock_unlock(&s->read_lock);

    semaphore_up(&s->not_full);
}

void blockq_push(struct blockq* s, void* buff) {
    if(s->size == s->max_size)
        log("blocking queue is full");

    semaphore_down(&s->not_full);

    spinlock_lock(&s->write_lock);
    size_t wb = ringbuff_write(&s->rbuff, buff, s->base_size);
    ASSERT(wb == s->base_size);
    s->size++;
    ASSERT(s->size <= s->max_size);
    spinlock_unlock(&s->write_lock);

    semaphore_up(&s->not_empty);
}

void blockq_push_nonblock(struct blockq* s, void* buff) {
    ASSERT(s->size < s->max_size);
    ringbuff_write(&s->rbuff, buff, s->base_size);
    s->size++;
    semaphore_up(&s->not_empty);
}
