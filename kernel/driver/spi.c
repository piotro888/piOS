#include "spi.h"

#define SPI_IO_ADDR (uint16_t*) 0x4
#define SPI_CS_ADDR (uint16_t*) 0x5

#define READY_BIT 0x8000

uint8_t spi_tranceive(uint8_t byte) {
    volatile uint16_t* spi_io_reg = SPI_IO_ADDR;

    while (!(*spi_io_reg & READY_BIT));

    *spi_io_reg = byte;

    while (!(*spi_io_reg & READY_BIT));

    return (*spi_io_reg)&0xFF;
}

void spi_transmit(uint8_t byte) {
    spi_tranceive(byte);
}

uint8_t spi_receive() {
    return spi_tranceive(0xFF);
}

void spi_cs_set(uint8_t value, uint8_t dev_no) {
    volatile uint16_t* spi_cs_reg = SPI_CS_ADDR;
    
    if(value)
        spi_cs_mask |= (1<<dev_no);
    else {
        spi_cs_mask |= (1<<dev_no);
        spi_cs_mask ^= (1<<dev_no);
    }

    *spi_cs_reg = spi_cs_mask;
}