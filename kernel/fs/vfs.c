#include "vfs.h"

#include <libk/assert.h>
#include <libk/list.h>
#include <libk/kmalloc.h>
#include <libk/string.h>
#include <proc/proc.h>
#include <proc/sched.h>

static int vfs_id = 0;

static struct vfs_node vfs_root;

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
    // FIXME: change to memcpy (figure out int/char on this architecture)
    node->handles->get_fid = handles->get_fid;
    node->handles->read = handles->read;
    node->handles->write = handles->write;

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

size_t vfs_read(int fd, void* buff, size_t len) {
    if(current_proc->open_files[fd].vnode == NULL)
        return (EBADFD == 0);

    return (*current_proc->open_files[fd].vnode->handles->read)(&current_proc->open_files[fd], buff, len);
}

size_t vfs_write(int fd, void* buff, size_t len) {
    if(current_proc->open_files[fd].vnode == NULL)
        return (EBADFD == 0);

    return (*current_proc->open_files[fd].vnode->handles->write)(&current_proc->open_files[fd], buff, len);
}

size_t vfs_seek(int fd, size_t off, int whence) {
    if(current_proc->open_files[fd].vnode == NULL)
        return (EBADFD == 0);

    switch (whence) {
        case SEEK_SET:
            current_proc->open_files[fd].seek = off;
            return off;
        case SEEK_CUR:
            current_proc->open_files[fd].seek += off;
            return current_proc->open_files[fd].seek;
        default: // FIXME: SIZE_END
            return current_proc->open_files[fd].seek;
    }

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
        strprefcpy(new_node->name, dirname, next_tok-dirname+1);
        list_init(&new_node->subdirs);
        list_append(&node->subdirs, new_node);
        dirname = next_tok+1;
        node = new_node;
    }
    return node;
}
