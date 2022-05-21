#include "virtual.h"

#include <libk/assert.h>
#include <libk/string.h>
#include <libk/math.h>
#include <proc/sched.h>

#define PAGE_PART(addr) ((addr)>>12)
#define ADDR_PART(addr) ((addr)&0x0fff)
#define PHYS_PAGE(addr, page_table) ((page_table)[PAGE_PART(addr)])

void memcpy_from_userspace(void* kbuff, struct proc* proc, u16 ubuff, size_t size) {
    while (size) {
        map_page_zero(PHYS_PAGE(ubuff, proc->mem_pages));
        size_t to_copy = MIN(size, PAGE_SIZE - ADDR_PART(ubuff));
        memcpy_full(kbuff, (void*)ADDR_PART(ubuff), to_copy);
        size -= to_copy;
        ubuff += to_copy;
        kbuff = (void*) ((u16)kbuff+to_copy);
    }
    map_page_zero(0);
}

void memcpy_to_userspace(struct proc* proc, u16 ubuff, void* kbuff, size_t size) {
    while (size) {
        map_page_zero(PHYS_PAGE(ubuff, proc->mem_pages));
        size_t to_copy = MIN(size, PAGE_SIZE - ADDR_PART(ubuff));
        memcpy_full((void*)ADDR_PART(ubuff), kbuff, to_copy);
        size -= to_copy;
        ubuff += to_copy;
        kbuff = (void*) ((u16)kbuff+to_copy);
    }
    map_page_zero(0);
}

extern void set_ram_mem(u16* data, int page, size_t size, size_t offset);
void load_into_userspace(int page, u16* data, size_t size, size_t offset) {
    ASSERT(offset+size <= PAGE_SIZE);
    set_ram_mem(data, page, size, offset);
    asm volatile ("":::"r0","r1","r2","r3","r4");
}

extern void set_program_mem(u16* data, int page, size_t size, size_t offset);
void load_into_userspace_program(int page, u16* data, size_t size, size_t offset) {
    ASSERT(offset+size <= PAGE_SIZE);

    // addr: 4b page 11b prog_address 1b h/l. page size is still 0x1000*16b
    // exec: 4b page 12b prog_addr (but 12th of prg_addr is LSB of page while programming). Page must be <<1 while programming | 12th bit (see elf.c)
    set_program_mem(data, page, size, offset);
    asm volatile ("":::"r0","r1","r2","r3","r4");
}

extern void set_mapping_from_struct(int* pages);
__attribute__((noreturn)) extern void c_switch(int* regs);
void switch_to_userspace(struct proc* p) {
    set_mapping_from_struct(p->mem_pages);
    asm volatile ("":::"r1"); // clobber r1

    c_switch(p->regs);
}

inline void map_page_zero(int page) {
    current_proc->mem_pages[0] = page;

    asm volatile (
        "srs %0, 0x10"
        :: "r" (page)
    );
}

inline void enable_paging() {
    asm volatile (
        "srl r0, 1\n"
        "ori r0, r0, 0x8\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}

inline void disable_paging() {
    asm volatile (
        "srl r0, 1\n"
        "ani r0, r0, 0xf7\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}
