#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//DEADLINE: 01/03/2026 - clinkedlist will become linked_list

typedef struct linked_list_node {
    void *data;
    struct linked_list_node *next;
} linked_list_node_t;

typedef linked_list_node_t clinkedlist_node_t;

typedef struct {
    linked_list_node_t *head;
    linked_list_node_t *tail;
    uint64_t length;
    void* (*alloc)(size_t size);
    void (*free)(void *ptr, size_t size);
} linked_list_t;

typedef linked_list_t clinkedlist_t;

linked_list_t *linked_list_create();
void linked_list_destroy(linked_list_t *list);
linked_list_t *linked_list_clone(const linked_list_t *list);
void linked_list_push_front(linked_list_t *list, void *data);
void *linked_list_pop_front(linked_list_t *list);
clinkedlist_node_t *linked_list_insert_after(linked_list_t *list, clinkedlist_node_t *node, void *data);

static inline void linked_list_push(linked_list_t *list, void *data){
    linked_list_insert_after(list, list->tail, data);
}

void* linked_list_get(linked_list_t *list, uint64_t index);
void *linked_list_remove(linked_list_t *list, clinkedlist_node_t *node);
void linked_list_update(linked_list_t *list, clinkedlist_node_t *node, void *new_data);
uint64_t linked_list_count(const linked_list_t *list);
uint64_t linked_list_size_bytes(const linked_list_t *list);
clinkedlist_node_t *linked_list_find(linked_list_t *list, void *key, int (*cmp)(void *, void *));
void linked_list_for_each(const linked_list_t *list, void (*func)(void *));

#define clinkedlist_create linked_list_create
#define clinkedlist_destroy linked_list_destroy
#define clinkedlist_clone linked_list_clone
#define clinkedlist_push_front linked_list_push_front
#define clinkedlist_pop_front linked_list_pop_front
#define clinkedlist_insert_after linked_list_insert_after
#define clinkedlist_push linked_list_push
#define clinkedlist_remove linked_list_remove
#define clinkedlist_update linked_list_update
#define clinkedlist_count linked_list_count
#define clinkedlist_size_bytes linked_list_size_bytes
#define clinkedlist_find linked_list_find
#define clinkedlist_for_each linked_list_for_each

#ifdef __cplusplus
}
#endif
