#include "serial.h"
#include <proc/virtual.h>
#include <proc/sched.h>

#define BASE_ADDR_OFF 0x000

#define RX_REG (volatile u8*)((BASE_ADDR_OFF)+0x0)
#define TX_REG (volatile u8*)((BASE_ADDR_OFF)+0x1)
#define STATUS_REG (volatile u8*)((BASE_ADDR_OFF)+0x2)
#define SETTINGS_REG (volatile u8*)((BASE_ADDR_OFF)+0x2)

#define STATUS_RX_READY 0x1
#define STATUS_TX_READY 0x2

// TODO: IRQ
// FIXME: MAP PAGE 0 IS NOT REMEMBERED AFTER CONTEXT SWITCH after map_page_0

// transmit function used in early logging (before init of proc or vfs)
void serial_direct_putc(char c) {
    // FIXME_LATER: MAP DEVICE PAGE
    while (!((*STATUS_REG) & STATUS_TX_READY));

   *TX_REG = c; 
}

