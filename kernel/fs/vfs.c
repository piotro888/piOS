#include "vfs.h"

#include <libk/assert.h>
#include <libk/list.h>
#include <libk/kmalloc.h>
#include <libk/string.h>
#include <proc/proc.h>
#include <proc/sched.h>
#include <sys/systructs.h>

static struct list mount_list;

static unsigned int req_id = 0;
static unsigned int free_vid = 0;

static struct inode* root_inode;

void vfs_init() {
    list_init(&mount_list);

    root_inode = kmalloc(sizeof(struct inode));
    root_inode->vnode = NULL;
    root_inode->name = kmalloc(2);
    strcpy(root_inode->name, "/");
    root_inode->type = INODE_TYPE_DIRECTORY;
}

struct inode* vfs_find_inode(const char* path) {
    // TODO: normalize path (eliminate ., ..)
    struct vnode* vnode = vfs_vnode_for(path);
    if (!vnode)
        return NULL;

    const char* vnode_local_path = path+strlen(vnode->abs_mount_path);
    return (*vnode->reg->find_inode)(vnode, vnode_local_path);
}

ssize_t vfs_open(struct inode* inode, struct proc_file* target) {
    if (!inode)
        return -ENOTFOUND;

    target->inode = inode;
    target->offset = 0;
    target->fcntl_flags = 0;

    return 0;
}

ssize_t vfs_close (struct proc_file* file) {
    if(!file->inode)
        return -EBADFD;
    file->inode = NULL;
    return 0;
}

ssize_t vfs_read_blocking(struct proc_file* file, int pid, void* buff, size_t size) {
    if(!file->inode)
        return -EBADFD;
    if(!file->inode->vnode->reg->async_request)
        return -ENOSUP;

    struct semaphore* lock = kmalloc(sizeof(struct semaphore));
    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = file;
    req->type = VFS_ASYNC_TYPE_READ;
    req->pid = pid;
    req->req_id = ++req_id;
    req->callback = NULL;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    if (file->fcntl_flags & O_NONBLOCK)
        req->flags |= VFS_ASYNC_FLAG_WANT_WOULDBLOCK;
    req->fin_sema = lock;

    semaphore_init(lock);

    ssize_t rc = (file->inode->vnode->reg->async_request)(req);
    if(rc < 0) {
        kfree(req);
        kfree(lock);
        return rc;
    }

    semaphore_down(lock);

    ssize_t res = req->res;

    kfree(req);
    kfree(lock);

    return res;
}

ssize_t vfs_write_blocking(struct proc_file* file, int pid, void* buff, size_t size) {
    if(!file->inode)
        return -EBADFD;
    if(!file->inode->vnode->reg->async_request)
        return -ENOSUP;

    struct semaphore* lock = kmalloc(sizeof(struct semaphore));
    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = file;
    req->type = VFS_ASYNC_TYPE_WRITE;
    req->pid = pid;
    req->req_id = ++req_id;
    req->callback = NULL;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    if (file->fcntl_flags & O_NONBLOCK)
        req->flags |= VFS_ASYNC_FLAG_WANT_WOULDBLOCK;
    req->fin_sema = lock;

    semaphore_init(lock);

    ssize_t rc = (file->inode->vnode->reg->async_request)(req);
    if(rc < 0) {
        kfree(req);
        kfree(lock);
        return rc;
    }

    semaphore_down(lock);

    ssize_t res = req->res;

    kfree(req);
    kfree(lock);

    return res;
}

ssize_t vfs_read_async(struct proc_file* file, int pid, void* buff, size_t size, void (*callback)(), int rid) {
    if(!file->inode)
        return -EBADFD;
    if(!file->inode->vnode->reg->async_request)
        return -ENOSUP;

    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = file;
    req->type = VFS_ASYNC_TYPE_READ;
    req->pid = pid;
    req->req_id = rid ? rid : ++req_id;
    req->callback = callback;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    if (file->fcntl_flags & O_NONBLOCK)
        req->flags |= VFS_ASYNC_FLAG_WANT_WOULDBLOCK;
    req->fin_sema = NULL;
    int id = req->req_id;

    ssize_t rc = (file->inode->vnode->reg->async_request)(req);

    // NOTE: req must be kfreed at completion callback/semaphore

    if(rc < 0) {
        kfree(req);
        return rc;
    }
    return id;
}

