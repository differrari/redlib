#include "types.h"
#include "hashmap.h"
#include "alloc/allocate.h"
#include "std/memory.h"
#include "string/string.h"

static int chm_bytewise_eq(const void* a, u64 alen, const void* b, u64 blen){
    if (alen != blen) return 0;
    const u8* p = (const u8*)a;
    const u8* q = (const u8*)b;
    for (u64 i = 0; i < alen; i++) if (p[i] != q[i]) return 0;
    return 1;
}

u64 hash_map_fnv1a64(const void* data, u64 len){
    const u8* bytes = (const u8*)data;
    u64 h = 0xcbf29ce484222325ULL;
    for (u64 i = 0; i < len; i++) {
        h^= (u64)bytes[i];
        h*= 0x100000001b3ULL; 
    }
    return h;
}

static void* chm_alloc(const hash_map_t* map, uint64_t sz){
    if (!map) return 0;
    if (map && map->alloc) return map->alloc(sz);
    return zalloc(sz);
}

static void chm_free(const hash_map_t* map, void* ptr){
    if (!map) return;
    if (map && map->free){
        map->free(ptr);
        return;
    }
    release(ptr);
}

static uint64_t chm_next_pow2(uint64_t x){
    if (x<8) return 8;
    x--;
    x|=x>>1;
    x|=x>>2;
    x|=x>>4;
    x|=x>>8;
    x|=x>>16;
    x|=x>>32;
    return x + 1;
}

static void chm_update_threshold(hash_map_t* map){
    map->resize_threshold = (map->capacity*3)/4;
}

hash_map_t* hash_map_create_alloc(uint64_t initial_capacity, void* (*alloc)(size_t size),void (*mfree)(void* ptr)){
    if (!alloc || !mfree) return 0;
    uint64_t cap = chm_next_pow2(initial_capacity ? initial_capacity : 8);
    hash_map_t* m = (hash_map_t*)alloc((uint64_t)sizeof(hash_map_t));

    if (!m) return 0;

    m->alloc = alloc; 
    m->free = mfree;
    m->hash_fn = hash_map_fnv1a64;
    m->keyeq_fn = chm_bytewise_eq;
    m->value_dispose = 0;
    m->capacity = cap;
    m->size = 0;
    m->buckets = (hash_map_entry_t**)alloc((uint64_t)sizeof(hash_map_entry_t*)*cap);

    if (!m->buckets) {
        m->free(m);
        return 0;
    }

    memset(m->buckets, 0, (uint64_t)sizeof(hash_map_entry_t*)*cap);
    chm_update_threshold(m);
    return m;
}

hash_map_t* hash_map_create(uint64_t initial_capacity){
    return hash_map_create_alloc(initial_capacity, zalloc, release);
}

void hash_map_destroy(hash_map_t* map){
    if(!map) return;

    for (uint64_t i = 0; i < map->capacity; i++) {
        hash_map_entry_t* e = map->buckets[i];
        while(e){
            hash_map_entry_t* n = e->next;
            if(e->key_len>0 && e->key) chm_free(map,e->key);
            if(map->value_dispose && e->value) map->value_dispose(e->value);
            chm_free(map, e);
            e = n;
        }
    }
    chm_free(map, map->buckets);
    chm_free(map, map);
}

void hash_map_set_allocator(hash_map_t* map, void* (*alloc)(size_t), void (*dealloc)(void*)){
    if (!map) return;
    map->alloc = alloc;
    map->free = dealloc;
}

void hash_map_set_hash(hash_map_t* map, hash_map_hash_fn hash_fn, hash_map_keyeq_fn keyeq_fn){
    if (!map) return;
    if (hash_fn) map->hash_fn = hash_fn;
    if (keyeq_fn) map->keyeq_fn = keyeq_fn;
}

void hash_map_set_value_dispose(hash_map_t* map, void (*dispose_fn)(void*)){
    if(!map) return;
    map->value_dispose = dispose_fn;
}

