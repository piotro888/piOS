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

void strcpy(char* trg, char* src) {
    while(*src)
        *(trg++) = *(src++);
    *trg = *src; // copy null byte
}

// Copy to max n bytes of dst and pad left space with zeroes
// May not produce null terminated strings if len(src) >= n
void strncpy(char* dst, char* src, size_t len) {
    while(*src && len--) {
        *(dst++) = *(src++);
    }

    while(len--)
        *(dst++) = '\0';
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