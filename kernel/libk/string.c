#include "string.h"

size_t strcmp(char* str1, char* str2) {
    while(*str1++ == *str2++) {
        if(*str1 == '\0' && *str2 == '\0')
            return 0;
    }
    return *str1-*str2;
}

size_t strncmp(char* str1, char* str2, size_t n) {
    if(!n)
        return 0;

    int pos = 1;
    while(*str1++ == *str2++) {
        if(*str1 == '\0' && *str2 == '\0')
            return 0;
        if(pos++ == n)
            return 0;
    }
    return *str1-*str2;
}