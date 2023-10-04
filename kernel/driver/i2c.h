#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H

#include <libk/types.h>
#include <fs/vfs.h>

void i2c_write(u8 addr, u8 reg, u8 data);
int i2c_read(u8 addr, u8 reg);
const struct vfs_reg* i2c_get_vfs_reg();

#define F_DRIVER_I2C_ADDRESS 0x100
#define F_DRIVER_I2C_REGISTER 0x101

#endif
