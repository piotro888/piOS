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

int atomic_compare_and_swap_int(volatile int* var, int cmp, int swap) {
    int int_should_enable = int_get();
    int_disable();

    int ret = 0;
    if(*var == cmp) {
        *var = swap;
        ret = 1;
    }

    if(int_should_enable)
        int_enable();
    return ret;
}
