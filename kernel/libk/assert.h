#ifndef LIBK_ASSERT_H
#define LIBK_ASSERT_H

#include <libk/kprintf.h>
#include <panic.h>

__attribute__((noreturn)) void __assert_fail(char* line, int line_nr);

// assert macro inspired from musl libc.
#define ASSERT(x) ((void)((x) || (__assert_fail((#x" @ "__FILE__), __LINE__),0)))
#define ASSERT_NOT_REACHED() __assert_fail("ASSERT NOT REACHED @ "__FILE__, __LINE__)

#endif
