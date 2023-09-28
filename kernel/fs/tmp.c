#include "tmp.h"
#include <string.h>
#include <libk/kmalloc.h>
#include <libk/list.h>
#include <libk/math.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

static struct list files;
static struct semaphore lock;

static unsigned fid = 0;

#define MAX_FILES 64
void* alloc_fid[MAX_FILES];

struct inode* tmp_find_inode(struct vnode* self, const char* path) {
    struct inode* res = NULL;
    list_foreach(&files) {
        if (strcmp(LIST_FOREACH_VAL(struct inode*)->name, path) == 0) {
            res = LIST_FOREACH_VAL(struct inode*);
            break;
        }
    }

    if (!res) {
        if(fid >= MAX_FILES)
            return NULL;

        // create
        res = kmalloc(sizeof(struct inode));
        res->fid = fid++;
        res->name = kmalloc(strlen(path)+1);
        strcpy(res->name, path);
        res->size = 0;
        res->type = INODE_TYPE_FILE;
        alloc_fid[res->fid] = NULL;

        list_append(&files, res);
    }
    
    if (res)
        res->vnode = self;

    return res;
}

ssize_t tmp_write(struct vfs_async_req_t* req) {
    // note: seek no supported
    void** fdat = &alloc_fid[req->file->inode->fid];

    if (req->size > req->file->inode->size) { // extend file
        if (*fdat)
            kfree(*fdat); // fully overwritten
        void* new = kmalloc(req->size);
        *fdat = new;
        req->file->inode->size = req->size;
    }

    memcpy(*fdat, req->vbuff, req->size);

    return req->size;
}

ssize_t tmp_read(struct vfs_async_req_t* req) {
    // note: seek no supported
    void* dat = alloc_fid[req->file->inode->fid];

    size_t size = MIN(req->size, req->file->inode->size);

    memcpy(req->vbuff, dat, size);

    return size;
}

ssize_t tmp_submit_req(struct vfs_async_req_t* req) {
    semaphore_down(&lock);
    if (req->type ==  VFS_ASYNC_TYPE_READ) {
        vfs_async_finalize(req, tmp_read(req));
    } else if (req->type == VFS_ASYNC_TYPE_WRITE) {
        vfs_async_finalize(req, tmp_write(req));
    } else {
        return -ENOSUP;
    }

    semaphore_up(&lock);
    return 0;
}

void tmp_init() {
    semaphore_init(&lock);
    semaphore_up(&lock);
    list_init(&files);
}

const static struct vfs_reg reg = {
    "tmp",
    VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
    NULL,

    tmp_find_inode,
    NULL,
    tmp_submit_req
};

const struct vfs_reg* tmp_get_vfs_reg() {
    return &reg;
}
