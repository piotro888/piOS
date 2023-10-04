#ifndef __FCNTL_H
#define __FCNTL_H

#define F_GETFL 1
#define F_SETFL 2
#define F_DRIVER_CUSTOM_START 0x100

#include <stdio.h>

int fcntl(FILE* file, unsigned mode, unsigned flags);

#endif
