#ifndef LIBK_RINGBUFF_H
#define LIBK_RINGBUFF_H

#include <libk/types.h>

struct ringbuff {
    size_t head;
    size_t tail;
    size_t size;
    u8* buff;
};

void ringbuff_init(struct ringbuff* rb, size_t size);
size_t ringbuff_length(struct ringbuff* rb);
size_t ringbuff_read(struct ringbuff* rb, u8* buff, size_t n);
size_t ringbuff_write(struct ringbuff* rb, u8* buff, size_t n);
void ringbuff_force_write(struct ringbuff* rb, u8* buff, size_t n);

/* NOTE: To safely read/write ringbuffer in multiple processes, using read_spinlock and write_spinlock is necessary
 * Using semaphore is recommended to signal data present on buffer and allow sleeping while waiting for data
 */

#endif
