#pragma once

#include "mem_types.h"

typedef struct {
    size_t size;
    struct page_index *next;
} page_index_hdr;

typedef struct {
    void *ptr;
    size_t size;
} page_index_entry;

typedef struct page_index {
    page_index_hdr header;
    page_index_entry ptrs[];
} page_index;

#define PAGE_INDEX_LIMIT (PAGE_SIZE-sizeof(page_index_hdr))/sizeof(page_index_entry)

void register_page_alloc(page_index *index, void *page, size_t size, page_allocator fallback);
void unregister_page_alloc(page_index *index, void *ptr);
size_t get_alloc_size(page_index *index, void *ptr);