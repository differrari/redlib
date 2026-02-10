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

// #define DEBUG_ALLOC
#ifdef DEBUG_ALLOC
#define alloc_print(fmt,...) print(fmt,##__VA_ARGS__)
#else
#define alloc_print(fmt,...) 
#endif

#define INDIVIDUAL_HDR (sizeof(size_t) * 2)//TODO: use the extra value as a canary to indicate live memory

#define ALIGN 0x1000

void *aligned_malloc(int size) {
    void *mem = malloc(size+ALIGN+sizeof(void*));
    void **ptr = (void**)((uintptr_t)(mem+ALIGN+sizeof(void*)) & ~(ALIGN-1));
    ptr[-1] = mem;
    return ptr;
}

void aligned_free(void *ptr) {
#ifdef CROSS
    free(((void**)ptr)[-1]);
#endif
}

static inline void free_proxy(void* ptr){
#ifdef CROSS
    aligned_free(ptr);
#else
    page_free(ptr);
#endif
}

static inline void* alloc_proxy(size_t size){
#ifdef CROSS
    void *m = aligned_malloc(size);
    memset(m,0,size);//Let the record show libc is fn stupid
#else 
    void* m = page_alloc(size);
#endif
    return m;
}

void* allocate(void* page, size_t size, page_allocator fallback){
    if (!fallback) fallback = alloc_proxy;
    if (!page) return 0;
    size += INDIVIDUAL_HDR;
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    if (size >= PAGE_SIZE - sizeof(allocator_header)){
        return fallback(size);
    }
    
    allocator_header *hdr = (allocator_header*)page;
    
    if (((int64_t)PAGE_SIZE - (int32_t)(hdr->used - sizeof(allocator_header))) >= (int64_t)size){
        if (!hdr->free_mem) hdr->free_mem = (uintptr_t)hdr + sizeof(allocator_header);
        
        if (((int64_t)PAGE_SIZE - (int32_t)(hdr->free_mem - (uintptr_t)hdr)) >= (int64_t)size){
            *(size_t*)hdr->free_mem = size;
            void *addr = (void*)(hdr->free_mem + INDIVIDUAL_HDR);
            hdr->free_mem += size;
            hdr->used += size;
            memset(addr, 0, size - INDIVIDUAL_HDR);
            return addr;
        }
        
        free_block *block = hdr->free_block;
        free_block **blk_ptr = &hdr->free_block;
        while (block && (uptr)block != 0xDEADBEEFDEADBEEF){
            if ((uintptr_t)block + block->block_size >= (uintptr_t)hdr + 0x1000){
                alloc_print("[ALLOC] Wrong allocation, a free block points outside its page %llx + %llx >= %llx",(uintptr_t)block & ~(0xFFF), block->block_size,(uintptr_t)hdr & ~(0xFFF));
                return 0;
            }
            if (block->block_size >= size){
                free_block *next = block->next;
                if ((uptr)next == 0xDEADBEEFDEADBEEF) next = 0;
                if (block->block_size > size){
                    next = (free_block*)((uptr)block + size);
                    next->block_size = block->block_size - size;
                }
                if (next && ((uintptr_t)next & ~0xFFF) != ((uintptr_t)hdr & ~0xFFF)){
                    alloc_print("[ALLOC] Wrong free block pointer, outside of current page %llx vs %llx",next,hdr);
                    return 0;
                }
                *blk_ptr = next;
                hdr->used += size;
                *(size_t*)block = size;
                void* addr = (void*)((uintptr_t)block + INDIVIDUAL_HDR);
                memset(addr, 0, size - INDIVIDUAL_HDR);
                return addr;
            }
            blk_ptr = &block->next;
            block = block->next;
        }
    }
    
    if (!hdr->next) hdr->next = fallback(PAGE_SIZE);
    return allocate(hdr->next, size, fallback);
}

static void* zalloc_page = 0;

void* zalloc(size_t size){
    if (!zalloc_page) zalloc_page = alloc_proxy(PAGE_SIZE);
    return allocate(zalloc_page, size, alloc_proxy);
}

void release(void* ptr){
    if (!((uintptr_t)ptr & 0xFFF)){
        free_proxy(ptr);
        return;
    }
    
    allocator_header *hdr = (allocator_header*)((uintptr_t)ptr & (~0xFFF));
    
    if (!hdr->used) return;
    
    uintptr_t header = (uintptr_t)ptr - INDIVIDUAL_HDR;
    
    size_t size = *(size_t*)header;
    
    alloc_print("[FREE] Freeing %llx at %llx",size,header);
    
    if (size > 0x1000 - ((uintptr_t)ptr & 0xFFF)){
        alloc_print("[FREE] allocated memory pointed outside of block");
        return;
    }
    
    memset32((void*)header, 0xDEADBEEF, size);
    
    hdr->used -= size;
    if (!hdr->used){
        hdr->free_block = 0;
        hdr->free_mem = 0;
    } else {
        free_block *block = (free_block*)header;
        block->block_size = size;
        block->next = hdr->free_block;
        if (block->next && ((uintptr_t)block->next & ~0xFFF) != ((uintptr_t)hdr & ~0xFFF)){
            alloc_print("[FREE] block free now pointing to other page %llx vs %llx",block->next,hdr);
            return;
        }
        hdr->free_block = block;
        if (((uintptr_t)block & ~0xFFF) != ((uintptr_t)hdr & ~0xFFF)){
            alloc_print("[FREE] free list head now pointing to other page %llx vs %llx",block,hdr);
            return;
        }
        while ((uintptr_t)block->next == header + size){
            free_block *next_block = (free_block*)block->next;
            block->next = next_block->next;
            block->block_size += next_block->block_size;
        } 
    }
}

void* reallocate(void* ptr, size_t new_size){
    //TODO: if possible, once we have a sentinel value in the extra 0x8, we can check if we can just take up more memory without reallocating entirely
    // allocator_header *hdr = (allocator_header*)((uintptr_t)ptr & (~0xFFF));
    
    // if (!hdr->used) return 0;
    
    uintptr_t header = (uintptr_t)ptr - INDIVIDUAL_HDR;
    
    size_t old_size = *(size_t*)header;
    
    // size_t diff = new_size > old_size ? new_size-old_size : size-old_size;
    
    // size_t next_val = *(size_t*)((uintptr_t)ptr + old_size);
    
    // if (next_val == 0 || next_val == 0xDEADBEEFDEADBEEF)
    
    void *new_ptr = allocate((void*)((uintptr_t)ptr & (~0xFFF)), new_size, malloc);
    if (!new_ptr) return 0;
    memcpy(new_ptr, ptr, old_size < new_size ? old_size : new_size);
    release(ptr);
    return new_ptr;
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