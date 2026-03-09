#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct linked_list_node {
    void *data;
    struct linked_list_node *next;
} linked_list_node_t;

typedef struct {
    linked_list_node_t *head;
    linked_list_node_t *tail;
    uint64_t length;
    void* (*alloc)(size_t size);
    void (*free)(void *ptr);
} linked_list_t;

linked_list_t *linked_list_create_alloc(void* (*alloc)(size_t size), void (*free)(void *ptr));
linked_list_t *linked_list_create();
void linked_list_destroy(linked_list_t *list);
linked_list_t *linked_list_clone(const linked_list_t *list);
void linked_list_push_front(linked_list_t *list, void *data);
void *linked_list_pop_front(linked_list_t *list);
linked_list_node_t *linked_list_insert_after(linked_list_t *list, linked_list_node_t *node, void *data);

static inline void linked_list_push(linked_list_t *list, void *data){
    linked_list_insert_after(list, list->tail, data);
}

void* linked_list_get(linked_list_t *list, uint64_t index);
void *linked_list_remove(linked_list_t *list, linked_list_node_t *node);
void linked_list_update(linked_list_t *list, linked_list_node_t *node, void *new_data);
uint64_t linked_list_count(const linked_list_t *list);
uint64_t linked_list_size_bytes(const linked_list_t *list);
linked_list_node_t *linked_list_find(linked_list_t *list, void *key, int (*cmp)(void *, void *));
void linked_list_for_each(const linked_list_t *list, void (*func)(void *));

#ifdef __cplusplus
}
#endif
