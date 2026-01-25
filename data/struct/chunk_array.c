#include "chunk_array.h"
#include "syscalls/syscalls.h"
#include "std/memory.h"

chunk_array_t* chunk_array_create(size_t item_size, size_t chunk_capacity){
    chunk_array_t *array = (chunk_array_t*)zalloc(sizeof(chunk_array_t) + (item_size * chunk_capacity));
    array->item_size = item_size;
    array->chunk_capacity = chunk_capacity;
    return array;
}

chunk_array_t* chunk_array_push(chunk_array_t* array, void *data){
    if (array->count < array->chunk_capacity){
        memcpy((void*)((uintptr_t)array + sizeof(chunk_array_t) + (array->count * array->item_size)),data,array->item_size);
        array->count++;
        return array;
    } else {
        if (!array->next)
            array->next = chunk_array_create(array->item_size, array->chunk_capacity);
        return chunk_array_push(array->next, data);
    }
}

void* chunk_array_get(chunk_array_t *array, uint64_t index){
    if (index < array->count){
        return (void*)((uintptr_t)array + sizeof(chunk_array_t) + (index * array->item_size));
    } else if (array->next){
        return chunk_array_get(array->next, index - array->count);
    } else return 0;
}

size_t chunk_array_count(chunk_array_t *array){
    size_t size = array->count;
    if (array->next) size += chunk_array_count(array->next);
    return size;
}

void chunk_array_destroy(chunk_array_t *array){
    if (array->next) chunk_array_destroy(array->next);
    free_sized(array, sizeof(chunk_array_t) + (array->item_size * array->chunk_capacity));
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
    assert_true(test_array->next, "Chunked Array: should not have next pointer");
    assert_true(chunk_array_count(test_array) == 20, "Chunked Array: Wrong number of calculated items. Expected 20, found %i",test_array->count);
}