#include "sio.h"
#include <libk/log.h>


int sio_fs_get_fid(char* path) { (void)path; return 0; }

ssize_t sio_fs_write(struct fd_info* file, void* buff, size_t len) {
    (void) file;
    // todo: don't igore len, use snprintf
    log("%s", buff);
    return len;
}

void sio_mnt_vfs() {
    const struct vfs_reg reg = {
            sio_fs_get_fid,
            NULL,
            sio_fs_write,
            NULL,
            NULL,
    };
    vfs_mount("/dev/log", &reg);
}
