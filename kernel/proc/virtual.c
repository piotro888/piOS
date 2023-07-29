#include "virtual.h"

#include <string.h>
#include <libk/assert.h>
#include <libk/math.h>
#include <proc/sched.h>

#define PAGE_PART(addr) ((addr)>>12)
#define ADDR_PART(addr) ((addr)&0x0fff)
#define PHYS_PAGE(addr, page_table) ((page_table)[PAGE_PART(addr)])

void memcpy_from_userspace(void* kbuff, struct proc* proc, u16 ubuff, size_t size) {
    while (size) {
        map_page_zero(PHYS_PAGE(ubuff, proc->mem_pages));
        size_t to_copy = MIN(size, PAGE_SIZE - ADDR_PART(ubuff));
        memcpy(kbuff, (void*)ADDR_PART(ubuff), to_copy);
        size -= to_copy;
        ubuff += to_copy;
        kbuff = (void*) ((u16)kbuff+to_copy);
    }
    map_page_zero(scheduling_enabled ? current_proc->mem_pages[0] : ILLEGAL_PAGE);
}

void memcpy_to_userspace(struct proc* proc, u16 ubuff, void* kbuff, size_t size) {
    while (size) {
        map_page_zero(PHYS_PAGE(ubuff, proc->mem_pages));
        size_t to_copy = MIN(size, PAGE_SIZE - ADDR_PART(ubuff));
        memcpy((void*)ADDR_PART(ubuff), kbuff, to_copy);
        size -= to_copy;
        ubuff += to_copy;
        kbuff = (void*) ((u16)kbuff+to_copy);
    }
    map_page_zero(scheduling_enabled ? current_proc->mem_pages[0] : ILLEGAL_PAGE);
}

extern void set_ram_mem(u16* data, int page, size_t end_addr, size_t offset);
void load_into_userspace(int page, void* data, size_t size, size_t offset) {
    ASSERT(offset+size <= PAGE_SIZE);

    map_page_zero(page);
    memcpy((void*)offset, data, size);
    map_page_zero(ILLEGAL_PAGE);

    //set_ram_mem(data, page, size+offset-1, offset);
    asm volatile ("":::"r0","r1","r2","r3","r4");
}

void load_into_userspace_program(int page, void* data, size_t size, size_t offset) {
    ASSERT(offset+size <= PAGE_SIZE); // able to load in DATA byte addresed page
    ASSERT(!(size&0b11)); // aligned to full instructions

    map_page_zero(page);
    memcpy((void*)offset, data, size*2); //WHY????? TRY REMOVING IN A MM
    map_page_zero(ILLEGAL_PAGE);
}

extern void set_mapping_from_struct(int* pages);
__attribute__((noreturn)) extern void c_switch(int* regs);
void switch_to_userspace(struct proc* p) {
    // this function can be called only from init or irq handler
    // disabling paging here is needed to set new paging and all our data needs to be still accessible with default addressing
    disable_paging();
    handling_interrupt = 0;

    set_mapping_from_struct(p->mem_pages);
    asm volatile ("":::"r1"); // clobber r1

    c_switch(p->regs);
}

inline void map_page_zero(int page) {
    if(scheduling_enabled && !handling_interrupt) // scheduling disabled - no interrupts, no need to restore this value. Dont overwrite thread pages in irq handler
        current_proc->mem_pages[0] = page;

    asm volatile (
        "srs %0, 0x200"
        :: "r" (page)
    );
}

// sets memory paging to default kernel setting (as with paging disabled). Allows to use map_page_zero
extern void enable_default_memory_paging_asm();
void enable_default_memory_paging() {
    enable_default_memory_paging_asm();
    asm volatile ("":::"r1");
}

inline void enable_paging() {
    asm volatile (
        "srl r0, 1\n"
        "ori r0, r0, 0x2\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}

inline void disable_paging() {
    asm volatile (
        "srl r0, 1\n"
        "ani r0, r0, 0xfd\n"
        "srs r0, 1\n"
        ::: "r0"
    );
}
