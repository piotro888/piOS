#include <driver/tty.h>
#include <fs/kbd.h>

#define RELEASE_SCANCODE 0xF0
#define EXTENDED_SCANCODE 0xE0

#define LSHIFT_SCANCODE 0x12
#define RSHIFT_SCANCODE 0x59

const char printable_scancodes[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\t', '`', 
    ' ', ' ', ' ', ' ', ' ', 'q', '1', ' ', ' ', ' ', 'z', 's', 'a', 'w', '2', ' ',
    ' ', 'c', 'x', 'd', 'e', '4', '3', ' ', ' ', ' ', 'v', 'f', 't', 'r', '5', ' ',
    ' ', 'n', 'b', 'h', 'g', 'y', '6', ' ', ' ', ' ', 'm', 'j', 'u', '7', '8', ' ',
    ' ', ',', 'k', 'i', 'o', '0', '9', ' ', ' ', '.', '/', 'l', ';', 'p', '-', ' ',
    ' ', ' ', '\'', ' ', '[', '=', ' ', ' ', ' ', ' ', '\n', ']', ' ', '\\', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', '\b'
};

const char shift_scancodes[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\t', '~', 
    ' ', ' ', ' ', ' ', ' ', 'Q', '!', ' ', ' ', ' ', 'Z', 'S', 'A', 'W', '@', ' ',
    ' ', 'C', 'X', 'D', 'E', '$', '#', ' ', ' ', ' ', 'V', 'F', 'T', 'R', '%', ' ',
    ' ', 'N', 'B', 'H', 'G', 'Y', '^', ' ', ' ', ' ', 'M', 'J', 'U', '&', '*', ' ',
    ' ', '<', 'K', 'I', 'O', ')', '(', ' ', ' ', '>', '?', 'L', ':', 'P', '_', ' ',
    ' ', ' ', '"', ' ', '{', '+', ' ', ' ', ' ', ' ', '\n', '}', ' ', '|', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', '\b'
};

uint8_t prev_scancode = 0;

#define STATE_SHIFT 1
#define STATE_CAPS 2
uint8_t keyboard_state = 0;

#define KEYB_BUFF_LEN 16
char* buff[KEYB_BUFF_LEN];
short buffptr = 0;

void print_scancode (uint8_t scancode) {
    if(scancode == LSHIFT_SCANCODE || scancode == RSHIFT_SCANCODE) {
        if(prev_scancode == RELEASE_SCANCODE)
            keyboard_state &= ~STATE_SHIFT;
        else  
            keyboard_state |= STATE_SHIFT;
    } else if(prev_scancode != RELEASE_SCANCODE && scancode != RELEASE_SCANCODE) {
        char outc;
        if(keyboard_state & STATE_SHIFT)
            outc = shift_scancodes[scancode];
        else
            outc = printable_scancodes[scancode];
        
        tty_putc(outc);
        kbd_vfs_submit_char(outc);
    }
    prev_scancode = scancode;
}