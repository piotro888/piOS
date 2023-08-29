#ifndef DRIVER_KEYBOARD_H
#define DRIVER_KEYBOARD_H

#include <libk/types.h>

#define SCANCODE_ADDR (volatile u8*) 0x40
#define SCANCODE_PAGE 0x4
#define KEYBOARD_IRQ_ID 1

void print_scancode(uint8_t scancode);
void keyboard_init();

#endif
