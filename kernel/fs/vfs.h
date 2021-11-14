#ifndef FS_VFS_H
#define FS_VFS_H

#include <libk/types.h>

struct vfs_reg_t {
    int8_t (*open)(char* path);
    int8_t (*close)(char* path);

    size_t (*read)(char* path, void* buff, size_t len);
    size_t (*write)(char* path, void* buff, size_t len);
};

void vfs_mount(char* path, struct vfs_reg_t* vfs_reg);
void vfs_unmout(char* path);

int8_t vfs_open(char* path);
int8_t vfs_close(int8_t fd);

size_t vfs_read(int8_t fd, void* buff, size_t len);
size_t vfs_write(int8_t fd, void* buff, size_t len);


#endif