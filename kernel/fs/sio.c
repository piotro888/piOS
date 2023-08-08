#include "sio.h"
#include <libk/log.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

int sio_fs_get_fid(char* path) { (void)path; return 0; }

ssize_t sio_submit_request(struct vfs_async_req_t* req) {
    if(req->type != VFS_ASYNC_TYPE_WRITE)
        return -ENOSUP;
    // todo: don't igore len, use snprintf
    log("%s", req->vbuff);
    vfs_async_finalize(req, req->size);
    return 0;
}

void sio_mnt_vfs() {
    const struct vfs_reg reg = {
            sio_fs_get_fid,
            sio_submit_request,
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY
    };
    vfs_mount("/dev/log", &reg);
}
