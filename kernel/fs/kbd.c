#include "kbd.h"
#include <fs/vfs.h>
#include <libk/types.h>
#include <libk/con/spinlock.h>
#include <libk/con/semaphore.h>

/* Temporary vfs device hooked up
 * to PS/2 keyboard driver.
 * Only for testing purposes of vfs 
 */

#define BUFF_SIZE 16

char c_buff[BUFF_SIZE];
int c_buff_head = 0, c_buff_tail = 0;

static struct spinlock read_spinlock;
static struct semaphore data_available;

int kbd_get_fid(char* path) {
    return 0; // we have only one file
}

size_t kbd_read(struct fd_info* file, char* buff, size_t len) {
    spinlock_lock(&read_spinlock);
    while(c_buff_head == c_buff_tail) {
        spinlock_unlock(&read_spinlock);
        semaphore_down(&data_available); // block until some data is in the buffer
        spinlock_lock(&read_spinlock); // lock reading (and tail modification) to one process
    }

    size_t size_to_read = (c_buff_head > c_buff_tail ? c_buff_head-c_buff_tail : BUFF_SIZE-c_buff_tail+c_buff_head);
    if(size_to_read > len)
        size_to_read = len;

    for(size_t i=0; i<size_to_read; i++) {
        *(char*)buff = c_buff[c_buff_tail];
        (char*)buff++;
        if(++c_buff_tail == BUFF_SIZE)
            c_buff_tail = 0;
    }

    if(c_buff_head != c_buff_tail)
        semaphore_binary_up(&data_available); // data is still in buffer, unblock other process

    spinlock_unlock(&read_spinlock);

    return size_to_read;
}

size_t kbd_write(struct fd_info* file, void* buff, size_t len) {
    return 0;
}

void kbd_vfs_submit_char(char c) {
    c_buff[c_buff_head] = c;

    if (++c_buff_head == BUFF_SIZE)
        c_buff_head = 0;
    // the best way seems to ignore buffer overruns and prevent locking on submit (irq driven)

    semaphore_binary_up(&data_available);
}

void kbd_vfs_init() {
    const struct vfs_reg kbd_vfs_reg = {
            kbd_get_fid,
            kbd_read,
            kbd_write
    };
    vfs_mount("/dev/kbd/", &kbd_vfs_reg);

    c_buff_head = c_buff_tail = 0;
    spinlock_init(&read_spinlock);
    semaphore_init(&data_available);
}
