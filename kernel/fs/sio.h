#ifndef FS_SIO_H
#define FS_SIO_H

#include <libk/types.h>
#include <fs/vfs.h>

int sio_fs_get_fid(char* path);

ssize_t sio_fs_write(struct fd_info* file, void* buff, size_t len);

void sio_mnt_vfs();


#endif
