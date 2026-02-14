#pragma once

#define PAGE_SIZE 0x1000
#define ALIGNMENT 0x10

#define ALIGN_4KB 0x1000
#define ALIGN_16B 0x10
#define ALIGN_64B 0x40

#include "types.h"

typedef void* (*page_allocator)(size_t size);