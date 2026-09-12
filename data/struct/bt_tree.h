#pragma once

#include "types.h"
#include "alloc/allocate.h"

typedef struct btnode {
    u64 metadata;
    union {
        struct {
            struct btnode* lh;
            struct btnode* rh;
        };
        struct btnode* children[2];
    };
    struct btnode* parent;
    i64 key;
    u8 data[];
} bt_node;

typedef enum { bt_balancing_none, bt_balancing_rb } bt_balancing;

typedef struct {
    bt_node *root;
    size_t data_size;
    void* (*allocator)(size_t size);
    void (*free)(void*);
    bt_balancing balancing;
    size_t count;
} bt_tree;

bt_tree bt_tree_create_alloc(size_t data_size, bt_balancing balancing, void* (*allocator)(size_t size), void (*free)(void*));

static inline bt_tree bt_tree_create(size_t data_size, bt_balancing balancing){
    return bt_tree_create_alloc(data_size, balancing, zalloc, release);
}

void bt_tree_insert(bt_tree *tree, void* data, i64 key);
void bt_tree_debug(bt_tree *tree);

bool bt_test();

typedef struct {
    u64 index;
    bt_tree *tree;
    bt_node *current;
} bt_tree_traversal;

bt_node* bt_tree_next(bt_tree_traversal *traversal);