#ifndef PROC_VIRTUAL_H
#define PROC_VIRTUAL_H

#define PAGE_SIZE 0x1000

void load_into_userspace_program(int page, int* data);
void switch_to_userspace();

void enable_paging();
void disable_paging();
void map_page_zero(int ext_page);
void set_virtual_direct();

#endif