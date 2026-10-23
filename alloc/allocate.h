#pragma once

#include "types.h"
#include "mem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INDIVIDUAL_HDR (sizeof(size_t) * 2)//TODO: use the extra value as a canary to indicate live memory

void* allocate(void* page, size_t size, page_allocator fallback);

void* zalloc(size_t size);

void release(void* ptr);

void* reallocate(void* ptr, size_t new_size);

#ifdef __cplusplus
}
#endif

bool test_zalloc();