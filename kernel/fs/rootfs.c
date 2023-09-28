#include "rootfs.h"

#include <string.h>
#include <libk/list.h>
#include <libk/log.h>
#include <libk/kmalloc.h>
#include <fs/vfs.h>

#include <fs/kbd.h>
#include <fs/sio.h>
#include <fs/tmp.h>
#include <driver/tty.h>

#include <libk/kprintf.h>

static struct list in_list;

static unsigned ffid = 0;

struct ent {
    char* full_path;
    struct inode* inode;
};

char* full_path(const char* path_prefix, const char* name) {
    char* full_path = kmalloc(strlen(path_prefix)+strlen(name)+1);
    strcpy(full_path, path_prefix);
    strcpy(full_path+strlen(path_prefix), name);
    return full_path;
}

struct inode* rootfs_create_entry(struct vnode* self, const char* path_prefix, const char* name, unsigned type) {
    struct ent* ent = kmalloc(sizeof(struct ent));
    struct inode* inode = kmalloc(sizeof(struct inode));
    ent->full_path = full_path(path_prefix, name); 
    ent->inode = inode;
    inode->fid = ffid++;
    inode->type = type;
    inode->name = ent->full_path+strlen(path_prefix);
    inode->size = 0;
    inode->vnode = self;
    list_append(&in_list, ent);
    return inode;
}

struct inode* rootfs_vfs_find_inode(struct vnode* self, const char* path) {
    list_foreach(&in_list) {
        struct ent* ent = LIST_FOREACH_VAL(struct ent*);
        if (!strcmp(path, ent->full_path)) {
            ent->inode->vnode = self;
            return ent->inode;
        }
    }
    return NULL;
}

char* rootfs_vfs_absolute_path(const struct inode* inode) {
    list_foreach(&in_list) {
        struct ent* ent = LIST_FOREACH_VAL(struct ent*);
        if (ent->inode->fid == inode->fid) {
            return ent->full_path;
        }
    }
    return NULL;
}

const static struct vfs_reg vfs_reg = {
    "rootfs",
    0,
    NULL,

    rootfs_vfs_find_inode,
    rootfs_vfs_absolute_path,
    NULL
};

const struct vfs_reg* rootfs_get_vfs_reg() {
    return &vfs_reg;
}

void rootfs_create(struct vnode* self) {
    list_init(&in_list);
    rootfs_create_entry(self, "", "", INODE_TYPE_DIRECTORY);
    rootfs_create_entry(self, "", "dev/", INODE_TYPE_DIRECTORY);
    
    tmp_init();
    vfs_mount(tmp_get_vfs_reg(), rootfs_create_entry(self, "", "tmp/", INODE_TYPE_DIRECTORY));

    log("mounting system devices");
    vfs_mount(tty_get_vfs_reg(), rootfs_create_entry(self, "dev/", "tty", INODE_TYPE_FILE));
    vfs_mount(sio_get_vfs_reg(), rootfs_create_entry(self, "dev/", "log", INODE_TYPE_FILE));
    kbd_vfs_init();
    vfs_mount(kbd_get_vfs_reg(), rootfs_create_entry(self, "dev/", "kbd", INODE_TYPE_FILE));
}
