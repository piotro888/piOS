#ifndef FS_VFS_H
#define FS_VFS_H

#include <libk/types.h>
#include <libk/list.h>
#include <fs/vfs_async.h>
#include <proc/proc.h>

struct vnode;

struct inode {
    char* name;
    u16 fid; // file id unique to vnode
    size_t size;
    struct vnode* vnode;
    unsigned type;
};

#define INODE_TYPE_FILE 1
#define INODE_TYPE_DIRECTORY 2
#define INODE_TYPE_DEVICE 3

struct vnode {
    unsigned vid;

    const struct vfs_reg* reg;
    
    char* abs_mount_path;
    struct inode* mount_inode;
};


#define VFS_REG_FLAG_KERNEL_BUFFER_ONLY 1
struct vfs_reg {
    char* fs_name;
    int flags;
    void* vnode_data;
    
    struct inode* (*find_inode)(struct vnode* self, const char* path);
    // get absoulte path inside driver (no leading slash) 
    char* (*get_abs)(const struct inode* inode);
    
    ssize_t (*async_request)(struct vfs_async_req_t* req);
};

void vfs_init();

struct vnode* vfs_vnode_for(const char* path);
struct inode* vfs_find_inode(const char* path);

struct vnode* vfs_mount(const struct vfs_reg* vfs_reg, struct inode* mount);

// proc_file opreations
int vfs_open(struct inode* inode, struct proc_file* target);

int vfs_close(struct proc_file* inode);

// used in all calls from userspace
ssize_t vfs_read_async(struct proc_file* file, int vpid, void* buff, size_t size, void (*callback)(), int rid);
ssize_t vfs_write_async(struct proc_file* file, int vpid, void* buff, size_t size, void (*callback)(), int rid);

ssize_t vfs_read_blocking(struct proc_file* file, int vpid, void* buff, size_t size);
ssize_t vfs_write_blocking(struct proc_file* file, int vpid, void* buff, size_t size);

ssize_t vfs_seek(struct proc_file* file, ssize_t off, int whence);

struct inode* vfs_get_root_inode();

int proc_free_fd(const struct proc* proc);
int validate_fd(int fd);

#define SEEK_SET 0
#define SEEK_CUR 1

#define EINVAL  1
#define ENOTFOUND 5
#define ETOOMANYFILES 6
#define EBADFD 7
#define EWOULDBLOCK 8
#define ENOSUP 9

#endif
