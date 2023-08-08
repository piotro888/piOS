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

/* For use in vfs. Returns unique file (inode) number or kernel error */
int tar_get_fid(char* path) {
    struct file_t* file = dir_tree_get_file(&tar_dir_tree, path);
    
    if(file == NULL)
        return -ENOTFOUND;

    return file->sector; // simply return sector number as unique file number
}

ssize_t tar_read_blocking(struct fd_info* file, int pid, void* vbuff, size_t len) {
    sd_read_adapter(0, sector_buff, file->inode, sizeof(struct tar_t), 0);

    struct tar_t* header = (struct tar_t*) sector_buff;
    
    size_t file_size = convert_octal((char*)header->size);
    if(file->seek >= file_size)
        return 0;

    if(file->seek + len > file_size)
        len = file_size - file->seek;

    int pos = file->seek % SECTOR_SIZE;
    int sector = file->inode + (file->seek / SECTOR_SIZE) + 1; // inode is header sector number
 
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
    
    file->seek += len;
    return len;
}

void tar_make_dir_tree() {
    semaphore_init(&request_wait);
    int empty_sectors = 0, sector = 0;
    struct tar_t* header = (struct tar_t*) sector_buff;
    dir_tree_init(&tar_dir_tree);

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
        struct file_t* file = kmalloc(sizeof(struct file_t));
        strcpy(file->name, (char*)header->name);
        file->size = size;
        file->type = FS_TYPE_FILE; // FIXME
        file->sector = sector;

        dir_tree_add_path(&tar_dir_tree, file);

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

void tar_init() {
    list_init(&req_list);
    semaphore_init(&list_sema);
    semaphore_init(&list_lock);
    semaphore_up(&list_lock);
    tar_make_dir_tree();
    make_kernel_thread("drv::tar", tar_driver_loop);
}

void tar_mount_sd() {
    const struct vfs_reg handles = {
            tar_get_fid,
            tar_submit_req,
            0
    };

    vfs_mount("/sd/", &handles);
}
