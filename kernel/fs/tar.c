#include "tar.h"
#include <libk/kprintf.h>
#include <libk/string.h>
#include <panic.h>

#define SECTOR_SIZE 0x200
#define TEMP_TAR_BEGIN 0xb000
#define MAX_FILES 16

struct fd_t {
    struct tar_t* header;
    unsigned int seek;
};

struct fd_t files[MAX_FILES];
uint16_t in_use;

int convert_octal(char* octal)  {
    int res = 0;
    for(int i = 0; i < 11; i++) {
        int cint = (int)(*(octal+i)-'0');
        res *= 8;
        res += cint;
    }
    return res;
}

void* search_file(char* name) {
    int empty_sectors = 0;
    struct tar_t* header = (struct tar_t*) TEMP_TAR_BEGIN; 

    while (empty_sectors < 2) {
        if(header->name[0] == '\0') {
           empty_sectors++;
           header = (void*) header + SECTOR_SIZE;
           continue;
        }

        kprintf("addr:%d", header->ustarzz);
        kprintf("name:%s", header->name);

        if(strncmp((char*)header->ustarzz, "ustar", 5))
            panic("tarFS: invalid header");

        if(strcmp(header->name, name) == 0) //TODO: long file names (name_prefix)
            return header;

        // 1 sector for header + ceil(size)
        int sectors_skip = (convert_octal(header->size)+SECTOR_SIZE-1)/SECTOR_SIZE + 1;
        kprintf(" content %d", convert_octal(header->size));
        header = (void*) header + SECTOR_SIZE*sectors_skip;
        kprintf("CH %d", header);
    }
    return NULL;
}

int8_t open(char* path) {
    struct tar_t* file_header = search_file(path);
    
    if(file_header == NULL)
        return -ENOTFOUND;
    
    int fd = -1;
    for(int i=0; i<MAX_FILES; i++) {
        if(!(in_use & (1<<i))) {
            in_use |= (1<<i);
            fd = i;
            files[i].header = file_header;
            files[i].seek = 0;
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