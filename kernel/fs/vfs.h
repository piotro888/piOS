#ifndef FS_VFS_H
#define FS_VFS_H

#include <libk/types.h>
#include <libk/list.h>
#include <fs/vfs_async.h>

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

#define VFS_REG_FLAG_KERNEL_BUFFER_ONLY 1
struct vfs_reg {
    int (*get_fid)(char* path);

    ssize_t (*async_request)(struct vfs_async_req_t* req);
    int flags;
};

void vfs_init();

int vfs_mount(char* path, const struct vfs_reg* handles);
int vfs_unmount(char* path);

int vfs_open(char* path);
int vfs_close(int fd);

// used in all calls from userspace
ssize_t vfs_read_async(int fd, int vpid, void* buff, size_t size, void (*callback)(), int rid);
ssize_t vfs_write_async(int fd, int vpid, void* buff, size_t size, void (*callback)(), int rid);

// blocking wrappers for kernel threads
ssize_t vfs_read_blocking(int fd, void* buff, size_t size);
ssize_t vfs_write_blocking(int fd, void* buff, size_t size);

int vfs_get_vnode_flags(int fd);

ssize_t vfs_seek(int fd, ssize_t off, int whence);
#define SEEK_SET 0
#define SEEK_CUR 1

#define EINVAL  1
#define ENOTFOUND 5
#define ETOOMANYFILES 6
#define EBADFD 7
#define EWOULDBLOCK 8
#define ENOSUP 9

#endif
