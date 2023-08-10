#include "string.h"

int strcnt(const char* str, char pat) {
    int cnt = 0;
    do {
        if (*str == pat)
            cnt++;
    } while(*str++);

    return cnt;
}
