#include "kprintf.h"
#include <driver/tty.h>

/* stdarg from gcc library */
#include <stdarg.h>

void print_number(unsigned int num, unsigned int base) {
    char buff[20];
    int pos = 0;

    if(!num)
        buff[pos++] = '0';

    while(num) {
        if(num % base < 10)
            buff[pos++] = '0' + (char)(num%base);
        else
            buff[pos++] = 'a' + (char)((num%base)-10);
        num /= base;
    }

    for(int i=0; i<pos/2; i++) {
        char swp = buff[i];
        buff[i] = buff[pos-i-1];
        buff[pos-i-1] = swp;
    }
    buff[pos] = '\0';
    tty_puts(buff);
}

/* basic printf */
int kprintf(const char* str, ...) {
    va_list vlist;
    va_start(vlist, str);
    int len = 0;

    while(*str) {
        if(*str == '%') {
            str++;
            
            switch (*str) {
                case 'd':
                case 'i':
                    str++;
                    int i_param = va_arg(vlist, int);

                    if(i_param < 0) {
                        tty_putc('-');
                        i_param *= -1;
                    }
                    
                    print_number((unsigned int) i_param, 10);
                    break;
                case 'u':
                    str++;
                    i_param = va_arg(vlist, int);
                    print_number(i_param, 10);
                    break;
                case 'x':
                    str++;
                    int x_param = va_arg(vlist, int);
                    print_number(x_param, 16);
                    break;
                case 's':
                    str++;
                    char* s_param = va_arg(vlist, char*);
                    tty_puts(s_param);
                    break;
                case 'c':
                    str++;
                    char c_param = (char)va_arg(vlist, int);
                    tty_putc(c_param);
                    break;
                default:
                    tty_putc('e');
                    break;
            }
        } else {
            tty_putc(*str);
            str++;
        }
    }
 
    va_end(vlist);
    return len;
}