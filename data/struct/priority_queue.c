#include "priority_queue.h"

priority_queue_t* priority_queue_create_alloc(bool ascending, void* (*allocator)(size_t), void (*free)(void*)){
    if (!allocator || !free) return 0;
    priority_queue_t *queue = allocator(sizeof(priority_queue_t));
    queue->allocator = allocator;
    queue->free = free;
    queue->ascending = ascending;
    return queue;
}

priority_queue_item_hdr* priority_queue_make_hdr(priority_queue_t* queue, void *item, i64 priority){
    priority_queue_item_hdr *hdr = queue->allocator(sizeof(priority_queue_item_hdr));
    hdr->priority = priority;
    hdr->ptr = item;
    return hdr;
}

void priority_queue_hdr_insert(priority_queue_t* queue, priority_queue_item_hdr *hdr, void *item, i64 priority){
    if (queue->ascending ? priority < hdr->priority : priority > hdr->priority){
        if (hdr->lh)
            priority_queue_hdr_insert(queue, hdr->lh, item, priority);
        else 
            hdr->lh = priority_queue_make_hdr(queue, item, priority);
    } else {
        if (hdr->rh)
            priority_queue_hdr_insert(queue, hdr->rh, item, priority);
        else 
            hdr->rh = priority_queue_make_hdr(queue, item, priority);
    }
}

void priority_queue_insert(priority_queue_t* queue, void *item, i64 priority){
    if (!queue || !item) return;
    if (!queue->root) 
        queue->root = priority_queue_make_hdr(queue, item, priority);
    else 
        priority_queue_hdr_insert(queue, queue->root, item, priority);
}

void priority_queue_hdr_traverse(priority_queue_t* queue, priority_queue_item_hdr *hdr, void (*call)(priority_queue_t *queue, priority_queue_item_hdr *hdr, void *item, i64 priority)){
    if (hdr->lh) priority_queue_hdr_traverse(queue, hdr->lh, call);
    call(queue, hdr, hdr->ptr, hdr->priority);
    if (hdr->rh) priority_queue_hdr_traverse(queue, hdr->rh, call);
}

void priority_queue_traverse(priority_queue_t *queue, void (*call)(priority_queue_t *queue, priority_queue_item_hdr *hdr, void *item, i64 priority)){
    if (!queue || !queue->root || !call) return;
    priority_queue_hdr_traverse(queue, queue->root, call);
}

void priority_queue_destroy_node(priority_queue_t *queue, priority_queue_item_hdr *hdr, void *item, i64 priority){
    if (!queue->free) return;
    queue->free(hdr);
}

void priority_queue_destroy(priority_queue_t *queue){
    if (!queue || !queue->free) return;
    priority_queue_traverse(queue, priority_queue_destroy_node);
    queue->free(queue);
}