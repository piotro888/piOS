#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int __write_to_stream;

static FILE* __stream_to_write;

static char* __buff_to_write;
static char* __buff_max;

static int __printf_size;

void __printf_write(char* buff, size_t len) {
    if(__write_to_stream) {
        __printf_size += fwrite(buff, len, 1, __stream_to_write);
    } else {
        int to_write = len;
        if (__buff_to_write+len >= __buff_max)
            to_write = __buff_max - __buff_to_write;
        memcpy(__buff_to_write, buff, len);
        __buff_to_write += to_write;
        __printf_size += to_write;
    }
}

void __printf_putc(char c) {
    if(__write_to_stream) {
        __printf_size += fwrite(&c, 1, 1, __stream_to_write);
    } else {
        if (__buff_to_write < __buff_max) {
            *__buff_to_write = c; 
            __buff_to_write++;
            __printf_size++;
        }
    }
}

void __printf_utoa(unsigned int num, unsigned int base) {
    int pos = 0;
    char buff[6];

    if(!num) {
        __printf_putc('0');
        return;
    }

    while(num) {
        if(num % base < 10)
            buff[pos++] = '0' + (char)(num%base);
        else
            buff[pos++] = 'a' + (char)((num%base)-10);
        num /= base;
    }

    for(int i=0; i<(pos>>1); i++) {
        char swp = buff[i];
        buff[i] = buff[pos-i-1];
        buff[pos-i-1] = swp;
    }

    __printf_write(buff, pos);
}

void __printf_core(const char* str, va_list vlist) {
    while(*str) {
        if(*str == '%') {
            str++;
            switch (*str) {
                case 'd':
                case 'i':
                    str++;
                    int i_param = va_arg(vlist, int);

                    if(i_param < 0) {
                        __printf_putc('-');
                        i_param *= -1;
                    }

                    __printf_utoa((unsigned int) i_param, 10);
                    break;
                case 'u':
                    str++;
                    i_param = va_arg(vlist, int);
                    __printf_utoa((unsigned int) i_param, 10);
                    break;
                case 'x':
                    str++;
                    i_param = va_arg(vlist, int);
                    __printf_utoa((unsigned int) i_param, 16);
                    break;
                case 's':
                    str++;
                    char* s_param = va_arg(vlist, char*);
                    __printf_write(s_param, strlen(s_param));
                    break;
                case 'c':
                    str++;
                    char c_param = (char)va_arg(vlist, int);
                    __printf_putc(c_param);
                    break;
                default:
                    __printf_putc(' ');
                    break;
            }
        } else {
            __printf_putc(*str++);
        }
    }

    if (!__write_to_stream) {
        __buff_max++; // increase limit reserved for termination
        __printf_putc('\0');
    }
}

int vfprintf( FILE *stream, const char *format, va_list vlist ) {
    __write_to_stream = 1;
    __stream_to_write = stream;
    __printf_size = 0;
    __printf_core(format, vlist);
    return __printf_size;
}

int vsnprintf( char *restrict buffer, size_t bufsz,
               const char *restrict format, va_list vlist ) {
    if (!bufsz)
        return 0;
    __write_to_stream = 0;
    __buff_to_write = buffer;
    if (bufsz == UINT16_MAX)
        __buff_max = (void*) 0xffff;
    else
        __buff_max = __buff_to_write + bufsz - 1;
    __printf_size = 0;
    __printf_core(format, vlist);
    return __printf_size;
}

int fprintf( FILE *stream, const char *format, ... ) {
    va_list vlist;
    va_start(vlist, format);
    int res = vfprintf(stream, format, vlist);
    va_end(vlist);
    return res;
}

int snprintf( char *restrict buffer, size_t bufsz,
               const char *restrict format, ... ) {
    va_list vlist;
    va_start(vlist, format);
    int res = vsnprintf(buffer, bufsz, format, vlist);
    va_end(vlist);
    return res;
}

int sprintf( char *restrict buffer,
               const char *restrict format, ... ) {
    va_list vlist;
    va_start(vlist, format);
    int res = vsnprintf(buffer, UINT16_MAX, format, vlist);
    va_end(vlist);
    return res;
}

int printf( const char *format, ... ) {
    va_list vlist;
    va_start(vlist, format);
    int res = vfprintf(stdout, format, vlist);
    va_end(vlist);
    return res;
}

int vprintf( const char *format, va_list vlist ) {
    return vfprintf(stdout, format, vlist);
}
