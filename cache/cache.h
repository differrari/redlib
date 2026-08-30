#pragma once

#include "data/struct/hashmap.h"
#include "types.h"
#include "syscalls/syscalls.h"
#include "memory/memory.h"

typedef enum { 
    cache_invalidate_manual = 1 << 0, 
    cache_invalidate_timeout = 1 << 1
} cache_invalidate_policy;

typedef struct  {
    cache_invalidate_policy policy;
    u64 timeout_len;
    size_t keylen;
    size_t initial_capacity;
    bool allow_overwrite;
    hash_map_t *map;
} cache_config;

typedef struct {
    bool manual_invalidate;
    u64 current_timeout;
    size_t len;
} cache_entry;

static inline cache_config cache_configure(cache_config config){
    if (!config.keylen) return (cache_config){};
    if (config.policy & cache_invalidate_timeout && !config.timeout_len) return (cache_config){};
    config.map = hash_map_create(config.initial_capacity ?: config.keylen * 8);
    return config;
}

static bool cache_remove(cache_config cache, void *key){
    return hash_map_remove(cache.map, key, cache.keylen, 0);
}

static void cache_cleanup(cache_config cache){
    if (!cache.map) return;
    for (uint64_t i = 0; i < cache.map->capacity; i++){
        hash_map_entry_t* e = cache.map->buckets[i];
        while(e){
            hash_map_entry_t* next = e->next;
            cache_entry *entry = e->value;
            bool can_invalidate = entry->current_timeout != 0 && (!(cache.policy & cache_invalidate_manual) || entry->manual_invalidate);
            if (can_invalidate){
                if (entry->current_timeout <= get_time()) cache_remove(cache,e->key);
            }
            e = next;
        }
    }
}

static sizedptr cache_prealloc(cache_config cache, void* key, size_t len){
    cache_entry *ptr = hash_map_get(cache.map, key, cache.keylen);
    if (ptr){
        if (!cache.allow_overwrite) return (sizedptr){};
        release(ptr);
    } 
    cache_entry *new_entry = zalloc(sizeof(cache_entry) + len);
    if (!new_entry) return (sizedptr){};
    hash_map_put(cache.map, key, cache.keylen, new_entry);
    if ((cache.policy & cache_invalidate_timeout) && !(cache.policy & cache_invalidate_manual)) new_entry->current_timeout = get_time() + cache.timeout_len;
    new_entry->len = len;
    return (sizedptr){ .ptr = (uptr)new_entry + sizeof(cache_entry), .size = len };
}

static bool cache_exists(cache_config cache, void* key, size_t *len){
    cache_entry *ptr = hash_map_get(cache.map, key, cache.keylen);
    if (!ptr) return false;
    if (len) *len = ptr->len;
    return true;
}

static bool cache_manual_invalidate(cache_config cache, void* key){
    if (!(cache.policy & cache_invalidate_manual)) return false;
    cache_entry *ptr = hash_map_get(cache.map, key, cache.keylen);
    if (!ptr) return false;
    ptr->manual_invalidate = true;
    if (cache.policy & cache_invalidate_timeout){
        ptr->current_timeout = get_time() + cache.timeout_len;
    } else {
        cache_remove(cache, key);
    }
    return true;
}

static sizedptr cache_get(cache_config cache, void* key){
    cache_entry *ptr = hash_map_get(cache.map, key, cache.keylen);
    if (!ptr) return (sizedptr){};
    return (sizedptr){.ptr = (uptr)ptr + sizeof(cache_entry), .size = ptr->len };
}
