#include "virtual.h"

extern void set_ram_mem(int page, int* data);
void load_into_userspace(int page, int* data) {
    set_ram_mem(page, data);
    asm volatile ("":::"r0","r1","r3","r4");
}

extern void set_program_mem(int page, int* data);
void load_into_userspace_program(int page, int* data) {
    set_program_mem(page*2, data);
    asm volatile ("":::"r0","r1","r3","r4");
}

__attribute__((noreturn)) extern void c_switch();
__attribute__((noreturn)) void switch_to_userspace() {
    // TODO: set ram & rom pages from proc struct
    asm volatile ( // temporary map program mem page 0 to 16
        "ldi r0, 16\n"
        "srs r0, 0x20"
        ::: "r0"
    );
    c_switch();
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

// Set 1->1 mapping in virtual memory table (for kernel)
void set_virtual_direct() {
    asm volatile (
        "ldi r0, 0\n"
        "srs r0, 0x10\n"
        "ldi r0, 1\n"
        "srs r0, 0x11\n"
        "ldi r0, 2\n"
        "srs r0, 0x12\n"
        "ldi r0, 3\n"
        "srs r0, 0x13\n"
        "ldi r0, 4\n"
        "srs r0, 0x14\n"
        "ldi r0, 5\n"
        "srs r0, 0x15\n"
        "ldi r0, 6\n"
        "srs r0, 0x16\n"
        "ldi r0, 7\n"
        "srs r0, 0x17\n"
        "ldi r0, 8\n"
        "srs r0, 0x18\n"
        "ldi r0, 9\n"
        "srs r0, 0x19\n"
        "ldi r0, 10\n"
        "srs r0, 0x1A\n"
        "ldi r0, 11\n"
        "srs r0, 0x1B\n"
        "ldi r0, 12\n"
        "srs r0, 0x1C\n"
        "ldi r0, 13\n"
        "srs r0, 0x1D\n"
        "ldi r0, 14\n"
        "srs r0, 0x1E\n"
        "ldi r0, 15\n"
        "srs r0, 0x1F\n"
        ::: "r0"
    );
}