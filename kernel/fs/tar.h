#ifndef FS_TARFS_H
#define FS_TARFS_H

#include <libk/types.h>

/* direct tar file structure */
/* Unix Standard Tape ARchive */
struct tar_t {
    uint8_t name[100];
    uint8_t mode[8];
    uint8_t owner_uid[8];
    uint8_t owner_gid[8];
    uint8_t size[12];
    uint8_t mod_time[12];
    uint8_t checksum[8];
    uint8_t link;
    uint8_t link_name[100];
    uint8_t ustarzz[8];
    uint8_t owner_name[32];
    uint8_t owner_group[32];
    uint8_t dev_major[8];
    uint8_t dev_minor[8];
    uint8_t name_prefix[155];
};

uint8_t open(char* path);

#endif