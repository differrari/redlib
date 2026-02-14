#pragma once

#include "types.h"
#include "mem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* allocate(void* page, size_t size, page_allocator fallback);

void* zalloc(size_t size);

void release(void* ptr);

void* reallocate(void* ptr, size_t new_size);

#ifdef __cplusplus
}
#endif

bool test_zalloc();