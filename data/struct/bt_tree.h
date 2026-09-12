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
} btnode;

typedef enum { bt_balancing_none, bt_balancing_rb } bt_balancing;

typedef struct {
    btnode *root;
    size_t data_size;
    void* (*allocator)(size_t size);
    void (*free)(void*);
    bt_balancing balancing;
} bt_tree;

bt_tree bt_tree_create_alloc(size_t data_size, bt_balancing balancing, void* (*allocator)(size_t size), void (*free)(void*));

static inline bt_tree bt_tree_create(size_t data_size, bt_balancing balancing){
    return bt_tree_create_alloc(data_size, balancing, zalloc, release);
}

void bt_tree_insert(bt_tree *tree, void* data, i64 key);
void bt_tree_debug(bt_tree *tree);

bool bt_tree_test();