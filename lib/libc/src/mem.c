#include "stdlib.h"
#include "stddef.h"
#include "sys/sys.h"

#define MEMORY_CHUNK 32
// (without last stack page)
#define MAX_HEAP_SIZE (unsigned) 0xF000
#define MIN_HEAP_START (unsigned) 0x1D00 

#define PAGES 16
#define PAGE_SIZE 0x1000
static int mem_pages;

static void* heap_start;

// 0 - unallocated, else size of allocation in chunks (to skip)
static unsigned allocations[(MAX_HEAP_SIZE-MIN_HEAP_START)/MEMORY_CHUNK];

void __malloc_init() {
    struct sys_proc_info info;
    sys_procinfo(0, &info);
    heap_start = info.load_brk;
    mem_pages = info.mem_pages_mapped;
    allocations[0] = ((unsigned)heap_start - MIN_HEAP_START + MEMORY_CHUNK-1)/MEMORY_CHUNK;
}

void* malloc(size_t size) {
    if (!size)
        return NULL;
    
    size_t size_in_chunks = (size + MEMORY_CHUNK-1)/MEMORY_CHUNK;
    size_t free_chunks = 0;
    
    // find free space
    void* base = 0;
    for (unsigned i=0; i<((MAX_HEAP_SIZE-MIN_HEAP_START)/MEMORY_CHUNK); i++) {
        if (allocations[i]) {
            i += allocations[i] - 1;
            free_chunks = 0;
        } else {
            free_chunks++;
            if (free_chunks == size_in_chunks) {
                base = (void*) (MIN_HEAP_START+((i-size_in_chunks+1)*MEMORY_CHUNK));
                allocations[i-size_in_chunks+1] = size_in_chunks;
                break;
            }
        }
    }
    
    if (!base)
        return NULL; // NOMEM
    
    // allocate needed pages
    size_t page_base = ((unsigned)base) >> 12;
    for (unsigned page=page_base; page<page_base+((size + PAGE_SIZE-1)/PAGE_SIZE); page++) {
        if (!(mem_pages & (1u<<page))) {
            sys_pgmap(page);
            mem_pages |= (1u<<page);
        }
    }

    return base;
}

void free(void* ptr) {
    size_t chunk = (size_t)ptr/MEMORY_CHUNK;
    chunk -= MIN_HEAP_START/MEMORY_CHUNK;
    allocations[chunk] = 0;
    // TODO: unmap pages
}