static int chm_resize(hash_map_t* map, uint64_t new_capacity){
    uint64_t ncap = chm_next_pow2(new_capacity);
    hash_map_entry_t** nb = (hash_map_entry_t**)chm_alloc(map, (uint64_t)sizeof(hash_map_entry_t*)*ncap);
    if (!nb) return 0;

    memset(nb, 0, (uint64_t)sizeof(hash_map_entry_t*)*ncap);
    for (uint64_t i = 0; i < map->capacity; i++) {
        hash_map_entry_t* e = map->buckets[i];
        while (e) {
            hash_map_entry_t* nxt = e->next;
            uint64_t idx = e->hash & (ncap-1);
            e->next = nb[idx];
            nb[idx] = e;
            e = nxt;
        }
    }
    chm_free(map, map->buckets);
    map->buckets = nb;
    map->capacity = ncap;
    chm_update_threshold(map);

    return 1;
}

int hash_map_put(hash_map_t* map, const void* key, uint64_t key_len, void* value){
    if (!map) return -1;

    if (map->size >= map->resize_threshold){
        if (!chm_resize(map, map->capacity << 1)) return -1;
    }
    uint64_t h = map->hash_fn(key,key_len);
    uint64_t idx = h & (map->capacity-1);
    hash_map_entry_t* e = map->buckets[idx];

    while(e){
        if(e->hash == h && map->keyeq_fn(e->key, e->key_len, key, key_len)){
            e->value = value;
            return 0;
        }
        e = e->next;
    }
    hash_map_entry_t* ne = (hash_map_entry_t*)chm_alloc(map, (uint64_t)sizeof(hash_map_entry_t));
    if (!ne) return -1;
    ne->hash = h;
    
    if (key_len>0) {
        void* k = chm_alloc(map, key_len);
        if (!k) { chm_free(map, ne); return -1; }
        memcpy(k, key, key_len);
        ne->key = k;
        ne->key_len = key_len;
    } else {
        ne->key = 0;
        ne->key_len = 0;
    }

    ne->value = value;
    ne->next = map->buckets[idx];
    map->buckets[idx] = ne;
    map->size++;

    return 1;
}

int hash_map_put_dictionary(hash_map_t* map, const char* str, void* value){
    return hash_map_put(map, str, strlen(str), value);
}

void* hash_map_get(const hash_map_t* map, const void* key, uint64_t key_len){
    if (!map) return 0;

    uint64_t h = map->hash_fn(key,key_len);
    uint64_t idx = h & (map->capacity-1);
    hash_map_entry_t* e = map->buckets[idx];

    while(e){
        if(e->hash == h && map->keyeq_fn(e->key, e->key_len, key, key_len)) return e->value;
        e = e->next;
    }
    return 0;
}

void* hash_map_get_dictionary(const hash_map_t *map, const char *str){
    return hash_map_get(map, str, strlen(str));
}

bool hash_map_remove(hash_map_t* map, const void* key, uint64_t key_len, void** out_value){
    if (!map) return false;

    uint64_t h = map->hash_fn(key, key_len);
    uint64_t idx = h & (map->capacity-1);
    hash_map_entry_t* e = map->buckets[idx];
    hash_map_entry_t* prev = 0;

    while(e){
        if (e->hash == h && map->keyeq_fn(e->key, e->key_len, key, key_len)) {
            if (prev) prev->next = e->next;
            else map->buckets[idx] = e->next;

            if (out_value) *out_value = e->value;
            else if(map->value_dispose && e->value) map->value_dispose(e->value);

            if (e->key_len > 0 && e->key) chm_free(map, e->key);
            chm_free(map, e);
            map->size--;
            return true;
        }
        prev = e;
        e = e->next;
    }

    return false;
}

void hash_map_empty(hash_map_t* map){
    if (!map) return;

    for (uint64_t i = 0; i < map->capacity; i++){
        hash_map_entry_t* e = map->buckets[i];
        while(e){
            hash_map_entry_t* next = e->next;
            if (e->key_len>0 && e->key) chm_free(map, e->key);
            if (map->value_dispose && e->value) map->value_dispose(e->value);
            chm_free(map, e);
            e = next;
        }
    }
    
    map->size = 0;
}

uint64_t hash_map_size(const hash_map_t* map){
    if (!map) return 0;
    return map->size;
}

uint64_t hash_map_capacity(const hash_map_t* map){
    if (!map) return 0;
    return map->capacity;
}

void hash_map_for_each(const hash_map_t* map, void (*func)(void* key, uint64_t key_len, void* value)){
    if (!map || !func) return;

    for (uint64_t i = 0; i < map->capacity; i++){
        hash_map_entry_t* e = map->buckets[i];
        while(e){
            hash_map_entry_t* next = e->next;
            func(e->key, e->key_len, e->value);
            e = next;
        }
    }
}
