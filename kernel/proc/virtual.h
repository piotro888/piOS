#ifndef PROC_VIRTUAL_H
#define PROC_VIRTUAL_H

#define PAGE_SIZE 0x1000

#include <proc/proc.h>

void memcpy_from_userspace(void* kbuff, struct proc* proc, u16 ubuff, size_t size);
void memcpy_to_userspace(struct proc* proc, u16 ubuff, void* kbuff, size_t size);

void load_into_userspace_program(int page, void* data, size_t size, size_t offset);
void load_into_userspace(int page, void* data, size_t size, size_t offset);
void switch_to_userspace(struct proc* p);

void enable_paging();
void enable_default_memory_paging();
void disable_paging();
void map_page_zero(int ext_page);

unsigned int get_proc_addr_page(int pid, void* addr);

#define ILLEGAL_PAGE 0

#define VIRT_PAGE_PART(addr)  (((unsigned int)addr)>>12)
#define VIRT_LOCAL_PART(addr) (((unsigned int)addr)&0x0fff)

#endif
