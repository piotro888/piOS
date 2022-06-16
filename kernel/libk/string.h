#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#include <libk/types.h>

size_t strcmp(char* str1, char* str2);
size_t strncmp(char* str1, char* str2, size_t len);

size_t strlen(char* str);

char* strcpy(char* dst, char* src);
char* strncpy(char* dst, char* src, size_t len);

char* strchr(char* str, int ch);

void strcpynt(char* dst, char* src, size_t len);
void strprefcpy(char* dst, char* src, size_t len);

void memcpy(void* dst, void* src, size_t size);

/*
TODO:
void memmove();
void memset();
void memcmp();
*/

#endif
