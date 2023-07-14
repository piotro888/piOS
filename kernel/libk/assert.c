#include "assert.h"
#include <libk/kprintf.h>
#include <panic.h>

void __assert(int expr, char* line, int linenr) {
    if(!expr) {
        kprintf("\n\nASSERTION FAILED (%s:%d)", line, linenr);
        panic("assertion");
    }
}
