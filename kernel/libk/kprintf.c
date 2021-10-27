#include "kprintf.h"
#include <video/tty.h>

/* stdarg from gcc library */
#include <stdarg.h>

void print_number(unsigned int num) {
    char buff[20];
    int pos = 0;
    while(num) {
        buff[pos++] = '0' + (char)(num%10);
        num /= 10;
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
                    print_number(i_param);
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