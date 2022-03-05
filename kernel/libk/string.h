#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#include <libk/types.h>

size_t strcmp(char* str1, char* str2);
size_t strncmp(char* str1, char* str2, size_t len);

size_t strlen(char* str);

void strcpy(char* dst, char* src);
void strncpy(char* dst, char* src, size_t len);

char* strchr(char* str, char ch);

void strcpynt(char* dst, char* src, size_t len);
void strprefcpy(char* dst, char* src, size_t len);

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