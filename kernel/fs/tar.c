#include "tar.h"
#include <libk/kprintf.h>
#include <libk/string.h>
#include <panic.h>

#define SECTOR_SIZE 0x200
#define TEMP_TAR_BEGIN 0xb000


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

uint8_t open(char* path) {
    
}