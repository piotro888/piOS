#include "ringbuff.h"
#include <libk/kmalloc.h>

void ringbuff_init(struct ringbuff* rb, size_t size) {
    size++; // ringbuff always keeps at least one free index
    rb->head = 0;
    rb->tail = 0;
    rb->size = size;
    rb->buff = kmalloc(size);
}

size_t ringbuff_length(struct ringbuff* rb) {
    return (rb->head >= rb->tail ? rb->head - rb->tail : rb->head + rb->size-rb->tail);
}

size_t ringbuff_write(struct ringbuff* rb, u8* buff, size_t n) {
    size_t wlen;
    for(wlen=0; wlen<n; wlen++) {
        if(rb->head == rb->tail-1 || (rb->tail == 0 && rb->head == rb->size-1))
            break;

        rb->buff[rb->head] = *buff++;
        if (++rb->head == rb->size)
            rb->head = 0;
    }
    return wlen;
}

size_t ringbuff_read(struct ringbuff* rb, u8* buff, size_t n) {
    size_t rlen;
    for(rlen = 0; rlen < n; rlen++) {
        if (rb->tail == rb->head)
            break;
        *buff++ = rb->buff[rb->tail];
        if (++rb->tail == rb->size)
            rb->tail= 0;
    }
    return rlen;
}

/* Write even if no space is available. It causes reset of buffer on buffer overflow,
 * but this is only way to do this with changing moving tail pointer and keeping thread safety.
 */
void ringbuff_force_write(struct ringbuff* rb, u8* buff, size_t n) {
    for(size_t i=0; i < n; i++) {
        rb->buff[rb->head] = *buff++;
        if (++rb->head == rb->size)
            rb->head = 0;
    }
}
