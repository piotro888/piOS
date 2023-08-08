#include "vfs.h"

#include <string.h>
#include <libk/assert.h>
#include <libk/list.h>
#include <libk/kmalloc.h>
#include <proc/proc.h>
#include <proc/sched.h>

static int vfs_id = 0;

static struct vfs_node vfs_root;

static unsigned int req_id = 0;

void vfs_init() {
    vfs_root.name = kmalloc(2);
    vfs_root.parent = &vfs_root;
    strcpy(vfs_root.name, "/");
    vfs_root.handles = NULL;
}

struct vfs_node* vnode_tree_create(char* path);
struct vfs_node* vnode_tree_find(char* path);
int fd_get_free();

int vfs_mount(char* path, const struct vfs_reg* handles) {
    ASSERT(path[0] == '/');

    struct vfs_node* node = vnode_tree_create(path);
    node->handles = kmalloc(sizeof(struct vfs_reg));
    memcpy(node->handles, handles, sizeof(struct vfs_reg));

    node->vid = ++vfs_id;

    return 0;
}

int vfs_open(char* path) {
    struct vfs_node *vnode = vnode_tree_find(path);
    if (!vnode)
        return -ENOTFOUND;

    // call open on handle with suffix of path
    int file_fid = (*vnode->handles->get_fid)(vnode->path_search);

    if (file_fid < 0) {
        return file_fid; // pass errors
    }

    int fd = fd_get_free();
    if(fd < 0)
        return -ETOOMANYFILES;

    ASSERT(scheduling_enabled); // current proc set

    current_proc->open_files[fd].vnode = vnode;
    current_proc->open_files[fd].inode = file_fid;
    current_proc->open_files[fd].seek = 0;

    return fd;
}

int vfs_close(int fd) {
    if(fd < 0 || fd > PROC_MAX_FILES || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;

    current_proc->open_files[fd].vnode = NULL;
    return 0;
}

ssize_t vfs_read_blocking(int fd, void* buff, size_t size) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;
    if(!current_proc->open_files[fd].vnode->handles->async_request)
        return -ENOSUP;

    struct semaphore* lock = kmalloc(sizeof(struct semaphore));
    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = &current_proc->open_files[fd];
    req->type = VFS_ASYNC_TYPE_READ;
    req->pid = 0;
    req->req_id = ++req_id;
    req->callback = NULL;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    req->fin_sema = lock;

    semaphore_init(lock);

    ssize_t rc = (*current_proc->open_files[fd].vnode->handles->async_request)(req);
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

ssize_t vfs_write_blocking(int fd, void* buff, size_t size) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;
    if(!current_proc->open_files[fd].vnode->handles->async_request)
        return -ENOSUP;

    struct semaphore* lock = kmalloc(sizeof(struct semaphore));
    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = &current_proc->open_files[fd];
    req->type = VFS_ASYNC_TYPE_WRITE;
    req->pid = 0;
    req->req_id = ++req_id;
    req->callback = NULL;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    req->fin_sema = lock;

    semaphore_init(lock);

    ssize_t rc = (*current_proc->open_files[fd].vnode->handles->async_request)(req);
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

ssize_t vfs_read_async(int fd, int pid, void* buff, size_t size, void (*callback)(), int rid) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;
    if(!current_proc->open_files[fd].vnode->handles->async_request)
        return -ENOSUP;

    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = &current_proc->open_files[fd];
    req->type = VFS_ASYNC_TYPE_READ;
    req->pid = pid;
    req->req_id = rid ? rid : ++req_id;
    req->callback = callback;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    req->fin_sema = NULL;
    int id = req->req_id;

    ssize_t rc = (*current_proc->open_files[fd].vnode->handles->async_request)(req);

    // NOTE: req must be kfreed at completion callback/semaphore

    if(rc < 0) {
        kfree(req);
        return rc;
    }
    return id;
}

