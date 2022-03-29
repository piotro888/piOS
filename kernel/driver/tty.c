#include "tty.h"
#include <libk/types.h>
#include <driver/vga.h>

int tty_cursor_col, tty_cursor_row;

/* Advance cursor row with scroll handling */
void tty_new_line() {
    tty_cursor_row++;
    if(tty_cursor_row >= VGA_TEXT_HEIGHT) {
        vga_fast_scroll();
        tty_cursor_row = VGA_TEXT_HEIGHT-1;
    }
}

/* Handle all non-printable characters */
void tty_handle_escapes(char c) {
    switch(c) {
        case '\n':
            tty_new_line();
            tty_cursor_col = 0;
            break;
        default:
            break;
    }
}

void tty_putc(char c) {
    if(c < 32 || c >= 127) { // not-printable character
        tty_handle_escapes(c);
        return;
    }

    vga_put_char_at(c, tty_cursor_row, tty_cursor_col);

    tty_cursor_col++;
    if(tty_cursor_col >= VGA_TEXT_WIDTH) {
        tty_cursor_col = 0;
        tty_new_line();
    }
}

/* Set tty color for new text 4 msb-bg color 4 lsb-fg color */
void tty_set_color(uint8_t color){
    vga_set_color(color);
}

/* Initialize text mode VGA */
void init_tty() {
    // init gpu to text mode
    vga_init();

    tty_cursor_col = 0;
    tty_cursor_row = 0;

    vga_clear_screen();
}

void tty_puts(const char* str) {
    while(*str) {
        tty_putc(*str);
        str++;
    }
}
