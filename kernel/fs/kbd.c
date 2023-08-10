#include "kbd.h"
#include <fs/vfs.h>
#include <fs/vfs_async.h>
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

static struct semaphore data_available, list_sema, insert_lock;
static struct list req_list;

static size_t list_read_pending = 0;

static struct inode root_inode = {
    "",
    0,
    0,
    NULL,
    INODE_TYPE_DEVICE
};

struct inode* kbd_fs_get_inode(struct vnode* self, const char* path) { 
    (void)path;
    root_inode.vnode = self;
    return &root_inode;
}

int kbd_submit_req(struct vfs_async_req_t* req) {
    if (req->type != 1)
        return -ENOSUP;

    semaphore_down(&insert_lock);
    if (req->flags & VFS_ASYNC_FLAG_WANT_WOULDBLOCK && list_read_pending > ringbuff_length(&c_buff))
        return -EWOULDBLOCK;
    list_read_pending += list_read_pending;
    
    list_append(&req_list, req);
    semaphore_up(&list_sema);
    semaphore_up(&insert_lock);

    return 0;
}

void kbd_thread_loop() {
    for(;;) {
        semaphore_down(&list_sema);

        struct vfs_async_req_t* req = req_list.first->val;

        // reading ringbuff is thread safe here
        while(!ringbuff_length(&c_buff))
            semaphore_down(&data_available);

        ASSERT(req->pid == 0); // only reads to kernel buffer are supported
        ssize_t res = ringbuff_read(&c_buff, req->vbuff, req->size);

        if(ringbuff_length(&c_buff))
            semaphore_binary_up(&data_available);
        
        list_read_pending -= res;
        list_remove(&req_list, req_list.first);

        vfs_async_finalize(req, req->size);
    }
}

void kbd_vfs_submit_char(char c) {
    ringbuff_write(&c_buff, (unsigned char*)&c, 1);

    semaphore_binary_up(&data_available);
}

const struct vfs_reg kbd_vfs_reg = {
        "kbd",
        VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
        NULL,

        kbd_fs_get_inode,
        NULL,
        kbd_submit_req
};

const struct vfs_reg* kbd_get_vfs_reg() {
    return &kbd_vfs_reg;
}


void kbd_vfs_init() {
    ringbuff_init(&c_buff, BUFF_SIZE);
    semaphore_init(&data_available);
    semaphore_init(&list_sema);
    semaphore_init(&insert_lock);
    semaphore_up(&insert_lock);
}
