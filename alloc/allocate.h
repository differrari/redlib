#pragma once

#include "types.h"

#define PAGE_SIZE 0x1000
#define ALIGNMENT 0x10

#ifdef __cplusplus
extern "C" {
#endif
typedef void* (*page_allocator)(size_t size);

void* allocate(void* page, size_t size, page_allocator fallback);

void* zalloc(size_t size);

void release(void* ptr);

void* reallocate(void* ptr, size_t new_size);

#ifdef __cplusplus
}
#endif

bool test_zalloc();