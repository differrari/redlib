#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//TODO: review allocs & C
extern void* malloc(size_t size);
extern void free_sized(void* ptr, size_t size);

typedef uint64_t (*hash_map_hash_fn)(const void* key, uint64_t len);
typedef int (*hash_map_keyeq_fn)(const void* a, uint64_t alen, const void* b, uint64_t blen);

typedef struct hash_map_entry_t {
    uint64_t hash;
    void* key;
    uint64_t key_len;
    void* value;
    struct hash_map_entry_t* next;
} hash_map_entry_t;

typedef struct hash_map {
    hash_map_entry_t** buckets;
    uint64_t capacity;
    uint64_t size;
    void* (*alloc)(size_t size);
    void (*free)(void* ptr, size_t size);
    hash_map_hash_fn hash_fn;
    hash_map_keyeq_fn keyeq_fn;
    void (*value_dispose)(void* value);
    uint64_t resize_threshold;
} hash_map_t;

hash_map_t* hash_map_create(uint64_t initial_capacity);
hash_map_t* hash_map_create_alloc(uint64_t initial_capacity, void* (*alloc)(size_t size),void (*mfree)(void* ptr, size_t size));
void hash_map_destroy(hash_map_t* map);
void hash_map_set_allocator(hash_map_t* map, void* (*alloc)(size_t), void (*dealloc)(void*, size_t));
void hash_map_set_hash(hash_map_t* map, hash_map_hash_fn hash_fn, hash_map_keyeq_fn keyeq_fn);
void hash_map_set_value_dispose(hash_map_t* map, void (*dispose_fn)(void*));
int hash_map_put(hash_map_t* map, const void* key, uint64_t key_len, void* value);
int hash_map_put_dictionary(hash_map_t* map, const char* key, void* value);
void* hash_map_get(const hash_map_t* map, const void* key, uint64_t key_len);

void* hash_map_get_dictionary(const hash_map_t *map, const char *str);
bool hash_map_remove(hash_map_t* map, const void* key, uint64_t key_len, void** out_value);
void hash_map_empty(hash_map_t* map);
uint64_t hash_map_size(const hash_map_t* map);
uint64_t hash_map_capacity(const hash_map_t* map);
void hash_map_for_each(const hash_map_t* map, void (*func)(void* key, uint64_t key_len, void* value));
uint64_t hash_map_fnv1a64(const void* data, uint64_t len);

#ifdef __cplusplus
}
#endif
