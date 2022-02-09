#include "kprintf.h"

#include <driver/tty.h>
#include <libk/string.h>

#define PRINTF_TTY_BUFFER_SIZE 512

/* print unsigned int to ascii buffer */
int utoa(char* buff, unsigned int num, unsigned int base) {
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
    return pos;
}

int vsprintf(char* buff, const char* str, va_list vlist) {
    char* initial_buff = buff;

    while(*str) {
        if(*str == '%') {
            str++;
            switch (*str) {
                case 'd':
                case 'i':
                    str++;
                    int i_param = va_arg(vlist, int);

                    if(i_param < 0) {
                        *buff++ = '-';
                        i_param *= -1;
                    }

                    buff += utoa(buff, (unsigned int) i_param, 10);
                    break;
                case 'u':
                    str++;
                    i_param = va_arg(vlist, int);
                    buff += utoa(buff, (unsigned int) i_param, 10);
                    break;
                case 'x':
                    str++;
                    i_param = va_arg(vlist, int);
                    buff += utoa(buff, (unsigned int) i_param, 16);
                    break;
                case 's':
                    str++;
                    char* s_param = va_arg(vlist, char*);
                    strcpy(buff, s_param);
                    buff += strlen(s_param);
                    break;
                case 'c':
                    str++;
                    char c_param = (char)va_arg(vlist, int);
                    *buff++ = c_param;
                    break;
                default:
                    *buff++ = 'E';
                    break;
            }
        } else {
            *buff++ = *str++;
        }
    }
    *buff = '\0';
    return buff-initial_buff;
}

int sprintf(char* buff, const char* fmt, ...) {
    va_list list;
    va_start(list, fmt);
    int res = vsprintf(buff, fmt, list);
    va_end(list);
    return res;
}

int kprintf(const char* str, ...) {
    va_list list;
    va_start(list, str);

    char buff[PRINTF_TTY_BUFFER_SIZE];
    int res = vsprintf(buff, str, list);

    va_end(list);
    tty_puts(buff);

    return  res;
}
