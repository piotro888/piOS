#include "atomic.h"

#include <irq/interrupt.h>

int atomic_read_int(volatile int* var) {
    int int_should_enable = int_get();
    int_disable();

    int val = *var;

    if(int_should_enable)
        int_enable();

    return val;
}

void atomic_write_int(volatile int* var, int val) {
    int int_should_enable = int_get();
    int_disable();

    *var = val;

    if(int_should_enable)
        int_enable();
}

void atomic_add_int(volatile int* var, int val) {
    int int_should_enable = int_get();
    int_disable();

    *var = *var + val;

    if(int_should_enable)
        int_enable();
}
