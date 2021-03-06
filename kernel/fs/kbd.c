#include "kbd.h"
#include <fs/vfs.h>
#include <sys/sysres.h>
#include <libk/types.h>
#include <libk/con/spinlock.h>
#include <libk/con/semaphore.h>
#include <libk/ringbuff.h>
#include <libk/assert.h>

/* Temporary vfs device hooked up
 * to PS/2 keyboard driver.
 * Only for testing purposes of vfs 
 */

#define BUFF_SIZE 16
struct ringbuff c_buff;

static struct spinlock read_spinlock;
static struct semaphore data_available;
static int read_notify = 0;

int kbd_get_fid(char* path) {
    return 0; // we have only one file
}

ssize_t kbd_read(struct fd_info* file, void* buff, size_t len) {
    spinlock_lock(&read_spinlock);
    while(!ringbuff_length(&c_buff)) {
        spinlock_unlock(&read_spinlock);
        semaphore_down(&data_available); // sleep until some data is in the buffer
        spinlock_lock(&read_spinlock); // lock reading (and tail/length modification) to one process
    }

    size_t size = ringbuff_read(&c_buff, buff, len);
    ASSERT(size != 0); // spinlock fail?

    if(ringbuff_length(&c_buff))
        semaphore_binary_up(&data_available); // data is still in buffer, wake up other process

    spinlock_unlock(&read_spinlock);

    return size;
}

ssize_t kbd_read_nonblock(struct fd_info* file, void* buff, size_t len) {
    spinlock_lock(&read_spinlock);
    if(!ringbuff_length(&c_buff)) {
        read_notify = 1; // FIXME_LATER: ioctl
        spinlock_unlock(&read_spinlock);
        return -EWOULDBLOCK;
    }

    size_t size = ringbuff_read(&c_buff, buff, len);
    ASSERT(size != 0);

    if(ringbuff_length(&c_buff)) {
        semaphore_binary_up(&data_available);
        if(read_notify) {
            read_notify = 0;
            sysres_notify();
        }
    }

    spinlock_unlock(&read_spinlock);

    return size;
}

void kbd_vfs_submit_char(char c) {
    ringbuff_force_write(&c_buff, &c, 1);

    semaphore_binary_up(&data_available);
    if(read_notify) {
        read_notify = 0;
        sysres_notify();
    }
}

void kbd_vfs_init() {
    const struct vfs_reg kbd_vfs_reg = {
            kbd_get_fid,
            kbd_read,
            NULL,
            kbd_read_nonblock,
            NULL,
    };
    vfs_mount("/dev/kbd", &kbd_vfs_reg);

    ringbuff_init(&c_buff, BUFF_SIZE);
    spinlock_init(&read_spinlock);
    semaphore_init(&data_available);
}
