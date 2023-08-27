#ifndef __STDIO_H
#define __STDIO_H

struct __FILE_INTERNAL;
typedef struct __FILE_INTERNAL FILE;

#include <stddef.h>

#define EOF -1

FILE* fopen( const char* path, const char* mode );
int fclose( FILE* file );

size_t fwrite( const void *buffer, size_t _size, size_t nmemb, FILE *stream );
size_t fread( void *buffer, size_t _size, size_t nmemb, FILE *stream );

int fflush( FILE* file );

int fputc( int ch, FILE *stream );
int fputs( const char* str, FILE *stream );

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#include <stdarg.h>

int vfprintf( FILE *stream, const char *format, va_list vlist );
int vsnprintf( char *restrict buffer, size_t bufsz, const char *restrict format, va_list vlist );

int fprintf( FILE *stream, const char *format, ... );
int snprintf( char *restrict buffer, size_t bufsz, const char *restrict format, ... );

int sprintf( char *restrict buffer, const char *restrict format, ... );
int printf( const char * format, ... );

int vprintf( const char * format, va_list vlist );

#endif
