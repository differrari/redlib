#pragma once

#include "types.h"
#include "alloc/allocate.h"

typedef struct priority_queue_item_hdr {
    i64 priority;
    struct priority_queue_item_hdr *lh;
    struct priority_queue_item_hdr *rh;
    void *ptr;
} priority_queue_item_hdr;

typedef struct {
    void* (*allocator)(size_t);
    void (*free)(void*);
    priority_queue_item_hdr *root;  
    bool ascending;
} priority_queue_t;

priority_queue_t* priority_queue_create_alloc(bool ascending, void* (*allocator)(size_t), void (*free)(void*));

static inline priority_queue_t* priority_queue_create(bool ascending){
    return priority_queue_create_alloc(ascending, zalloc, release);
}

void priority_queue_insert(priority_queue_t* queue, void *item, i64 priority);
void priority_queue_traverse(priority_queue_t *queue, void (*call)(priority_queue_t *queue, priority_queue_item_hdr *hdr, void *item, i64 priority));
void priority_queue_destroy(priority_queue_t *queue);