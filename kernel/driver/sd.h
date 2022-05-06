#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#include <libk/types.h>

#define SECTOR_SIZE 512

void sd_init();
void sd_register_thread();

void sd_read_block(uint8_t* buff, uint16_t addr);

#include <libk/con/semaphore.h>
struct sd_driver_req {
    u8* buff;
    size_t block_nr;
    struct semaphore* notify_done;
};
void sd_submit_request(struct sd_driver_req req);

#endif
