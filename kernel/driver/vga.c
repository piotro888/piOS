#include "vga.h"
#include <libk/types.h>

#define VGA_FRAMEBUFF_ADDR 0x1000
#define VGA_SETTINGS_ADDR 0x4000
#define VGA_FAST_SCROLL_ADDR 0x4001

#define VGA_TEXT_106_48_MODE 1

static int vga_text_color;
static int vga_scroll_begin;

#define REAL_ROW(x) ((x)+vga_scroll_begin < VGA_TEXT_HEIGHT ? (x)+vga_scroll_begin : (x)+vga_scroll_begin-VGA_TEXT_HEIGHT)

void vga_put_char_at(char c, int row, int col) {
    u16 char_value = (u16) c | (vga_text_color<<8);
    u16* vga_char_addr = (u16*) (VGA_FRAMEBUFF_ADDR + (REAL_ROW(row)*VGA_TEXT_WIDTH) + col);
    *vga_char_addr = char_value;
}

/* Set tty color for new text 4 msb-bg color 4 lsb-fg color */
void vga_set_color(u8 color) {
    vga_text_color = color;
}

void vga_clear_line(int row) {
    u16* vga_ptr = (u16*) (VGA_FRAMEBUFF_ADDR + (REAL_ROW(row)*VGA_TEXT_WIDTH));
    for(int i=0; i<VGA_TEXT_WIDTH; i++) {
        *vga_ptr = 0;
        vga_ptr = (u16*)(((u16)vga_ptr)+1);
    }
}

void vga_clear_screen() {
    u16* vga_ptr = (u16*) VGA_FRAMEBUFF_ADDR;
    for(int i=0; i<VGA_TEXT_WIDTH*VGA_TEXT_HEIGHT; i++) {
        *vga_ptr = 0;
        vga_ptr = (u16*)(((u16)vga_ptr)+1);
    }
}

void vga_fast_scroll() {
    vga_scroll_begin = (vga_scroll_begin+1 >= VGA_TEXT_HEIGHT ? 0 : vga_scroll_begin+1);
    *(u16*) VGA_FAST_SCROLL_ADDR = vga_scroll_begin;
    vga_clear_line(VGA_TEXT_HEIGHT-1);
}

void vga_init() {
    u16* vga_settings = (u16*) VGA_SETTINGS_ADDR;
    *vga_settings = VGA_TEXT_106_48_MODE;

    vga_text_color = 0x0F;

    vga_scroll_begin = 0;
    u16* vga_fast_scroll = (u16*) VGA_FAST_SCROLL_ADDR;
    *vga_fast_scroll = vga_scroll_begin;
}
