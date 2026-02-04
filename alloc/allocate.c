#include "allocate.h"
#include "syscalls/syscalls.h"
#include "std/memory.h"

typedef struct free_block {
    size_t block_size;//TEST: corrupt this value
    struct free_block* next;
} free_block;

typedef struct allocator_header {
    size_t used;//TEST: corrupt this value
    free_block *free_block;
    uintptr_t free_mem;
    struct allocator_header* next;
} allocator_header;

#define INDIVIDUAL_HDR (sizeof(size_t) * 2)

void* allocate(void* page, size_t size, page_allocator fallback){
    if (!fallback) fallback = malloc;
    if (!page) return 0;
    size += INDIVIDUAL_HDR;
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    if (size >= PAGE_SIZE){
        return fallback(size);
    }
    
    allocator_header *hdr = (allocator_header*)page;
    
    if (!hdr->free_mem) hdr->free_mem = (uintptr_t)hdr + sizeof(allocator_header);
    
    if (((int64_t)PAGE_SIZE - (int32_t)(hdr->free_mem - (uintptr_t)hdr)) >= (int64_t)size){
        *(size_t*)hdr->free_mem = size;
        void *addr = (void*)(hdr->free_mem + INDIVIDUAL_HDR);
        hdr->free_mem += size;
        hdr->used += size;
        return addr;
    }
    
    free_block *block = hdr->free_block;
    free_block **blk_ptr = &hdr->free_block;
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
            return (void*)((uintptr_t)block + INDIVIDUAL_HDR);        
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

void release(void* ptr){
    allocator_header *hdr = (allocator_header*)((uintptr_t)ptr & (~0xFFF));
    
    if (!hdr->used) return;
    
    uintptr_t header = (uintptr_t)ptr - INDIVIDUAL_HDR;
    
    size_t size = *(size_t*)header;
    
    if (size > 0x1000 - ((uintptr_t)ptr & 0xFFF)) return;
    
    memset32((void*)header, 0xDEADBEEF, size);
    
    hdr->used -= size;
    if (!hdr->used){
        hdr->free_block = 0;
        hdr->free_mem = 0;
    } else {
        free_block *block = (free_block*)header;
        block->block_size = size;
        block->next = hdr->free_block;
        hdr->free_block = block;
        while ((uintptr_t)block->next == header + size){
            free_block *next_block = (free_block*)block->next;
            block->next = next_block->next;
            block->block_size += next_block->block_size;
        } 
    }
}

#include "test.h"

bool test_zalloc(){
    
    uintptr_t first = (uintptr_t)zalloc(0xCF8);//Heap + 0
    assert_eq(first & 0xFF, 0x30, "First allocation in wrong place %llx",first);
    print("First allocation at %llx",first);
    uintptr_t three_pages = (uintptr_t)zalloc(0x29f8);//Heap + 1 to Heap + 3
    assert_false(three_pages & 0xFFF, "Multi-page allocation not aligned correctly %x",three_pages & 0xFFF);
    assert_eq((three_pages & ~(0xFFF)), (first & ~(0xFFF)) + 0x1000, "Multi-page allocation not allocated in correct place %llx",three_pages);
    print("Multi-page allocation at %llx - %llx",(three_pages & ~(0xFFF)),(three_pages & ~(0xFFF)) + 0x3000);
    uintptr_t overflow = (uintptr_t)zalloc(0x3f8);//Heap + 4
    assert_eq(overflow & 0xFF, 0x30, "Overflown allocation not placed in new page %llx",overflow);
    assert_eq((overflow & ~(0xFFF)), (first & ~(0xFFF)) + 0x4000, "Multi-page allocation not aligned correctly");
    print("Second in-page alloc page at %llx",(overflow & ~(0xFFF)));
    uintptr_t in_page = (uintptr_t)zalloc(0x3f8);
    assert_eq((overflow & ~(0xFFF)), (in_page & ~(0xFFF)), "Allocation meant for %llx found in %llx",(overflow & ~(0xFFF)),(in_page & ~(0xFFF)));
    
    for (size_t i = first; i < 0x5000; i++){
        assert_false(*(uint8_t*)(first + i), "Allocated memory not 0'd %x",*(uint8_t*)(first + i));
    }
    
    print("alloc with virtual memory working correctly");
    
    release((void*)first);
    
    assert_eq(*(uint32_t*)(first + 0x30), 0xDEADBEEF, "First allocation not freed correctly",first);
    //TEST: more tests for releasing memory, including fragmentation protection
    
    print("alloc release working correctly");
    
    return true;
}