#ifndef DRIVER_VGA_H
#define DRIVER_VGA_H

#include <libk/types.h>

void vga_init();

void vga_put_char_at(char c, int row, int col);

void vga_fast_scroll();
void vga_clear_line(int row);
void vga_clear_screen();
void vga_set_color(u8 color);

#define VGA_TEXT_WIDTH 106
#define VGA_TEXT_HEIGHT 48

#endif
