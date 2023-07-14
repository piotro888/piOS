#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#include <libk/types.h>

ssize_t strcmp(const char* str1, const char* str2);
ssize_t strncmp(const char* str1, const char* str2, size_t len);

size_t strlen(const char* str);

char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t len);

char* strchr(const char* str, int ch);

void strcpynt(char* dst, char* src, size_t len);
void strprefcpy(char* dst, char* src, size_t len);
char* strcpyend(char* dst, char* src);

void* memcpy(void* dst, const void* src, size_t size);
void* memset(void* dst, int val, size_t size);

/*
TODO:
void memmove();
void memcmp();
*/

#endif
