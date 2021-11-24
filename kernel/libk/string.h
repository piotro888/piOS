#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#include <libk/types.h>

size_t strcmp(char* str1, char* str2);
size_t strncmp(char* str1, char* str2, size_t len);

/*
TODO:
void memcpy();
void memmove();
void memset();
void memcmp();
void strcpy();
void strncpy();
*/

#endif