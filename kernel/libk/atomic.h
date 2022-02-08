#ifndef LIBK_ATOMIC_H
#define LIBK_ATOMIC_H

/* Atomic operations for kernel mode
 * Without using locks, only by disabling interrupts
 * */

int atomic_read_int(volatile int* var);
void atomic_write_int(volatile int* var, int val);
void atomic_add_int(volatile int* var, int val);

#endif
