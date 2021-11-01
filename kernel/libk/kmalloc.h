#ifndef LIBK_KMALLOC_H
#define LIBK_KMALLOC_H

#include <libk/types.h>

void init_malloc();

void* kmalloc(size_t size);

void kfree(void* ptr);

#endif