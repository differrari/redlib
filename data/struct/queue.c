#include "queue.h"
#include "alloc/allocate.h"
#include "std/memory.h"

void* cqueue_alloc(CQueue *q, size_t size) {
    if (q && q->alloc) return q->alloc(size);
    return zalloc(size);
}

void cqueue_free(CQueue *q, void *ptr) {
    if (!ptr) return;
    if (q && q->free) {
        q->free(ptr);
        return;
    }
    release(ptr);
}

void cqueue_init(CQueue* q, uint64_t max_capacity, uint64_t elem_size, cqueue_alloc_fn alloc, cqueue_free_fn free) {
    if (!q) return;
    q->buffer = NULL;
    q->capacity = max_capacity;
    q->max_capacity = max_capacity;
    q->elem_size = elem_size;
    q->head = q->tail = q->length = 0;
    q->alloc = alloc ? alloc : zalloc;
    q->free = free ? free : release;
    if (max_capacity > 0) {
        uintptr_t b = (uintptr_t)cqueue_alloc(q, max_capacity * elem_size);
        if (b) q->buffer = (void*)b;
    }
}

int32_t cqueue_enqueue(CQueue* q, const void* item) {
    if (!q || !item || !q->elem_size) return 0;
    if (q->max_capacity > 0) {
        if (q->length == q->capacity) return 0;
    } else {
        if (q->length == q->capacity) {
            uint64_t old_capacity = q->capacity;
            uint64_t nc = old_capacity ? old_capacity * 2 : 4;
            uintptr_t nb = (uintptr_t)cqueue_alloc(q, nc * q->elem_size);
            if (!nb) return 0;
            void* newb = (void*)nb;
            for (uint64_t i = 0; i < q->length; ++i) {
                uint64_t idx = (q->tail + i) % old_capacity;
                memcpy((uint8_t*)newb + i * q->elem_size,
                       (uint8_t*)q->buffer + idx * q->elem_size,
                       q->elem_size);
            }
            cqueue_free(q, q->buffer);
            q->buffer = newb;
            q->capacity = nc;
            q->head = q->length;
            q->tail = 0;
        }
    }
    if (!q->capacity || !q->buffer) return 0;
    memcpy((uint8_t*)q->buffer + q->head * q->elem_size, item, q->elem_size);
    q->head = (q->head + 1) % q->capacity;
    q->length++;
    return 1;
}

int32_t cqueue_dequeue(CQueue* q, void* out) {
    if (!q || !out || q->length == 0 || !q->buffer) return 0;
    memcpy(out, (uint8_t*)q->buffer + q->tail * q->elem_size, q->elem_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->length--;
    return 1;
}

int32_t cqueue_is_empty(const CQueue* q) {
    return (!q || q->length == 0) ? 1 : 0;
}

uint64_t cqueue_size(const CQueue* q) {
    return q ? q->length : 0;
}

void cqueue_clear(CQueue* q) {
    if (!q) return;
    q->head = q->tail = q->length = 0;
}

void cqueue_destroy(CQueue* q) {
    if (!q) return;
    cqueue_free(q, q->buffer);
    q->buffer = 0;
    q->capacity = 0;
    q->head = q->tail = q->length = 0;
}
