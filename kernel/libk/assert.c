#include "assert.h"
#include <libk/kprintf.h>
#include <irq/interrupt.h>
#include <panic.h>

_Noreturn void __assert_fail(char* line, int linenr) {
    /// TODO: FLUSH OUTPUT BUFFER
    int_disable();
    kprintf("\n\nASSERTION FAILED (%s:%d)", line, linenr);
    panic("assertion");
}
