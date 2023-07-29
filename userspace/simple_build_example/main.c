#include <string.h>
#include <sys.h>

char test[40];

void  __attribute__((constructor)) ctr() {
    syscall_raw(0,1,0,0,0);
}

int main() {
    syscall_raw(0,2,0,0,0);
    strcpy(test, "Hello");
}
