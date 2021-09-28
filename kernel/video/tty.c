#include "tty.h"
#include <libk/types.h>

#define TTY_WIDTH 106
#define TTY_HEIGHT 48
#define VGA_FRAMEBUFF_ADDR 0x1000

int tty_buffer[TTY_HEIGHT][TTY_WIDTH];

int tty_text_color;
int tty_cursor_col, tty_cursor_row;

// copy internal framebuffer to screen [flush]
void tty_print_buffer() {
    // begin of vga framebuffer
    //int* vga_ptr = (int*) VGA_FRAMEBUFF_ADDR;
    int addr = VGA_FRAMEBUFF_ADDR;

    for(int i=0; i<TTY_HEIGHT; i++) {
        for(int j=0; j<TTY_WIDTH; j++){
            *((int*)addr) = tty_buffer[i][j];
            addr++;
            /* incrementing pointers is  currently broken in pcpu-gcc port
            this line increments pointer by exactly 1 address */
            //vga_ptr = (int*)(((void*)vga_ptr)+1);
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
    tty_buffer[tty_cursor_row][tty_cursor_col] = (int) c | (tty_text_color<<8);

    tty_cursor_col++;
    if(tty_cursor_col == TTY_WIDTH) {
        tty_cursor_col = 0;
        tty_cursor_row++;
        if(tty_cursor_row == 3) {
            tty_scroll();
            tty_cursor_row--;
        }
    }
    //tty_print_buffer();
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
}

