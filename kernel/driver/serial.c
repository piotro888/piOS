#include "serial.h"

#include <proc/virtual.h>

#define BASE_ADDR_OFF 0x000
#define ADDRESS_PAGE 0x4

#define STATUS_REG (volatile u8*)((BASE_ADDR_OFF)+0x0)
#define RX_REG (volatile u8*)((BASE_ADDR_OFF)+0x2)
#define TX_REG (volatile u8*)((BASE_ADDR_OFF)+0x4)

#define STATUS_RX_READY 0x1
#define STATUS_TX_READY 0x2

// TODO: IRQ

// transmit function used in early logging (before init of proc or vfs)
void serial_direct_putc(char c) {
    map_page_zero(ADDRESS_PAGE);

    while (!((*STATUS_REG) & STATUS_TX_READY));

    *TX_REG = c;
    map_page_zero(ILLEGAL_PAGE);
}

