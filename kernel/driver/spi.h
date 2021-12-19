#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <libk/types.h>

void spi_transmit(uint8_t byte);
uint8_t spi_receive();
void spi_cs_set(uint8_t value, uint8_t dev_no);

uint8_t spi_tranceive(uint8_t byte);

uint16_t spi_cs_mask;

#endif