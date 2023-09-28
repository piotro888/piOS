#ifndef __STDLIB_H
#define __STDLIB_H

#include <stddef.h>

void *malloc( size_t size );
void free( void *ptr );

#define RAND_MAX  32767
int rand(void);
void srand(unsigned int seed);

#endif
