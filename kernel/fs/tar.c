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
       
        if(strncmp((char*)header->ustarzz, "ustar", 5))
            panic("tarFS: invalid header");

        if(strcmp(header->name, name) == 0) //TODO: long file names (name_prefix)
            return header;
        
        int sectors_skip = (convert_octal(header->size)+SECTOR_SIZE-1)/SECTOR_SIZE + 1;
        header = (void*) header + SECTOR_SIZE*sectors_skip;
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
    
    size_t file_size = convert_octal(files[fd].header->size);
    if(files[fd].seek >= file_size)
        return 0;

    if(files[fd].seek+size > file_size)
        size = file_size - files[fd].seek;

    char* data = (char*) files[fd].header + SECTOR_SIZE + files[fd].seek;

    char* buffc = (char*) buff;
    for(size_t i=0; i<size; i++) {
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
    int fd_1 = open("boot.s");
    kprintf("fd: %d", fd_1);
    int fd_2 = open("panic.c");
    kprintf("\nfd_p: %d", fd_2);
    close(fd_1);
    int fd_3 = open("boot.s");
    kprintf("\nfd3: %d", fd_3);
    kprintf("Reading boot.s:\n");
    char buff[256];
    int rs = read(fd_3, buff, 128);
    kprintf("Read %d chars: %s", rs, buff);
    rs = read(fd_3, buff, 10);
    buff[11] = '\0';
    kprintf("\nand 10 more...\n %s", buff);
}