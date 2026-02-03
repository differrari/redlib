#include "allocate.h"
#include "syscalls/syscalls.h"

typedef struct free_block {
    size_t block_size;//TEST: corrupt this value
    struct free_block* next;
} free_block;

typedef struct allocator_header {
    size_t used;//TEST: corrupt this value
    free_block *free_ptr;
    uintptr_t free_mem;
    struct allocator_header* next;
} allocator_header;

void* allocate(void* page, size_t size, page_allocator fallback){
    if (!fallback) fallback = malloc;
    if (!page) return 0;
    size += sizeof(size_t) * 2;
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    if (size >= PAGE_SIZE){
        return fallback(size);
    }
    
    allocator_header *hdr = (allocator_header*)page;
    
    if (!hdr->free_mem) hdr->free_mem = (uintptr_t)hdr + sizeof(allocator_header);
    
    if (((int64_t)PAGE_SIZE - (int32_t)(hdr->free_mem - (uintptr_t)hdr)) >= (int64_t)size){
        *(size_t*)hdr->free_mem = size;
        void *addr = (void*)(hdr->free_mem + (sizeof(size_t) * 2));
        hdr->free_mem += size;
        hdr->used += size;
        return addr;
    }
    
    free_block *block = hdr->free_ptr;
    free_block **blk_ptr = &hdr->free_ptr;
    while (block){
        if (block->block_size >= size){
            free_block *next = block->next;
            if (block->block_size > size){
                next = block + size;
                next->block_size = block->block_size - size;
            }
            *blk_ptr = next;
            hdr->used += size;
            *(size_t*)block = size;
            return (void*)((uintptr_t)block + (sizeof(size_t) * 2));        
        }
        block = block->next;
    }
    
    if (!hdr->next) hdr->next = fallback(PAGE_SIZE);
    return allocate(hdr->next,size, fallback);
}

static void* zalloc_page = 0;

void* zalloc(size_t size){
    if (!zalloc_page) zalloc_page = malloc(PAGE_SIZE);
    return allocate(zalloc_page, size, malloc);
}