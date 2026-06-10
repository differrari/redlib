#include "allocate.h"
#include "syscalls/syscalls.h"
#include "std/memory.h"
#include "page_index.h"

#ifdef CROSS
extern void free(void*);
#endif

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

// #define DEBUG_ALLOC
#ifdef DEBUG_ALLOC
#define alloc_print(fmt,...) print(fmt,##__VA_ARGS__)
#else
#define alloc_print(fmt,...) 
#endif

#define INDIVIDUAL_HDR (sizeof(size_t) * 2)//TODO: use the extra value as a canary to indicate live memory

page_index *p_index;

static inline void* fallback_proxy(size_t size, page_allocator fallback){
    if (!p_index) {
        p_index = fallback(PAGE_SIZE);
        if (!p_index) return 0;
    }
    void *p = fallback(size);
    if (!p) return 0;
    register_page_alloc(p_index, p, size, fallback);
    return p;
}

void* allocate(void* page, size_t size, page_allocator fallback){
    if (!fallback) fallback = page_alloc;
    if (!page) return 0;
    size += INDIVIDUAL_HDR;
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    if (size >= PAGE_SIZE - sizeof(allocator_header)){
        return fallback_proxy(size,fallback);
    }
    
    allocator_header *hdr = (allocator_header*)page;
    while (hdr) {
        if (!hdr->free_mem) hdr->free_mem = (uintptr_t)hdr + sizeof(allocator_header);
        
        free_block **blk_ptr = &hdr->free_block;
        while (*blk_ptr){
            free_block *block = *blk_ptr;
            if ((uintptr_t)block < (uintptr_t)hdr + sizeof(allocator_header) || (uintptr_t)block + sizeof(free_block) > (uintptr_t)hdr + PAGE_SIZE){
                print("[ALLOC] Wrong allocation, a free block points outside its page %llx + %llx >= %llx",(uintptr_t)block & ~(0xFFF), block->block_size,(uintptr_t)hdr & ~(0xFFF));
                return 0;
            }
            if ((uintptr_t)block + block->block_size > (uintptr_t)hdr + PAGE_SIZE) return 0;
            free_block *next = block->next;
            if (next && ((uintptr_t)next & ~(PAGE_SIZE - 1)) != ((uintptr_t)hdr & ~(PAGE_SIZE - 1))) return 0;
            if (block->block_size >= size){
                size_t alloc_size = block->block_size;
                if (block->block_size >= size + sizeof(free_block)){
                    free_block *split = (free_block*)((uptr)block + size);
                    split->block_size = block->block_size - size;
                    split->next = next;
                    next = split;
                    alloc_size = size;
                }
                *blk_ptr = next;
                hdr->used += alloc_size;
                *(size_t*)block = alloc_size;
                void* addr = (void*)((uintptr_t)block + INDIVIDUAL_HDR);
                memset(addr, 0, alloc_size - INDIVIDUAL_HDR);
                return addr;
            }
            blk_ptr = &block->next;
        }

        if (hdr->used + size <= PAGE_SIZE - sizeof(allocator_header) && hdr->free_mem + size <= (uintptr_t)hdr + PAGE_SIZE) {
            *(size_t*)hdr->free_mem = size;
            void *addr = (void*)(hdr->free_mem + INDIVIDUAL_HDR);
            hdr->free_mem += size;
            hdr->used += size;
            memset(addr, 0, size - INDIVIDUAL_HDR);
            return addr;
        }
    
        if (!hdr->next) {
            hdr->next = fallback(PAGE_SIZE);
            if (!hdr->next) return 0;
        }
        hdr = hdr->next;
    }
    return 0;
}

static void* zalloc_page = 0;

void* zalloc(size_t size){
    if (!zalloc_page) zalloc_page = page_alloc(PAGE_SIZE);
    return zalloc_page ? allocate(zalloc_page, size, page_alloc) : 0;
}

