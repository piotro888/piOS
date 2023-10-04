#ifndef PROC_ELF_H
#define PROC_ELF_H

#include <proc/proc.h>

struct proc* elf_load(struct proc_file* file);

#endif
