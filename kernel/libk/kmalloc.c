#include "kmalloc.h"

#define KERNEL_HEAP_START 0x7000
#define KERNEL_HEAP_END 0xFFFF
#define CHUNK_SIZE 16
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_END-KERNEL_HEAP_START)

const int heap_arr_size = KERNEL_HEAP_SIZE/CHUNK_SIZE;
uint16_t memory[KERNEL_HEAP_SIZE/CHUNK_SIZE];

void init_malloc() {
    for(int i=0; i<heap_arr_size; i++) {
        memory[i] = 0;
    }
}

// simple first fit allocator
void* kmalloc(size_t size) {
    int size_in_chunks = (size+CHUNK_SIZE)/CHUNK_SIZE;
    
    void* ptr = NULL;
    int free_chunks = 0, current_chunk = 0;
    while(current_chunk < heap_arr_size ) {
        if(memory[current_chunk] == NULL) {
            free_chunks++;
           
            if(free_chunks == size_in_chunks) {
                ptr = (void*) (KERNEL_HEAP_SIZE + (current_chunk - size_in_chunks + 1)*CHUNK_SIZE);
                memory[current_chunk - size_in_chunks + 1] = size_in_chunks;
                break;
            }

            current_chunk++;
        } else {
            free_chunks = 0;
            current_chunk += memory[current_chunk];
        }
    }

    if(!ptr)
        panic("kmalloc: out of memory");

    return ptr;
}

void kfree(void* ptr) {
    if((uint16_t)ptr < KERNEL_HEAP_START || (uint16_t)ptr > KERNEL_HEAP_END)
        panic("kfree: pointer out of bounds");
    
    uint16_t chunk_number = ((uint16_t)ptr - KERNEL_HEAP_START)/CHUNK_SIZE;
    memory[chunk_number] = 0;
}