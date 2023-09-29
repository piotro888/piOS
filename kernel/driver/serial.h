#ifndef DRIVER_SERIAL_H
#define DRIVER_SERIAL_H

#include <libk/types.h>

void serial_direct_putc(char c);
void serial2_direct_putc(char c);
const struct vfs_reg* serial_get_vfs_reg(int devno);

#endif
