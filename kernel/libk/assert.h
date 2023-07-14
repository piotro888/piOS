#ifndef LIBK_ASSERT_H
#define LIBK_ASSERT_H

#include <libk/kprintf.h>
#include <panic.h>

void __assert(int expr, char* line, int line_nr);

#define ASSERT(x) __assert((x), (#x" @ "__FILE__), __LINE__)
#define ASSERT_NOT_REACHED() __assert(0, "ASSERT NOT REACHED @ "__FILE__, __LINE__)

#endif
