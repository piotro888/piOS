#include "virtual.h"

extern void set_ram_mem(int* data, int page);
void load_into_userspace(int page, int* data) {
    set_ram_mem(data, page);
    asm volatile ("":::"r0","r1","r2","r3","r4");
}

extern void set_program_mem(int* data, int page);
void load_into_userspace_program(int page, int* data) {
    set_program_mem(data, page*2);
     asm volatile ("":::"r0","r1","r2","r3","r4");
}

extern void set_mapping_from_struct(int* pages);
__attribute__((noreturn)) extern void c_switch(int* regs);
void switch_to_userspace(struct proc* p) {
    p->prog_pages[0] = 16; // temporary map program mem page 0 to 16
    set_mapping_from_struct(p->mem_pages);
    asm volatile ("":::"r1"); // clobber r1

    c_switch(p->regs);
}

inline void map_page_zero(int page) {
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