#ifndef FS_VFS_H
#define FS_VFS_H

#include <libk/types.h>
#include <libk/list.h>

struct vfs_node {
    int vid;
    char* name;
    struct vfs_reg* handles;
    struct vfs_node* parent;

    struct list subdirs;

    char* path_search; // FIXME: find better way to do this
};

struct fd_info {
    struct vfs_node* vnode;
    int inode;

    char* name;
    size_t seek;
};

struct vfs_reg {
    int (*get_fid)(char* path);

    size_t (*read)(struct fd_info* file, void* buff, size_t len);
    size_t (*write)(struct fd_info* file, void* buff, size_t len);
};

void vfs_init();

int vfs_mount(char* path, const struct vfs_reg* handles);
int vfs_unmount(char* path);

int vfs_open(char* path);
int vfs_close(int fd);

size_t vfs_read (int fd, void* buff, size_t len);
size_t vfs_write(int fd, void* buff, size_t len);

#define ENOTFOUND 5
#define ETOOMANYFILES 6
#define EBADFD 7

#endif
