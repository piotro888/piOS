#ifndef __FCNTL_H
#define __FCNTL_H

#define F_GETFL 1
#define F_SETFL 2

#include <stdio.h>

int fcntl(FILE* file, unsigned mode, unsigned flags);

#endif
