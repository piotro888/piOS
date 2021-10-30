#include <driver/tty.h>

#define RELEASE_SCANCODE 0xF0
#define EXTENDED_SCANCODE 0xE0

const char printable_scancodes[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\t', '`', 
    ' ', ' ', ' ', ' ', ' ', 'q', '1', ' ', ' ', ' ', 'z', 's', 'a', 'w', '2', ' ',
    ' ', 'c', 'x', 'd', 'e', '4', '3', ' ', ' ', ' ', 'v', 'f', 't', 'r', '5', ' ',
    ' ', 'n', 'b', 'h', 'g', 'y', '6', ' ', ' ', ' ', 'm', 'j', 'u', '7', '8', ' ',
    ' ', ',', 'k', 'i', 'o', '0', '9', ' ', ' ', '.', '/', 'l', ';', 'p', '-', ' ',
    ' ', ' ', '\'', ' ', '[', '=', ' ', ' ', ' ', ' ', '\n', ']', ' ', '\\', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', '\b'
};

uint8_t prev_scancode = 0;

void print_scancode (uint8_t scancode) {
    if(prev_scancode != RELEASE_SCANCODE)
        tty_putc(printable_scancodes[scancode]);
    prev_scancode = scancode;
}

