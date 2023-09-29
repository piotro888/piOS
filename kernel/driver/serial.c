#include "serial.h"

#include <proc/virtual.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>

#define BASE_ADDR_OFF 0x000
#define BASE_ADDR2_OFF 0x008

#define ADDRESS_PAGE 0x4

#define STATUS_REG(base) (volatile u8*)((base)+0x0)
#define RX_REG(base) (volatile u8*)((base)+0x2)
#define TX_REG(base) (volatile u8*)((base)+0x4)

#define STATUS_RX_READY 0x1
#define STATUS_TX_READY 0x2

// TODO: IRQ

// transmit function used in early logging (before init of proc or vfs)
void serial_direct_putc(char c) {
    map_page_zero(ADDRESS_PAGE);

    while (!((*STATUS_REG(BASE_ADDR_OFF)) & STATUS_TX_READY));

    *TX_REG(BASE_ADDR_OFF) = c;
    map_page_zero(ILLEGAL_PAGE);
}

void serial2_direct_putc(char c) {
    if (c == '\e')
        return;

    map_page_zero(ADDRESS_PAGE);

    while (!((*STATUS_REG(BASE_ADDR2_OFF)) & STATUS_TX_READY));

    *TX_REG(BASE_ADDR2_OFF) = c;
    map_page_zero(ILLEGAL_PAGE);
}


static struct inode root_inode = {
    "",
    0,
    0,
    NULL,
    INODE_TYPE_DEVICE
};
struct inode* serial_fs_get_inode(struct vnode* self, const char* path) { 
    (void)path;
    root_inode.vnode = self;
    return &root_inode;
}

ssize_t serial_submit_request(struct vfs_async_req_t* req) {
    if(req->type != VFS_ASYNC_TYPE_WRITE)
        return -ENOSUP;

    for (size_t i = 0; i < req->size; i++) {
        if ((int)req->file->inode->vnode->reg->vnode_data)
            serial2_direct_putc(((char*)req->vbuff)[i]);
        else
            serial_direct_putc(((char*)req->vbuff)[i]);
    }

    vfs_async_finalize(req, req->size);
    return 0;
}

const static struct vfs_reg reg_1 = {
            "serial",
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
            0,
            serial_fs_get_inode,
            NULL,
            serial_submit_request
};
const static struct vfs_reg reg_2 = {
            "serial",
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY,
            (void*) 1,
            serial_fs_get_inode,
            NULL,
            serial_submit_request
};
const struct vfs_reg* serial_get_vfs_reg(int devno) {
    if (devno)
        return &reg_2;
    else
        return &reg_1;
}
