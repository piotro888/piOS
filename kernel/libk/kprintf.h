#ifndef LIBK_KPRINTF
#define LIBK_KPRINTF

/* stdarg provided by compiler */
#include <stdarg.h>

int kprintf(const char* str, ...);

int sprintf(char* buff, const char* fmt, ...);
int vsprintf(char* buff, const char* str, va_list vlist);

int utoa(char* buff, unsigned int num, unsigned int base);

#endif