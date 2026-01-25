#pragma once

#include "types.h"

typedef struct chunk_array_t {
    size_t chunk_capacity;
    size_t item_size;
    size_t count;
    struct chunk_array_t *next;
} chunk_array_t;

chunk_array_t* chunk_array_create(size_t item_size, size_t chunk_capacity);
chunk_array_t* chunk_array_push(chunk_array_t* array, void *data);
void chunk_array_destroy(chunk_array_t *array);
//TODO: can be optimized by keeping a convenience pointer to the latest chunk & amount of chunks. 
void* chunk_array_get(chunk_array_t *array, uint64_t index);
size_t chunk_array_count(chunk_array_t *array);