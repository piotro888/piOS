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


#endif
