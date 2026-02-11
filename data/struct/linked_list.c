#include "linked_list.h"
#include "alloc/allocate.h"
#include "syscalls/syscalls.h"

linked_list_t *linked_list_create_alloc(void* (*alloc)(size_t size), void (*free)(void *ptr)){
    uintptr_t mem = (uintptr_t)alloc(sizeof(linked_list_t));
    if((void *)mem == NULL) return NULL;
    linked_list_t *list = (linked_list_t *)mem;
    list->alloc = alloc;
    list->free = free;
    return list;
}

linked_list_t *linked_list_create(){
    return linked_list_create_alloc(zalloc, release);
}

void* linked_list_alloc(linked_list_t *list, size_t size){
    if (list->alloc) return list->alloc(size);
    return zalloc(size);
}

void linked_list_free(linked_list_t *list, void*ptr, size_t size){
    if (list->free) list->free(ptr);
    return release(ptr);
}

void linked_list_destroy(linked_list_t *list){
    if(list == NULL) return;
    linked_list_node_t *node = list->head;
    while(node){
        linked_list_node_t *next = node->next;
        linked_list_free(list, node, sizeof(linked_list_node_t));
        node = next;
    }
    linked_list_free(list, list, sizeof(linked_list_t));
}

linked_list_t *linked_list_clone(const linked_list_t *list){
    if(list == NULL) return NULL;
    linked_list_t *clone = linked_list_create();
    if(clone == NULL) return NULL;
    linked_list_node_t *it = list->head;
    while(it){
        if(clone->tail){
            linked_list_node_t *new_node = (linked_list_node_t *)zalloc(sizeof(linked_list_node_t));
            new_node->data = it->data;
            new_node->next = NULL;
            clone->tail->next = new_node;
            clone->tail = new_node;
            clone->length++;
        } else {
            linked_list_push_front(clone, it->data);
        }
        it = it->next;
    }
    return clone;
}

void linked_list_push_front(linked_list_t *list, void *data){
    if(list == NULL) return;
    linked_list_node_t *node = (linked_list_node_t *)linked_list_alloc(list, sizeof(linked_list_node_t));
    node->data = data;
    node->next = list->head;
    list->head = node;
    if(list->tail == NULL) list->tail = node;
    list->length++;
}

void *linked_list_pop_front(linked_list_t *list){
    if(list == NULL || list->head == NULL) return NULL;
    linked_list_node_t *node = list->head;
    void *data = node->data;
    list->head = node->next;
    if (list->head == NULL) list->tail = NULL;
    list->length--;
    linked_list_free(list, node, sizeof(linked_list_node_t));
    return data;
}

linked_list_node_t *linked_list_insert_after(linked_list_t *list, linked_list_node_t *node, void *data){
    if(list == NULL) return NULL;
    if(node == NULL){
        linked_list_push_front(list, data);
        return list->head;
    }
    linked_list_node_t *new_node = (linked_list_node_t *)linked_list_alloc(list, sizeof(linked_list_node_t));
    new_node->data = data;
    new_node->next = node->next;
    node->next = new_node;
    if(list->tail == node) list->tail = new_node;
    list->length++;
    return new_node;
}

void *linked_list_remove(linked_list_t *list, linked_list_node_t *node){
    if(list == NULL || node == NULL || list->head == NULL) return NULL;
    if(node == list->head){
        return linked_list_pop_front(list);
    }
    linked_list_node_t *prev = list->head;
    while(prev->next && prev->next != node){
        prev = prev->next;
    }
    if(prev->next != node) return NULL;
    prev->next = node->next;
    if(node == list->tail) list->tail = prev;
    void *data = node->data;
    list->length--;
    linked_list_free(list, node, sizeof(linked_list_node_t));
    return data;
}

void linked_list_update(linked_list_t *list, linked_list_node_t *node, void *new_data){
    (void)list;
    if(node) node->data = new_data;
}

uint64_t linked_list_count(const linked_list_t *list){
    return list ? list->length : 0;
}

uint64_t linked_list_size_bytes(const linked_list_t *list){
    return list ? list->length * sizeof(linked_list_node_t) : 0;
}

linked_list_node_t *linked_list_find(linked_list_t *list, void *key, int (*cmp)(void *, void *)){
    if(list == NULL || cmp == NULL) return NULL;
    for (linked_list_node_t *it = list->head; it; it = it->next){
        if(cmp(it->data, key) == 0) return it;
    }
    return NULL;
}

void linked_list_for_each(const linked_list_t *list, void (*func)(void *)){
    if(list == NULL || func == NULL) return;
    for (linked_list_node_t *it = list->head; it; it = it->next){
        func(it->data);
    }
}

void* linked_list_get(linked_list_t *list, uint64_t index){
    u64 i = 0;
    for (linked_list_node_t *node = list->head; node && i < index; node = node->next, i++){
        if (i == index) return node;
    }
    return 0;
}

//TEST: whole file