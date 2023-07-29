#include "string.h"

// __LIBC_KERNEL__ WANTS ALL

char* strcpy(char* restrict dest, const char* restrict src) {
    char* orig_dest = dest;

    while(*src) 
        *(dest++) = *(src++);
    
    *dest = *src; // copy null byte
    return orig_dest;
}

// copy string up to len and pad with null bytes. May produce unterminated strings.
char* strncpy(char* restrict dest, const char* restrict src, size_t count) {
    char* orig_dest = dest;

    while(*src && count) {
        *(dest++) = *(src++);
        count--;
    } 
    
    // if termination is in count range, it is filled here
    while(count--)
        *(dest++) = '\0';

    return orig_dest;
}

// String length, not including termination
size_t strlen(const char *str) {
    size_t len = 0;
    while(*(str++))
        len++;

    return len;
}

// Sign of result is difference between first different chars
int strcmp(const char* lhs, const char* rhs) {
    while(*lhs == *rhs) {
        // both terminating
        if(!(*lhs) && !(*rhs))
            break;
        lhs++;
        rhs++;
    }
    // only sign of result is important in libc
    return *lhs-*rhs;
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
    if (!count)
        return 0; // libc

    size_t index = 0;
    while(*lhs == *rhs) {
        // both terminating
        if(!(*lhs) && !(*rhs))
            break;
        // equal up to a range
        if (++index >= count)
            break;

        lhs++;
        rhs++;
    }
    return *lhs-*rhs;
}

// First occurence of character. Null termination can be found.
char* strchr(const char *str, int ch) {
    ch = (unsigned char)ch; // libc: after converion to char (libc and musl uses unsigned, makes more sense)
    do {
        if ((unsigned char)*str == ch) // libc: each character as unsigned char
            return (char*) str;
    } while(*str++);
    return NULL; // null pointer if not found
}

// Set count bytes from dest to ch interpreted as unsigned char
void* memset(void* dest, int ch, size_t count) {
    char* restrict _dest = dest; 
    while(count--)
        *_dest++ = (unsigned char) ch; 
    return dest;
}

// Copy count bytess from src to dest (as unsigned char)
void* memcpy(void* restrict dest, const void* restrict src, size_t count) {
    char* restrict _dest = dest; 
    const char* restrict _src = src; 
    while(count--)
        *_dest++ = *_src++; 
    return dest;

}