ssize_t vfs_write_async(struct proc_file* file, int pid, void* buff, size_t size, void (*callback)(), int rid) {
    if(!file->inode)
        return -EBADFD;
    if(!file->inode->vnode->reg->async_request)
        return -ENOSUP;

    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = file;
    req->type = VFS_ASYNC_TYPE_WRITE;
    req->pid = pid;
    req->req_id = rid ? rid : ++req_id;
    req->callback = callback;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    if (file->fcntl_flags & O_NONBLOCK)
        req->flags |= VFS_ASYNC_FLAG_WANT_WOULDBLOCK;
    req->fin_sema = NULL;

    int id = req->req_id;

    ssize_t rc = (*file->inode->vnode->reg->async_request)(req);

    if(rc < 0) {
        kfree(req);
        return rc;
    }
    return id;
}

ssize_t vfs_seek(struct proc_file* file, ssize_t off, int whence) {
    if(!file->inode)
        return -EBADFD;

    switch (whence) {
        case SEEK_SET:
            if(off < 0)
                return -EINVAL;
            file->offset = off;
            return off;
        case SEEK_CUR:
            if(file->offset + off < 0)
                return -EINVAL;
            file->offset += off;
            return file->offset;
        default: // FIXME: SIZE_END
            return file->offset;
    }

}

int vfs_fio_ctl(struct proc_file* file, int vpid, unsigned type, unsigned number, int fdrel) {
    if(!file->inode)
        return -EBADFD;
    if(!file->inode->vnode->reg->fio_ctl)
        return -ENOSUP;

    if (fdrel)
        return (file->inode->vnode->reg->fio_ctl)(file->inode, type, number, file, vpid);  
    else 
        return (file->inode->vnode->reg->fio_ctl)(file->inode, type, number, NULL, 0);  
}

int validate_fd(int fd) {
    return !(fd > PROC_MAX_FILES || fd < 0);
}

int proc_free_fd(const struct proc* proc) {
    for(int i=0; i<PROC_MAX_FILES; i++) {
        if(proc->open_files[i].inode == NULL)
            return i;
    }
    return -ETOOMANYFILES;
}

char* vfs_abs_path_alloc(struct inode* inode) {
    if (!inode->vnode)
        return inode->name;

    char* path_inside_vnode = inode->vnode->reg->get_abs(inode);
    char* vnode_mount = inode->vnode->abs_mount_path;

    char* res = kmalloc(strlen(path_inside_vnode)+strlen(vnode_mount)+1);
    strcpy(res, vnode_mount);
    strcpy(res+strlen(vnode_mount), path_inside_vnode);

    return res;
}

struct vnode* vfs_vnode_for(const char* path) {
    struct vnode* best_match = NULL;
    unsigned int match_slash_cnt = 0;

    list_foreach(&mount_list) {
        struct vnode* itv = LIST_FOREACH_VAL(struct vnode*);
        if (strcnt(itv->abs_mount_path, '/') > match_slash_cnt
           && !strncmp(itv->abs_mount_path, path, strlen(itv->abs_mount_path))) {
            // above if finds mount point with largest number of slashes, that is a prefix of searched path

            // match last segment in mount point to path. (it may be without slash at end; and prefix could incorrectly match)
            const char* path_match_segment_end = strchr(path+strlen(itv->abs_mount_path)-1, '/');
            if (!path_match_segment_end)
                path_match_segment_end = path+strlen(path);

            size_t full_match_len = path_match_segment_end-path;

            if (strncmp(itv->abs_mount_path, path, full_match_len))
                continue;

            match_slash_cnt = strcnt(itv->abs_mount_path, '/');
            best_match = itv;
        }
    }

    return best_match;
}

struct vnode* vfs_mount(const struct vfs_reg* vfs_reg, struct inode* mount) {
    struct vnode* vnode = kmalloc(sizeof(struct vnode));
    vnode->abs_mount_path = vfs_abs_path_alloc(mount);
    vnode->mount_inode = mount;
    vnode->reg = vfs_reg;
    vnode->vid = free_vid++;

    // todo add flag for inode is mountpooint
    list_append(&mount_list, vnode);

    return vnode;
}

struct inode* vfs_get_root_inode() {
    return root_inode;
}
