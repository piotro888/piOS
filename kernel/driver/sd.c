#include "sd.h"

#include <driver/spi.h>
#include <libk/types.h>
#include <libk/kprintf.h>
#include <panic.h>

#define SD_DEVICE_NO 0

#define CMD0 0
#define CMD0_CRC 0x4A
#define CMD8 8
#define CMD8_CRC 0x43

#define STATUS_IDLE 0x1
#define STATUS_ILL 0x3

// ak <3

/* 
    TODO:
    assert responses
    timeouts
    separate functions for r1, r3/7 (without waiting)
*/

void sd_get_response(uint8_t* buff, size_t len) { 
    while(len--) {
        int response = 0xFF;
        while(response == 0xFF)
            response = spi_receive();
        *buff = response;
        buff++;   
    }
}

uint8_t sd_command(uint8_t index, uint16_t arg_upper, uint16_t arg_lower, uint8_t crc) {
    spi_transmit(0x40 | index);
    spi_transmit(arg_upper >> 8);
    spi_transmit(arg_upper & 0xFF);
    spi_transmit(arg_lower >> 8);
    spi_transmit(arg_lower & 0xFF);
    spi_transmit((crc << 1) | 1);

    uint8_t response;
    sd_get_response(&response, 1);

    return response;
}

void sd_end_command() {
    spi_transmit(0xFF); // some cards need additional 0xFF after command 
}

void sd_hc_init() {
    uint8_t r;
    kprintf("SD HC CARD DETECTED\n");
    
    for(int i=0; i<4; i++) {
        kprintf("%x ", spi_receive());
    }
    sd_end_command();

    // ACMD41 sequence with HCSD arg
    while (1) {
        r = sd_command(55, 0, 0, 0);
        sd_end_command();
        kprintf("cmd55 response : 0x%x\n", r);
        r = sd_command(41, 0x4000, 0, 0);
        sd_end_command();
        kprintf("cmd41 response : 0x%x\n", r);
        if(r == 0)
            break;
    }

    r = sd_command(58, 0, 0, 0);
    sd_end_command();
    kprintf("response : 0x%x\n", r);
}

void sd_init() {
    uint8_t r;
    
    spi_cs_set(1, SD_DEVICE_NO); // disable chip select
    for(int i=0; i<74; i++) {
        spi_transmit(0xFF);
    }
    spi_cs_set(0, SD_DEVICE_NO);
    // spi mode enabled

    r = sd_command(CMD0, 0, 0, CMD0_CRC);
    sd_end_command();
    if(r != 0x1) {
        kprintf("driver/sd: invalid response to CMD0: %d", r);
        panic("driver/sd: sd card failed to initialize");
    }
    kprintf("sd intit stage1 ok\n");

    r = sd_command(CMD8, 0, 0x1AA, CMD8_CRC);
    if(r == 0x1) {
        sd_hc_init();
    } else if (r == 0x5) {
        //sd_lc_init();
    } else {
        kprintf("driver/sd: invalid response to CMD8: 0x%x\n", r);
        panic("driver/sd: sd card failed to initialize");
    }
    
    kprintf("driver/sd: sd card initialized");
}