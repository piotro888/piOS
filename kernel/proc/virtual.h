#ifndef PROC_VIRTUAL_H
#define PROC_VIRTUAL_H

#define PAGE_SIZE 0x1000

#include <proc/proc.h>

void load_into_userspace_program(int page, int* data);
void load_into_userspace(int page, int* data);
void switch_to_userspace(struct proc* p);

void enable_paging();
void disable_paging();
void map_page_zero(int ext_page);

#endif