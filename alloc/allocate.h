#pragma once

#include "types.h"

#define PAGE_SIZE 0x1000
#define ALIGNMENT 0x10

typedef void* (*page_allocator)(size_t size);

void* allocate(void* page, size_t size, page_allocator fallback);

void* zalloc(size_t size);