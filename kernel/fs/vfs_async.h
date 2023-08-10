#ifndef FS_VFS_ASYNC_H
#define FS_VFS_ASYNC_H

#include <libk/types.h>
#include <libk/con/semaphore.h>

#define VFS_ASYNC_TYPE_READ 1
#define VFS_ASYNC_TYPE_WRITE 2

#define VFS_ASYNC_FLAG_WANT_WOULDBLOCK 1

struct vfs_async_req_t {
    unsigned int req_id;
    struct proc_file* file;
    int type;

    unsigned int pid;
    void* vbuff;
    size_t size;

    void (*callback)(struct vfs_async_req_t* req);
    struct semaphore* fin_sema;
    ssize_t res;


    int flags;
};

void vfs_async_finalize(struct vfs_async_req_t* req, ssize_t ret);

#endif