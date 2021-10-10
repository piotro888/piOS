#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <libk/types.h>

void init_tty();
void tty_putc(char c);
void tty_puts(const char* s);
void tty_set_color(uint8_t color);
void tty_print_buffer();

#endif