void release(void* ptr){
    if (!ptr) return;

    size_t alloc_size = p_index ? get_alloc_size(p_index, ptr) : 0;
    if (alloc_size) {
        unregister_page_alloc(p_index, ptr);
        page_free(ptr);
        return;
    }

    if (!((uintptr_t)ptr & (PAGE_SIZE - 1))){
        alloc_print("[FREE] Page free %llx",ptr);
        page_free(ptr);
        //if (p_index) unregister_page_alloc(p_index, ptr);
        return;
    }
    
    allocator_header *hdr = (allocator_header*)((uintptr_t)ptr & (~(PAGE_SIZE - 1)));
    
    if (!hdr->used) return;
    
    uintptr_t header = (uintptr_t)ptr - INDIVIDUAL_HDR;
    
    size_t size = *(size_t*)header;
    
    alloc_print("[FREE] Freeing %llx at %llx",size,header);
    
    if (size < INDIVIDUAL_HDR || header + size > ((uintptr_t)hdr + PAGE_SIZE)){
        alloc_print("[FREE] allocated memory pointed outside of block");
        return;
    }
    
    memset32((void*)header, 0xDEADBEEF, size);
    if (hdr->used < size) {
        hdr->used = 0;
        hdr->free_block = 0;
        hdr->free_mem = 0;
        return;
    }
    
    hdr->used -= size;
    if (!hdr->used){
        hdr->free_block = 0;
        hdr->free_mem = 0;
    } else {
        free_block *block = (free_block*)header;
        free_block *prev = 0;
        free_block *cur = hdr->free_block;
        block->block_size = size;
        while (cur && cur < block) {
            if (((uintptr_t)cur & ~(PAGE_SIZE - 1)) != ((uintptr_t)hdr & ~(PAGE_SIZE - 1))){
                print("[FREE] block free now pointing to other page %llx vs %llx", cur, hdr);
                return;
            }
            prev = cur;
            cur = cur->next;
        }
        block->next = cur;
        if (prev) prev->next = block;
        else hdr->free_block = block;
        // while ((uintptr_t)block->next == header + size){
        //     free_block *next_block = (free_block*)block->next;
        //     block->next = next_block->next;
        //     block->block_size += next_block->block_size;
        // } 
        if (block->next && (uintptr_t)block + block->block_size == (uintptr_t)block->next) {
            block->block_size += block->next->block_size;
            block->next = block->next->next;
        }
        
        if (prev && (uintptr_t)prev + prev->block_size == (uintptr_t)block) {
            prev->block_size += block->block_size;
            prev->next = block->next;
        }
    }
}

void* reallocate(void* ptr, size_t new_size){
    //TODO: if possible, once we have a sentinel value in the extra 0x8, we can check if we can just take up more memory without reallocating entirely
    // allocator_header *hdr = (allocator_header*)((uintptr_t)ptr & (~0xFFF));
    if (!new_size) {
        if (ptr) release(ptr);
        return 0;
    }
    
    if (!ptr) return zalloc(new_size);
    
    size_t old_size = p_index ? get_alloc_size(p_index, ptr) : 0;
   if (old_size) {
        if (old_size < INDIVIDUAL_HDR) return 0;
        old_size -= INDIVIDUAL_HDR;
    } else {
        if (!((uintptr_t)ptr & (PAGE_SIZE - 1))) return 0;
        uintptr_t header = (uintptr_t)ptr - INDIVIDUAL_HDR;
        old_size = *(size_t*)header;
        if (old_size < INDIVIDUAL_HDR) return 0;
        old_size -= INDIVIDUAL_HDR;
    }
    if (new_size <= old_size) return ptr;
    
    // size_t diff = new_size > old_size ? new_size-old_size : size-old_size;
    
    // size_t next_val = *(size_t*)((uintptr_t)ptr + old_size);
    
    // if (next_val == 0 || next_val == 0xDEADBEEFDEADBEEF)
    
    void *new_ptr = zalloc(new_size);
    if (!new_ptr) return 0;
    memcpy(new_ptr, ptr, old_size);
    release(ptr);
    return new_ptr;
}

#include "debug/assert.h"

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