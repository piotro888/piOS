#include "spi.h"

#include <proc/virtual.h>

#define DEVICE_PAGE 0x4
#define SPI_TX (volatile u8*) 0x20
#define SPI_RX (volatile u8*) 0x22
#define SPI_STATUS (volatile u8*) 0x24
#define SPI_SETTINGS (volatile u8*) 0x26
#define SPI_CS (volatile u8*) 0x28

#define BUSY_BIT 0x1

void spi_init() {
    map_page_zero(DEVICE_PAGE);
    // SPI mode 0 (CPOL=0 CPHA=0)
    // Clock div 5 (200kHz)
    *SPI_SETTINGS = (0x0 << 4) | 0x4;
    map_page_zero(ILLEGAL_PAGE);
}

u8 spi_tranceive(uint8_t byte) {
    map_page_zero(DEVICE_PAGE);

    while ((*SPI_STATUS & BUSY_BIT));

    *SPI_TX = byte;

    while ((*SPI_STATUS & BUSY_BIT));

    u8 resp = *SPI_RX;

    map_page_zero(ILLEGAL_PAGE);
    return resp;
}

void spi_transmit(u8 byte) {
    spi_tranceive(byte);
}

u8 spi_receive() {
    return spi_tranceive(0xFF);
}

u8 spi_cs_set(u8 value, u8 dev_no) {
    map_page_zero(DEVICE_PAGE);
    if(value)
        *SPI_CS |= (1<<dev_no);
    else
        *SPI_CS &= ~(1<<dev_no);

    u8 ret = *SPI_CS;
    map_page_zero(ILLEGAL_PAGE);
    return ret;
}
