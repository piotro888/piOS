#ifndef FS_ROOTFS_H
#define FS_ROOTFS_H

#include <fs/vfs.h>

// Read-only, FS, for storing mountpoints inodes in empty fs space
// all directories along paths must be manually created

void rootfs_create(struct vnode* self);

const struct vfs_reg* rootfs_get_vfs_reg();

struct inode* rootfs_create_entry(struct vnode* self, const char* path_prefix, const char* name, unsigned inode_type);

#endif
