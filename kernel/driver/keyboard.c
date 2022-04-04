#include "keyboard.h"
#include <driver/tty.h>
#include <fs/kbd.h>

#define RELEASE_SCANCODE 0xF0
#define EXTENDED_SCANCODE 0xE0

#define LSHIFT_SCANCODE 0x12
#define RSHIFT_SCANCODE 0x59
#define CTRL_SCANCODE 0x14

#define ARR_UP_ESCANCODE 0x75
#define ARR_DOWN_ESCANCODE 0x72
#define ARR_LEFT_ESCANCODE 0x6B
#define ARR_RIGHT_ESCANCODE 0x74

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

u8 prev_scancode = 0;

#define STATE_SHIFT 1
#define STATE_CAPS 2
#define STATE_CTRL 4
u8 keyboard_state = 0;
int extended_scancode = 0;

void send_key(char c) {
    tty_submit_char(c);
    kbd_vfs_submit_char(c);
}

void print_scancode(u8 scancode) {
    if(scancode == 0x81) // this is buggy scancode that hardware driver sometimes produces
        return;

    if (scancode == LSHIFT_SCANCODE || scancode == RSHIFT_SCANCODE) {
        if (prev_scancode == RELEASE_SCANCODE)
            keyboard_state &= ~STATE_SHIFT;
        else
            keyboard_state |= STATE_SHIFT;
    } else if (scancode == CTRL_SCANCODE) {
        if(prev_scancode == RELEASE_SCANCODE)
            keyboard_state &= ~STATE_CTRL;
        else
            keyboard_state |= STATE_CTRL;
    } else if (scancode == EXTENDED_SCANCODE) {
        extended_scancode = 1;
    } else if (extended_scancode) {
        if(scancode != RELEASE_SCANCODE)
            extended_scancode = 0;

        if(prev_scancode != RELEASE_SCANCODE) {
            if (scancode == ARR_UP_ESCANCODE) {
                send_key('\033');
                send_key('[');
                send_key('A');
            } else if (scancode == ARR_DOWN_ESCANCODE) {
                send_key('\033');
                send_key('[');
                send_key('B');
            } else if (scancode == ARR_RIGHT_ESCANCODE) {
                send_key('\033');
                send_key('[');
                send_key('C');
            } else if (scancode == ARR_LEFT_ESCANCODE) {
                send_key('\033');
                send_key('[');
                send_key('D');
            }
        }
    } else if (prev_scancode != RELEASE_SCANCODE && scancode != RELEASE_SCANCODE) {
        char outc;
        if(keyboard_state & STATE_CTRL) {
            if ((shift_scancodes[scancode] >= '@' && shift_scancodes[scancode] <= 'Z')
                || (shift_scancodes[scancode] >= '^' && shift_scancodes[scancode] <= '_'))
                outc = shift_scancodes[scancode]-'@';
            else if (printable_scancodes[scancode] >= '[' && printable_scancodes[scancode] <= ']')
                outc = printable_scancodes[scancode]-'@';
            else if (shift_scancodes[scancode] == '?')
                outc = (char) 127;
            else
                outc = (keyboard_state & STATE_SHIFT ? shift_scancodes[scancode] : printable_scancodes[scancode]);
        } else if (keyboard_state & STATE_SHIFT)
            outc = shift_scancodes[scancode];
        else
            outc = printable_scancodes[scancode];

        send_key(outc);
    }
    prev_scancode = scancode;
}
