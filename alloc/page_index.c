#include "page_index.h"
#include "syscalls/syscalls.h"

void register_page_alloc(page_index *index, void *ptr, size_t size, page_allocator fallback){
    if (!index) return;
    if (!index){
        print("[ALLOC error] registering allocation with no index");
        return;
    }
    if (!ptr || !size){
        print("[ALLOC error] trying to register null allocation");
        return;
    }
    while (index->header.next) {
        index = index->header.next;
    }
    if (index->header.size >= PAGE_INDEX_LIMIT){
        index->header.next = fallback(PAGE_SIZE);
        index = index->header.next;
    }
    index->ptrs[index->header.size].ptr = ptr;
    index->ptrs[index->header.size++].size = size;
}

void unregister_page_alloc(page_index *index, void *ptr){
    if (!index){
        print("[ALLOC error] freeing allocation with no index");
        return;
    }
    if (!ptr){
        print("[ALLOC error] trying to un-register null allocation");
        return;
    }
    for (page_index *ind = index; ind; ind = ind->header.next){
        for (u64 i = 0; i < ind->header.size; i++){
            if (ind->ptrs[i].ptr == ptr){
                page_index_entry last = ind->ptrs[ind->header.size-1];
                ind->ptrs[i].ptr = last.ptr;
                ind->ptrs[i].size = last.size;
                ind->ptrs[ind->header.size-1].ptr = 0;
                ind->ptrs[ind->header.size-1].size = 0;
                ind->header.size--;
                return;
            }
        }
    }
    print("[ALLOC error] trying to free non-registered page");
}

size_t get_alloc_size(page_index *index, void *ptr){
    if (!index) return 0;
    if (!ptr) return 0;
    for (page_index *ind = index; ind; ind = ind->header.next)
        for (u64 i = 0; i < ind->header.size; i++)
            if (ind->ptrs[i].ptr == ptr)
                return ind->ptrs[i].size;
    return 0;
}