#ifndef FS_KBDFS_H
#define FS_KBDFS_H

void kbd_vfs_init();
const struct vfs_reg* kbd_get_vfs_reg();
void kbd_vfs_submit_char(char c);

#endif
