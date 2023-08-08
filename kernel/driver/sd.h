#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#include <libk/types.h>

#define SECTOR_SIZE 512

void sd_init();
void sd_register_thread();

void sd_read_block(unsigned int pid, uint8_t* vbuff, uint16_t addr, size_t buff_size, size_t buff_offset);

#include <libk/con/semaphore.h>
struct sd_driver_req {
    unsigned int pid;
    u8* vbuff;
    size_t block_nr;
    size_t offset; // temporary
    size_t length; // temporary
    struct semaphore* notify_done;
};
void sd_submit_request(struct sd_driver_req req);

#endif