ssize_t vfs_write_async(int fd, int pid, void* buff, size_t size, void (*callback)(), int rid) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;
    if(!current_proc->open_files[fd].vnode->handles->async_request)
        return -ENOSUP;

    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));
    req->file = &current_proc->open_files[fd];
    req->type = VFS_ASYNC_TYPE_WRITE;
    req->pid = pid;
    req->req_id = rid ? rid : ++req_id;
    req->callback = callback;
    req->vbuff = buff;
    req->size = size;
    req->flags = 0;
    req->fin_sema = NULL;

    int id = req->req_id;

    ssize_t rc = (*current_proc->open_files[fd].vnode->handles->async_request)(req);
    
    if(rc < 0) {
        kfree(req);
        return rc;
    }
    return id;
}

ssize_t vfs_seek(int fd, ssize_t off, int whence) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;

    switch (whence) {
        case SEEK_SET:
            if(off < 0)
                return -EINVAL;
            current_proc->open_files[fd].seek = off;
            return off;
        case SEEK_CUR:
            if(current_proc->open_files[fd].seek + off < 0)
                return -EINVAL;
            current_proc->open_files[fd].seek += off;
            return current_proc->open_files[fd].seek;
        default: // FIXME: SIZE_END
            return current_proc->open_files[fd].seek;
    }

}

int vfs_get_vnode_flags(int fd) {
    if(fd > PROC_MAX_FILES || fd < 0 || current_proc->open_files[fd].vnode == NULL)
        return -EBADFD;
    return current_proc->open_files[fd].vnode->handles->flags;
}

int fd_get_free() {
    for(int i=0; i<PROC_MAX_FILES; i++) {
        if(current_proc->open_files[i].vnode == NULL)
            return i;
    }
    return -1;
}

struct vfs_node* vnode_tree_find(char* path) {
    struct vfs_node* node = &vfs_root;

    char* dirname = ++path;  // skip first slash (already matched in vfs_root)
    char* next_tok = path;
    while(*dirname) {
        next_tok = strchr(next_tok+1, '/');
        if(next_tok == NULL) // file nodes without /
            next_tok = dirname+strlen(dirname)-1;

        int subdir_found = 0;
        if(strncmp(dirname, "../", 3) == 0) {
            node = node->parent;
            dirname = next_tok+1;
            subdir_found = 1;
            continue; // path_search is already set for this node
        } else if(strncmp(dirname, "./", 2) == 0) {
            dirname = next_tok+1;
            subdir_found = 1;
            continue;
        }

        list_foreach(&node->subdirs) {
            struct vfs_node* sub = LIST_FOREACH_VAL(struct vfs_node*);
            if(strncmp(dirname, sub->name, next_tok-dirname+1) == 0) {
                node = sub;
                node->path_search = next_tok;
                dirname = next_tok+1;

                if(*next_tok == '/')
                    subdir_found = 1;
                break;
            }
        }
        if(!subdir_found)
            break;
    }

    // use last vnode with handles set
    while(node != &vfs_root) {
        if(node->handles)
            return node;

        node = node->parent;
    }
    return NULL;
}

struct vfs_node* vnode_tree_create(char* path) {
    struct vfs_node* node = &vfs_root;

    char* dirname = ++path; // skip first slash (already matched in vfs_root)
    char* next_tok = path;
    while(*dirname) {
        next_tok = strchr(next_tok+1, '/');
        if(next_tok == NULL)
            next_tok = dirname+strlen(dirname)-1;

        int subdir_found = 0;
        list_foreach(&node->subdirs) {
            struct vfs_node* sub = LIST_FOREACH_VAL(struct vfs_node*);
            if(strncmp(dirname, sub->name, next_tok-dirname+1) == 0) {
                // enter next node in path
                node = sub;
                dirname = next_tok+1;

                if(*next_tok == '/')
                    subdir_found = 1;
                break;
            }
        }
        if(subdir_found)
            continue;

        // vnode for dir not found, create one
        struct vfs_node* new_node = kmalloc(sizeof(struct vfs_node));
        new_node->name = kmalloc(next_tok-path+2);
        new_node->parent = node;
        new_node->handles = NULL;
        size_t prefix_len = next_tok-dirname+1;
        strncpy(new_node->name, dirname, prefix_len);
        new_node->name[prefix_len+1] = '\0';
        list_init(&new_node->subdirs);
        list_append(&node->subdirs, new_node);
        dirname = next_tok+1;
        node = new_node;
    }
    return node;
}
