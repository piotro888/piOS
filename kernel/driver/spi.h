#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <libk/types.h>

void spi_init();

void spi_transmit(u8 byte);
u8 spi_receive();
u8 spi_cs_set(u8 value, u8 dev_no);

u8 spi_tranceive(u8 byte);

#endif