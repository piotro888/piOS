#include "tty.h"
#include <libk/types.h>

#define TTY_WIDTH 106
#define TTY_HEIGHT 48/2
#define VGA_FRAMEBUFF_ADDR 0x1000

int tty_buffer[TTY_HEIGHT][TTY_WIDTH];

int tty_text_color;
int tty_cursor_col, tty_cursor_row;

// copy internal framebuffer to screen [flush]
void tty_print_buffer() {
    // begin of vga framebuffer
    int* vga_ptr = (int*) VGA_FRAMEBUFF_ADDR;

    for(int i=0; i<TTY_HEIGHT; i++) {
        for(int j=0; j<TTY_WIDTH; j++){
            *(vga_ptr) = tty_buffer[i][j];
 
            /* incrementing hardware pointers is currently broken in pcpu-gcc port
            this line increments pointer by exactly 1 address */
            vga_ptr = (int*)(((int)vga_ptr)+1);
        }
    }
}

// scroll screen one line down
void tty_scroll() {
    for(int i=1; i<TTY_HEIGHT; i++) {
        for(int j=0; j<TTY_WIDTH; j++){
            tty_buffer[i-1][j] = tty_buffer[i][j];
        }
    }
}

void tty_putc(char c) {
    // 4 msb - bg color | 4 b - fg color | 8 lsb - char code
    int char_value = (int) c | (tty_text_color<<8);

    tty_buffer[tty_cursor_row][tty_cursor_col] = char_value;

    // write changes immediatly to screen, use buffer only for scrolling (not needed for escape codes too)
    int* vga_char_addr = (int*) (VGA_FRAMEBUFF_ADDR + (tty_cursor_row*TTY_WIDTH) + (tty_cursor_col));
    *vga_char_addr = char_value;

    tty_cursor_col++;
    if(tty_cursor_col == TTY_WIDTH) {
        tty_cursor_col = 0;
        tty_cursor_row++;
        if(tty_cursor_row == TTY_HEIGHT) {
            tty_scroll();
            tty_cursor_row--;
            tty_print_buffer();
        }
    }
}

/* Set tty color for new text 4 msb-bg color 4 lsb-fg color */
void tty_set_color(uint8_t color){
    tty_text_color = color;
}

void init_vga() {
    int* vga_settings = (int*) 0x4000;
    *vga_settings = 1; // 106*48 text mode
}

/* Initialize text mode VGA */
void init_tty() {
    // clear text buffer
    for(int i=0; i<TTY_HEIGHT; i++){
        for(int j=0; j<TTY_WIDTH; j++){
            tty_buffer[i][j] = 0;
        }
    }
    // init gpu to text mode
    init_vga();
    // white text black bg
    tty_text_color = 0x0F;
    tty_cursor_col = 0;
    tty_cursor_row = 0;
    tty_print_buffer();
}

void tty_puts(const char* str) {
    while(*str) {
        tty_putc(*str);
        str++;
    }
}