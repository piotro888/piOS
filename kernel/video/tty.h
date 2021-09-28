#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

void init_tty();
void tty_putc(char c);
void tty_print_buffer();

#endif