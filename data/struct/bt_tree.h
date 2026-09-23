#pragma once

#include "types.h"
#include "alloc/allocate.h"

//TODO: removal can leave nodes orphaned and not clean them up. This is currently by design (but might change) in case they still need to be addressed (by undo-redo for example). They should be stored somewhere to be cleaned up manually if this remains.
typedef struct bt_node {
    u64 metadata;
    union {
        struct {
            struct bt_node* lh;
            struct bt_node* rh;
        };
        struct bt_node* children[2];
    };
    struct bt_node* parent;
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

bt_node* bt_tree_leftmost(bt_node *node);

static inline bt_node* bt_tree_lowest(bt_node *node){
    return bt_tree_leftmost(node);
}

bt_node* bt_tree_rightmost(bt_node *node);

static inline bt_node* bt_tree_highest(bt_node *node){
    return bt_tree_rightmost(node);
}

bt_node* bt_tree_insert(bt_tree *tree, void* data, i64 key);

static inline bt_node* bt_tree_insert_ptr(bt_tree *tree, void* ptr, i64 key){
    if (tree->data_size != sizeof(uptr)) return 0;
    return bt_tree_insert(tree, &ptr, key);
}

bt_node* bt_tree_find_node(bt_tree *tree, i64 exact_key, void *ctx, tern (*find_query)(void *ctx, bt_tree *tree, bt_node *node));

void bt_tree_remove(bt_tree *tree, bt_node *node);

void bt_tree_debug(bt_tree *tree);

size_t bt_tree_count(bt_tree *tree);

bool bt_test();

void bt_reset(bt_tree *tree);

void bt_destroy(bt_tree *tree);

bt_node* bt_tree_update(bt_tree *tree, bt_node *node, i64 new_key);

typedef struct {
    u64 index;
    bt_tree *tree;
    bt_node *current;
    bool backwards;
} bt_tree_traversal;

bt_node* bt_tree_next(bt_tree_traversal *traversal);
