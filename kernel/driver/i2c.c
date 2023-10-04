#include "i2c.h"
#include <libk/types.h>
#include <libk/kmalloc.h>
#include <fs/vfs.h>
#include <proc/virtual.h>

#define BASE_ADDR_OFF 0x40

#define ADDRESS_PAGE 0x4

#define ADDR_REG (volatile u8*)(BASE_ADDR_OFF+0x0)
#define REG_REG (volatile u8*)(BASE_ADDR_OFF+0x2)
#define DATA_REG (volatile u8*)(BASE_ADDR_OFF+0x4)
#define STATUS_REG (volatile u8*)(BASE_ADDR_OFF+0x6)

#define BUSY_BIT 0x1

void i2c_write(u8 addr, u8 reg, u8 data) {
    addr <<= 1;

    map_page_zero(ADDRESS_PAGE);

    while (*STATUS_REG & BUSY_BIT);

    *ADDR_REG = addr;
    *REG_REG = reg;
    *DATA_REG = data;

    *STATUS_REG = 0; // write to start operation

    map_page_zero(ILLEGAL_PAGE);
}

int i2c_read(u8 addr, u8 reg) {
    addr <<= 1;
    addr |= 1;

    map_page_zero(ADDRESS_PAGE);

    while (*STATUS_REG & BUSY_BIT);

    *ADDR_REG = addr;
    *REG_REG = reg;

    *STATUS_REG = 0; // write to start operation

    map_page_zero(ILLEGAL_PAGE);
// to nie dziala naprawic
    return 0;
}

struct i2c_drv_dat {
    unsigned addr;
    unsigned reg;
};


static struct inode root_inode = {
    "",
    0,
    0,
    NULL,
    INODE_TYPE_DEVICE
};
struct inode* i2c_fs_get_inode(struct vnode* self, const char* path) { 
    (void)path;
    root_inode.vnode = self;
    return &root_inode;
}

ssize_t i2c_submit_request(struct vfs_async_req_t* req) {
    if (!req->file->driver_data)
        req->file->driver_data = kmalloc(sizeof(struct i2c_drv_dat)); 
   
    if (req->size) {
        struct i2c_drv_dat* dat = (struct i2c_drv_dat*) req->file->driver_data;
        if(req->type == VFS_ASYNC_TYPE_WRITE) {
            i2c_write(dat->addr, dat->reg, *(u8*)req->vbuff);
            vfs_async_finalize(req, 1);
        } else if (req->type == VFS_ASYNC_TYPE_READ) {
            int rd = i2c_read(dat->addr, dat->reg);
            if (rd < 0) {
                vfs_async_finalize(req, 0);
            } else {
                *(u8*)req->vbuff = (u8) rd;
                vfs_async_finalize(req, 1);
            }
        }
    }
   
    vfs_async_finalize(req, 0);

    return 0;
}

int i2c_fio_ctl(const struct inode* inode, unsigned type, unsigned number, struct proc_file* file, int vpid) {
    if (!file)
        return -ENOSUP;
    
    if (!file->driver_data)
        file->driver_data = kmalloc(sizeof(struct i2c_drv_dat)); 
    
    struct i2c_drv_dat* dat = (struct i2c_drv_dat*) file->driver_data;
    if (type == F_DRIVER_I2C_ADDRESS) {
        unsigned ret = dat->addr;
        dat->addr = number;
        return ret;
    } else if (type == F_DRIVER_I2C_REGISTER) {
        unsigned ret = dat->reg;
        dat->reg = number;
        return ret;
    } else {
        return -ENOSUP;
    }
}

const static struct vfs_reg reg = {
            "i2c",
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
            0,
            i2c_fs_get_inode,
            NULL,
            i2c_submit_request,
            i2c_fio_ctl
};
const struct vfs_reg* i2c_get_vfs_reg() {
    return &reg;
}

