#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#include <libk/types.h>

void sd_init();
void sd_read_block(uint8_t* buff, uint16_t addr);

#endif