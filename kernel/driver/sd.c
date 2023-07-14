#include "sd.h"

#include <driver/spi.h>
#include <libk/kprintf.h>
#include <libk/con/blockq.h>
#include <libk/log.h>
#include <proc/sched.h>
#include <panic.h>


#define SD_DEVICE_NO 0

#define CMD0 0
#define CMD0_CRC 0x4A
#define CMD8 8
#define CMD8_PARAM 0x1AA
#define CMD8_CRC 0x43
#define ACMD_PRE_CMD55 55
#define ACMD41 41
#define ACMD41_ARG_HC 0x4000
#define CMD58 58
#define CMD16 16
#define CMD17 17

#define STATUS_NULL 0x0
#define STATUS_IDLE 0x1
#define STATUS_ILLEGAL_CMD 0x5

#define SD_RESP_TIMEOUT 16
#define SD_INIT_TIMEOUT 200

#define SD_READ_TOKEN 0xFE

#define OCR_1_3V3 0x10
#define OCR_0_HC 0x80
#define CMD8_3_VOLATAGE 0x1

#define SD_REQ_QUEUE_SIZE 32

struct blockq requests;

void sd_command(uint8_t index, uint16_t arg_upper, uint16_t arg_lower, uint8_t crc, uint8_t* resp_buff, uint8_t resp_size) {
    spi_transmit(0x40 | index);
    spi_transmit(arg_upper >> 8);
    spi_transmit(arg_upper & 0xFF);
    spi_transmit(arg_lower >> 8);
    spi_transmit(arg_lower & 0xFF);
    spi_transmit((crc << 1) | 1);

    // wait for first byte
    uint8_t response = 0xFF, timeout = SD_RESP_TIMEOUT; 
    while(response == 0xFF) {
        response = spi_receive();
        if(--timeout == 0)
            panic("driver/sd: command timeout");
    }

    *resp_buff = response;
    // R3/R7 responses
    for(int i=0; i<resp_size-1; i++) {
        *(++resp_buff) = spi_receive();
    }

    spi_transmit(0xFF); // some cards need additional byte after command
}

void sd_hc_init() {
    uint8_t r;
    uint8_t rb[5];

    // ACMD41 sequence with HCSD arg
    uint8_t timeout = SD_INIT_TIMEOUT;
    while (timeout--) {
        sd_command(ACMD_PRE_CMD55, 0, 0, 0, &r, 1);
        if(r != STATUS_IDLE)
            panic("driver/sd: illegal response to cmd55 (acmd41)");
        
        sd_command(ACMD41, ACMD41_ARG_HC, 0, 0, &r, 1);
        if(r == STATUS_NULL) // card initialized (exited idle state)
            break;
        else if(r != STATUS_IDLE)
            panic("driver/sd: illegal response to acmd41");
        else if(timeout == 0)
            panic("driver/sd: timeout while intializing");
    }

    sd_command(CMD58, 0, 0, 0, rb, 5);
    if(rb[0] != STATUS_NULL)
        panic("driver/sd: illegal response to cmd58");
    if(!(rb[2] & OCR_1_3V3))
        panic("driver/sd: volatage range not supported");
    if(!(rb[1] & OCR_0_HC))
        panic("driver/sd: not hc card");

    // set block size to 512
    sd_command(CMD16, 0, SECTOR_SIZE, 0, &r, 1);
    if(r != STATUS_NULL)
        panic("driver/sd: illegal response to cmd16");
}

void sd_read_block(uint8_t* buff, uint16_t addr) {
    uint8_t r;

    sd_command(CMD17, 0, addr, 0, &r, 1);
    if(r != STATUS_NULL) {
       kprintf("REF: %d", r);
       panic("driver/sd: invalid response to read");
    }

    r = spi_receive();
    while(r != SD_READ_TOKEN) { // blocking polling. Maybe worth setting IRQ for non-blocking IO in future?
        r = spi_receive();
        if(r != 0xFF && r != SD_READ_TOKEN)
            panic("driver/sd: data before read token"); // if that is true sd_command might consumed token
    }

    for(int i=0; i<SECTOR_SIZE; i++) {
        *(buff++) = spi_receive();
    }

    // not using CRC, only cosuming bits
    spi_receive(); spi_receive();

    spi_transmit(0xFF);
}

__attribute__((noreturn))
void sd_driver_loop() {
    for(;;) {
        struct sd_driver_req req;
        blockq_pop(&requests, &req);
        sd_read_block(req.buff, req.block_nr);
        semaphore_up(req.notify_done);
    }
}

void sd_submit_request(struct sd_driver_req req) {
    blockq_push(&requests, &req);
}

void sd_init() {
    uint8_t r;
    uint8_t rb[5];

    spi_cs_set(1, SD_DEVICE_NO); // disable chip select
    for(int i=0; i<80; i++) {
        spi_transmit(0xFF);
    }
    spi_cs_set(0, SD_DEVICE_NO);
    // spi mode enabled

    sd_command(CMD0, 0, 0, CMD0_CRC, &r, 1);
    if(r != STATUS_IDLE) {
        kprintf("driver/sd: invalid response to CMD0: %d", r);
        panic("driver/sd: sd card failed to initialize");
    }

    sd_command(CMD8, 0, CMD8_PARAM, CMD8_CRC, rb, 5);
    if(rb[0] == STATUS_IDLE) {
        if(rb[3] != CMD8_3_VOLATAGE || rb[4] != (CMD8_PARAM&0xff)) // voltage range and check pattern
            panic("driver/sd: invalid response to cmd8");

        sd_hc_init();
    } else if (rb[0] == STATUS_ILLEGAL_CMD) {
        panic("driver/sd: low capacity cards not implemented");
    } else {
        kprintf("driver/sd: invalid response to CMD8: 0x%x\n", rb[0]);
        panic("driver/sd: sd card failed to initialize");
    }

    kprintf("driver/sd: sd card initialized\n");
}

void sd_register_thread() {
    blockq_init(&requests, SD_REQ_QUEUE_SIZE, sizeof(struct sd_driver_req));
    make_kernel_thread("drv::SD", sd_driver_loop);
}
