#ifndef LIBC_STRING
#define LIBC_STRING

#include <stddef.h>

char *strcpy( char *restrict dest, const char *restrict src );
char *strncpy( char *restrict dest, const char *restrict src, size_t count );

size_t strlen( const char *str );

int strcmp( const char *lhs, const char *rhs );
int strncmp( const char* lhs, const char* rhs, size_t count );

char *strchr( const char *str, int ch );

void *memset( void *dest, int ch, size_t count );
void *memcpy( void *restrict dest, const void *restrict src, size_t count );


#endif
