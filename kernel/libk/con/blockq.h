#ifndef LIBK_BLOCKQ_H
#define LIBK_BLOCKQ_H

#include <libk/con/semaphore.h>
#include <libk/con/spinlock.h>
#include <libk/ringbuff.h>
#include <libk/types.h>

struct blockq {
    struct ringbuff rbuff;
    struct semaphore not_empty, not_full;
    size_t size, max_size, base_size;
    struct spinlock read_lock, write_lock;
};

void blockq_init(struct blockq* s, size_t size, size_t base_size);
void blockq_push(struct blockq* s, void* buff);
void blockq_pop(struct blockq* s, void* buff);
void blockq_push_nonblock(struct blockq* s, void* buff);

#endif
