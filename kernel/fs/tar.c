#include "tar.h"
#include <string.h>
#include <libk/kprintf.h>
#include <libk/kmalloc.h>
#include <libk/list.h>
#include <libk/math.h>
#include <fs/dirtree.h>
#include <fs/vfs.h>
#include <fs/vfs_async.h>
#include <proc/sched.h>
#include <driver/sd.h>
#include <panic.h>

#define SECTOR_SIZE 512

static char sector_buff[SECTOR_SIZE];

static struct dir_t_node tar_dir_tree;
static struct semaphore request_wait;

static struct list req_list;
static struct semaphore list_sema, list_lock;

int convert_octal(char* octal)  {
    int res = 0;
    for(int i = 0; i < 11; i++) {
        int cint = (int)(*(octal+i)-'0');
        res *= 8;
        res += cint;
    }
    return res;
}

void sd_read_adapter(int pid, char* vbuff, size_t block, size_t len, size_t offset) {
    struct sd_driver_req req = {
            pid,
            (u8*)vbuff,
            block,
            offset,
            len,
            &request_wait
    };
    sd_submit_request(req);
    semaphore_down(&request_wait);
}

struct inode* tar_find_inode(struct vnode* self, const char* path) {
    struct inode* res = dir_tree_get_file(&tar_dir_tree, path);
    if (res)
        res->vnode = self;
    return res;
}

ssize_t tar_read_blocking(struct proc_file* file, int pid, void* vbuff, size_t len) {
    size_t file_size = file->inode->size;
    if(file->offset >= file_size)
        return 0;

    if(file->offset + len > file_size)
        len = file_size - file->offset;

    int pos = file->offset % SECTOR_SIZE;
    int sector = file->inode->fid + (file->offset / SECTOR_SIZE) + 1; // file id is header sector number
 
    char* buffc = (char*) vbuff;

    size_t new_len = len;
    
    size_t readlen = MIN(new_len, SECTOR_SIZE-pos);
    sd_read_adapter(pid, vbuff, sector++, readlen, pos);
    new_len -= readlen;
    vbuff += readlen;

    if (new_len) {
        for(size_t i=0; i<new_len/512; i++) {
            sd_read_adapter(pid, vbuff, sector++, SECTOR_SIZE, 0);
            vbuff += SECTOR_SIZE;
            new_len -= SECTOR_SIZE;
        }
        if (new_len)
            sd_read_adapter(pid, vbuff, sector, new_len, 0);
    }
    
    file->offset += len;
    return len;
}

void tar_make_dir_tree(struct vnode* self) {
    semaphore_init(&request_wait);
    int empty_sectors = 0, sector = 0;
    struct tar_t* header = (struct tar_t*) sector_buff;
    dir_tree_init(&tar_dir_tree, self);

    while (empty_sectors < 2) {
        sd_read_adapter(0, sector_buff, sector, sizeof(struct tar_t), 0);
        if(header->name[0] == '\0') {
            empty_sectors++;
            sector++;
            continue;
        }

        if(strncmp((char*)header->ustarzz, "ustar", 5)) {
            kprintf("tarFS: invalid header at sector %d", sector);
            panic("tarFS: invalid header");
        };
        
        size_t size = convert_octal((char*)header->size);
        struct inode* inode = kmalloc(sizeof(struct inode));
        inode->fid = sector;
        inode->type = (char)header->type == '5' ? INODE_TYPE_DIRECTORY : INODE_TYPE_FILE;
        inode->size = size;
        inode->name = kmalloc(strlen((char*)header->name));
        inode->vnode = self;
        strcpy(inode->name, (char*)header->name);
        
        dir_tree_add_path(&tar_dir_tree, inode);

        int sectors_skip = (size+SECTOR_SIZE-1)/SECTOR_SIZE + 1;
        sector += sectors_skip;
    }
}

void tar_process_request(struct vfs_async_req_t* req) {
    if (req->type == VFS_ASYNC_TYPE_READ) {
        ssize_t res = tar_read_blocking(req->file, req->pid, req->vbuff, req->size);
        vfs_async_finalize(req, res);
    }
}

ssize_t tar_submit_req(struct vfs_async_req_t* req) {
    semaphore_down(&list_lock);
    if (req->type != VFS_ASYNC_TYPE_READ)
        return -ENOSUP;
    
    list_append(&req_list, req);
    semaphore_up(&list_sema);
    semaphore_up(&list_lock);
    return 0;
}

void __attribute__((noreturn)) tar_driver_loop() {
    for(;;) {
        semaphore_down(&list_sema);
        tar_process_request(req_list.first->val);
        list_remove(&req_list, req_list.first);
    }
}

void tar_init(struct vnode* vnode) {
    list_init(&req_list);
    semaphore_init(&list_sema);
    semaphore_init(&list_lock);
    semaphore_up(&list_lock);
    tar_make_dir_tree(vnode);
    make_kernel_thread("drv::tar", tar_driver_loop);
}

const static struct vfs_reg reg = {
    "tar",
    0,
    NULL,

    tar_find_inode,
    NULL,
    tar_submit_req
};

const struct vfs_reg* tar_get_vfs_reg() {
    return &reg;
}
