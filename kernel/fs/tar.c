#include "tar.h"
#include <libk/kprintf.h>
#include <libk/string.h>
#include <libk/kmalloc.h>
#include <fs/dirtree.h>
#include <fs/vfs.h>
#include <driver/sd.h>
#include <panic.h>

#define SECTOR_SIZE 512

char sector_buff[SECTOR_SIZE];

struct dir_t_node tar_dir_tree;
struct semaphore request_wait;

int convert_octal(char* octal)  {
    int res = 0;
    for(int i = 0; i < 11; i++) {
        int cint = (int)(*(octal+i)-'0');
        res *= 8;
        res += cint;
    }
    return res;
}

void sd_read_adapter(char* buff, size_t block) {
    struct sd_driver_req req = {
            buff,
            block,
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

ssize_t tar_read(struct fd_info* file, void* buff, size_t len) {
    sd_read_adapter(sector_buff, file->inode);

    struct tar_t* header = sector_buff;
    
    size_t file_size = convert_octal(header->size);
    if(file->seek >= file_size)
        return 0;

    if(file->seek + len > file_size)
        len = file_size - file->seek;

    int pos = file->seek % SECTOR_SIZE;
    int sector = file->inode + (file->seek / SECTOR_SIZE) + 1; // inode is header sector number
    char* data = sector_buff + pos;
    sd_read_adapter(sector_buff, sector);

    char* buffc = (char*) buff;
    for(size_t i=0; i<len; i++) {
        if(pos++ == 512) {
            sd_read_adapter(sector_buff, ++sector);
            data = sector_buff;
            pos = 0;
        }
        *buffc++ = *data++;
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
        sd_read_adapter(sector_buff, sector);

        if(header->name[0] == '\0') {
            empty_sectors++;
            sector++;
            continue;
        }

        if(strncmp((char*)header->ustarzz, "ustar", 5)) {
            kprintf("tarFS: invalid header at sector %d", sector);
            panic("tarFS: invalid header");
        };
        
        size_t size = convert_octal(header->size);
        struct file_t* file = kmalloc(sizeof(struct file_t));
        strcpy(file->name, header->name);
        file->size = size;
        file->type = FS_TYPE_FILE; // FIXME
        file->sector = sector;

        dir_tree_add_path(&tar_dir_tree, file);

        int sectors_skip = (size+SECTOR_SIZE-1)/SECTOR_SIZE + 1;
        sector += sectors_skip;
    }
}

ssize_t tar_write(struct fd_info* file, void* buff, size_t len) {
    (void) file, (void) buff, (void) len;
    return -ENOSUP;
}

void tar_mount_sd() {
    const struct vfs_reg handles = {
            tar_get_fid,
            tar_read,
            tar_write,
            NULL,
            NULL,
    };

    vfs_mount("/sd/", &handles);
}
