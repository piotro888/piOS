#include "tar.h"
#include <libk/kprintf.h>
#include <libk/string.h>
#include <driver/sd.h>
#include <panic.h>

#define SECTOR_SIZE 512
#define MAX_FILES 16

struct fd_t {
    unsigned int sector;
    unsigned int seek;
};

struct fd_t files[MAX_FILES];
uint16_t in_use;

char sector_buff[SECTOR_SIZE];

int convert_octal(char* octal)  {
    int res = 0;
    for(int i = 0; i < 11; i++) {
        int cint = (int)(*(octal+i)-'0');
        res *= 8;
        res += cint;
    }
    return res;
}

int search_file(char* name) {
    int empty_sectors = 0, sector = 0;
    struct tar_t* header = (struct tar_t*) sector_buff;

    while (empty_sectors < 2) {
        sd_read_block(sector_buff, sector);

        if(header->name[0] == '\0') {
            empty_sectors++;
            sector++;
            continue;
        }

        if(strncmp((char*)header->ustarzz, "ustar", 5))
            panic("tarFS: invalid header");

        if(strcmp(header->name, name) == 0) //TODO: long file names (name_prefix)
            return sector;
        
        int sectors_skip = (convert_octal(header->size)+SECTOR_SIZE-1)/SECTOR_SIZE + 1;
        sector += sectors_skip;
    }
    return -1;
}

int8_t open(char* path) {
    int sector = search_file(path);
    
    if(sector == -1)
        return -ENOTFOUND;
    
    int fd = -1;
    for(int i=0; i<MAX_FILES; i++) {
        if(!(in_use & (1<<i))) {
            in_use |= (1<<i);
            fd = i;
            files[i].sector = sector;
            files[i].seek = 0;
            break;
        }
    }
    if(fd == -1)
        return -ETOOMANYFILES;
    return fd;
}

int8_t close(uint8_t fd) {
    if(fd >= MAX_FILES || !(in_use & (1<<fd)))
        return -EINVALIDFD;
    
    in_use ^= (1<<fd);
    
    return 0;
}

size_t read(int8_t fd, void* buff, size_t size) {
    if(fd >= MAX_FILES || !(in_use & (1<<fd)))
        return -EINVALIDFD;

    sd_read_block(sector_buff, files[fd].sector);

    struct tar_t* header = sector_buff;
    
    size_t file_size = convert_octal(header->size);
    if(files[fd].seek >= file_size)
        return 0;

    if(files[fd].seek+size > file_size)
        size = file_size - files[fd].seek;
    
    int pos = files[fd].seek % SECTOR_SIZE;
    int sector = files[fd].sector + (files[fd].seek / SECTOR_SIZE) + 1;
    char* data = sector_buff + pos;
    sd_read_block(sector_buff, sector);

    char* buffc = (char*) buff;
    for(size_t i=0; i<size; i++) {
        if(pos++ == 512) {
            sd_read_block(sector_buff, ++sector);
            pos = 0;
        }
        *buffc++ = *data++;
    }
    
    files[fd].seek += size;
    return size;
}

size_t seek(int8_t fd, size_t seek) {
    if(fd >= MAX_FILES || !(in_use & (1<<fd)))
        return -EINVALIDFD;

    files[fd].seek = seek;
    
    return seek;
}

void tar_test() {
    int fd_1 = open("./boot.s");
    kprintf("fd: %d", fd_1);
    int fd_2 = open("./panic.c");
    kprintf("\nfd_p: %d", fd_2);
    close(fd_1);
    int fd_3 = open("./boot.s");
    kprintf("\nfd3: %d", fd_3);
    kprintf("Reading boot.s:\n");
    char buff[256];
    int rs = read(fd_3, buff, 128);
    kprintf("Read %d chars: %s", rs, buff);
    rs = read(fd_3, buff, 10);
    buff[11] = '\0';
    kprintf("\nand 10 more...\n %s", buff);
}