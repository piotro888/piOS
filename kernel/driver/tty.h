#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <libk/types.h>

void tty_init_basic();
void tty_init_driver();
void tty_mnt_vfs();
void tty_register_thread();

void tty_putc(char c);
void tty_puts(const char* s);
void tty_direct_write(char* buff, size_t len);

void tty_submit_char(char c);

void tty_set_color(uint8_t color)  __attribute__((deprecated));

#endif
