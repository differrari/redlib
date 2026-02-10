#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//TODO: review allocs & C
extern void* malloc(size_t size);
extern void free_sized(void* ptr, size_t size);

typedef uint64_t (*chashmap_hash_fn)(const void* key, uint64_t len);
typedef int (*chashmap_keyeq_fn)(const void* a, uint64_t alen, const void* b, uint64_t blen);

typedef struct hash_map_entry_t {
    uint64_t hash;
    void* key;
    uint64_t key_len;
    void* value;
    struct hash_map_entry_t* next;
} hash_map_entry_t;

typedef hash_map_entry_t chashmap_entry_t;

typedef struct hash_map {
    hash_map_entry_t** buckets;
    uint64_t capacity;
    uint64_t size;
    void* (*alloc)(size_t size);
    void (*free)(void* ptr, size_t size);
    chashmap_hash_fn hash_fn;
    chashmap_keyeq_fn keyeq_fn;
    void (*value_dispose)(void* value);
    uint64_t resize_threshold;
} hash_map_t;

typedef hash_map_t chashmap_t;

hash_map_t* hash_map_create(uint64_t initial_capacity);
hash_map_t* hash_map_create_alloc(uint64_t initial_capacity, void* (*alloc)(size_t size),void (*mfree)(void* ptr, size_t size));
void hash_map_destroy(hash_map_t* map);
void hash_map_set_allocator(hash_map_t* map, void* (*alloc)(size_t), void (*dealloc)(void*, size_t));
void hash_map_set_hash(hash_map_t* map, chashmap_hash_fn hash_fn, chashmap_keyeq_fn keyeq_fn);
void hash_map_set_value_dispose(hash_map_t* map, void (*dispose_fn)(void*));
int hash_map_put(hash_map_t* map, const void* key, uint64_t key_len, void* value);
void* hash_map_get(const hash_map_t* map, const void* key, uint64_t key_len);
int hash_map_remove(hash_map_t* map, const void* key, uint64_t key_len, void** out_value);
uint64_t hash_map_size(const hash_map_t* map);
uint64_t hash_map_capacity(const hash_map_t* map);
void hash_map_for_each(const hash_map_t* map, void (*func)(void* key, uint64_t key_len, void* value));
uint64_t hash_map_fnv1a64(const void* data, uint64_t len);

#define chashmap_create hash_map_create
#define chashmap_create_alloc hash_map_create_alloc
#define chashmap_destroy hash_map_destroy
#define chashmap_set_allocator hash_map_set_allocator
#define chashmap_set_hash hash_map_set_hash
#define chashmap_set_value_dispose hash_map_set_value_dispose
#define chashmap_put hash_map_put
#define chashmap_get hash_map_get
#define chashmap_remove hash_map_remove
#define chashmap_size hash_map_count
#define chashmap_capacity hash_map_capacity
#define chashmap_for_each hash_map_for_each
#define chashmap_fnv1a64 hash_map_fnv1a64

#ifdef __cplusplus
}
#endif
