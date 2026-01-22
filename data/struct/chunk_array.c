#include "chunk_array.h"
#include "alloc/allocate.h"
#include "std/memory.h"

#define CHUNK_ARRAY_ITEM(index) (void*)((uintptr_t)array + sizeof(chunk_array_t) + (index * array->item_size))

chunk_array_t* chunk_array_create(size_t item_size, size_t chunk_capacity){
    chunk_array_t *array = (chunk_array_t*)zalloc(sizeof(chunk_array_t) + (item_size * chunk_capacity));
    array->item_size = item_size;
    array->chunk_capacity = chunk_capacity;
    return array;
}

size_t chunk_array_push(chunk_array_t* array, void *data){
    if (!array) return 0;
    if (array->count < array->chunk_capacity){
        memcpy((void*)((uintptr_t)array + sizeof(chunk_array_t) + (array->count * array->item_size)), data, array->item_size);
        return array->count++;
    } else {
        if (!array->next)
            array->next = chunk_array_create(array->item_size, array->chunk_capacity);
        return array->count + chunk_array_push(array->next, data);
    }
}

void* chunk_array_get(chunk_array_t *array, uint64_t index){
    if (!array) return 0;
    if (index < array->count){
        return CHUNK_ARRAY_ITEM(index);
    } else if (array->next){
        return chunk_array_get(array->next, index - array->count);
    } else return 0;
}

size_t chunk_array_count(chunk_array_t *array){
    if (!array) return 0;
    size_t size = array->count;
    if (array->next) size += chunk_array_count(array->next);
    return size;
}

void chunk_array_destroy(chunk_array_t *array){
    if (!array) return;
    if (array->next) chunk_array_destroy(array->next);
    release(array);
}

void* chunk_array_find(chunk_array_t *array, void *query, bool (*match)(void* value, void *query)){
    if (!array) return 0;
    for (size_t i = 0; i < array->count; i++){
        if (match(CHUNK_ARRAY_ITEM(i), query)) return CHUNK_ARRAY_ITEM(i);
    }
    if (array->next) return chunk_array_find(array->next, query, match);
    return 0;
}

void chunk_array_reset(chunk_array_t *array){
    if (!array) return;
    array->count = 0;
    if (array->next) chunk_array_reset(array->next);
}

#include "test.h"

bool chunk_array_test(){
    chunk_array_t *test_array = chunk_array_create(sizeof(uint64_t), 10);
    
    for (uint64_t i = 0; i < 10; i++){
        chunk_array_push(test_array, &i);
    }
    
    assert_true(test_array->count == 10, "Chunked Array: Wrong number of items. Expected 10, found %i",test_array->count);
    assert_true(chunk_array_count(test_array) == 10, "Chunked Array: Wrong number of calculated items. Expected 10, found %i",test_array->count);
    assert_false(test_array->next, "Chunked Array: should not have next pointer");
    
    for (uint64_t i = 0; i < 10; i++){
        chunk_array_push(test_array, &i);
    }
    
    assert_true(test_array->count == 10, "Chunked Array: Wrong number of items in chunk. Expected 10, found %i",test_array->count);
    assert_true(test_array->next > 0, "Chunked Array: should not have next pointer");
    assert_true(chunk_array_count(test_array) == 20, "Chunked Array: Wrong number of calculated items. Expected 20, found %i",test_array->count);
    
    return true;
}