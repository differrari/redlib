#pragma once

#include "allocate.h"
#include "syscalls/syscalls.h"
#include "memory/memory.h"

static void *alloc_page = 0;

static inline void* alloc(size_t size){
    if (!alloc_page) alloc_page = page_alloc(PAGE_SIZE);
    return allocate(alloc_page, size, page_alloc);
}

#define new(type) alloc(sizeof(type))
