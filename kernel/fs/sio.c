#include "sio.h"
#include <libk/log.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

static struct inode root_inode = {
    "",
    0,
    0,
    NULL,
    INODE_TYPE_DEVICE
};

struct inode* sio_fs_get_inode(struct vnode* self, const char* path) { 
    (void)path;
    root_inode.vnode = self;
    return &root_inode;
}

ssize_t sio_submit_request(struct vfs_async_req_t* req) {
    if(req->type != VFS_ASYNC_TYPE_WRITE)
        return -ENOSUP;
    // todo: don't igore len, use snprintf
    log("%s", req->vbuff);
    vfs_async_finalize(req, req->size);
    return 0;
}

const static struct vfs_reg reg = {
            "sio",
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
            NULL,

            sio_fs_get_inode,
            NULL,
            sio_submit_request
};
const struct vfs_reg* sio_get_vfs_reg() {
    return &reg;
}
