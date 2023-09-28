#ifndef FS_TMP_H
#define FS_TMP_H

#include <libk/types.h>
#include <fs/vfs.h>

void tmp_init();
const struct vfs_reg* tmp_get_vfs_reg();

#endif
