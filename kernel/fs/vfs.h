#ifndef FS_VFS_H
#define FS_VFS_H

#include <libk/types.h>

struct vfs_reg {
    int (*open)(char* path);
    int (*close)(char* path);

    size_t (*read)(char* path, void* buff, size_t len);
    size_t (*write)(char* path, void* buff, size_t len);
};

void vfs_init();

int vfs_mount(char* path, struct vfs_reg* handles);
int vfs_unmout(char* path);

int vfs_open(char* path);
int vfs_close(int8_t fd);

size_t vfs_read(int8_t fd, void* buff, size_t len);
size_t vfs_write(int8_t fd, void* buff, size_t len);

#define ENOTFOUND 5

#endif