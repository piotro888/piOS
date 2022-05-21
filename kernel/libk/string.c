#include "string.h"

size_t strcmp(char* str1, char* str2) {
    while(*str1 == *str2) {
        if(*str1 == 0 && *str2 == 0)
            return 0;
        str1++;
        str2++;
    }
    return *str1-*str2;
}

size_t strncmp(char* str1, char* str2, size_t n) {
    if(!n)
        return 0;

    size_t pos = 0;
    while(*str1 == *str2) {
        if(*str1 == 0 && *str2 == 0)
            return 0;

        if(++pos >= n)
            return 0; 
        str1++;
        str2++;
    }
    return *str1-*str2;
}

size_t strlen(char* str) {
    size_t len = 0;
    while(*(str++))
        len++;

    return len;
}

char* strcpy(char* dst, char* src) {
    while(*src)
        *(dst++) = *(src++);
    *dst = *src; // copy null byte
    return dst;
}

// Copy to max n bytes of dst and pad left space with zeroes
// May not produce null terminated strings if len(src) >= n
char* strncpy(char* dst, char* src, size_t len) {
    while(*src && len--) {
        *(dst++) = *(src++);
    }

    while(len--)
        *(dst++) = '\0';
    return dst;
}

char* strchr(char* str, int ch) {
    do {
        if(*str == (char)ch)
            return str;
    } while (*str++);
    return NULL;
}

// -- not std libc functions -- 

// strncpy but always null terminate if reached end of n
void strncpynt(char* dst, char* src, size_t len) {
    while(*src && len--) {
        *(dst++) = *(src++);
    }

    while(len--)
        *(dst++) = '\0';
    
    *(--dst) = '\0';
}

// Copy n char prefix from src, and put it allways null terminated to dst
// Copies one byte more than len
void strprefcpy(char* dst, char* src, size_t len) {
    while(*src && len--) {
        *(dst++) = *(src++);
    }
    *dst = '\0'; // always null terminate, but uses extra byte
}

// Copy u8* lower -> u8* lower
void memcpy_8(u8* dst, u8* src, size_t size) {
    while(size--)
        *dst++ = *src++;
}

// Exact copy of memory (u16, but without skips)
void memcpy_full(void* dst, void* src, size_t size) {
    u16 dsti = (u16) dst, srci= (u16) src;
    while(size--) {
        *((u16*)dsti) = *((u16*)srci);
        dsti++; srci++;
    }
}

// Copy like u16* layout (skips every 2nd word)
void memcpy_16(u16* dst, u16* src, size_t size) {
    while(size--)
        *dst++ = *src++;
}
