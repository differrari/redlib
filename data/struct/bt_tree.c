#include "bt_tree.h"
#include "memory/memory.h"
#include "syscalls/syscalls.h"
#include "utils/indent.h"
#include "debug/assert.h"

static inline void* bt_tree_alloc(bt_tree *tree, size_t size){
    return tree && tree->allocator ? tree->allocator(size) : zalloc(size);
}

static inline void bt_tree_free(bt_tree *tree, void *ptr){
    return tree && tree->free ? tree->free(ptr) : release(ptr);
}

bt_node* bt_tree_new_node(bt_tree *tree){
    return bt_tree_alloc(tree, tree->data_size + sizeof(bt_node));
}

static inline bool bt_tree_is_nil(bt_node *node){
    return !node;
}

static inline bool bt_tree_is_leaf(bt_node *node){
    return node && bt_tree_is_nil(node->lh) && bt_tree_is_nil(node->rh);
}

bt_tree bt_tree_create_alloc(size_t data_size, bt_balancing balancing, void *(*allocator)(size_t), void (*free)(void *)){
    if (!allocator || !free) return (bt_tree){};
    return (bt_tree){
        .data_size = data_size,
        .allocator = allocator,
        .free = free,
        .balancing = balancing
    };
}

extern void rb_tree_balance(bt_tree *tree, bt_node *new_node);

void bt_tree_insert(bt_tree *tree, void* data, i64 key){
    if (!tree) return;

    bt_node *new_node = bt_tree_new_node(tree);
    if (tree->data_size) memcpy(new_node+sizeof(bt_node), data, tree->data_size);
    new_node->key = key;

    tree->count++;

    if (!tree->root) {
        tree->root = new_node;
        return;
    }

    bt_node *parent = 0;
    bt_node *current = tree->root;

    while (current){
        if (current->key > key){
            parent = current;
            current = current->lh;
        } else {
            parent = current;
            current = current->rh;
        }
    }

    new_node->parent = parent;

    if (parent->key > key){
        parent->lh = new_node;
    } else {
        parent->rh = new_node;
    }

    switch (tree->balancing){
        case bt_balancing_rb:
            rb_tree_balance(tree,new_node);
            return;
        default: return;
    }

}

void bt_tree_debug_node(bt_node *node, int depth){
    if (!node) {
        // print("NIL");
        return;
    }

    bt_tree_debug_node(node->lh, depth+1);
    print("%s%i - %i",indent_by(depth),node->key,node->metadata);
    bt_tree_debug_node(node->rh, depth+1);
}

void bt_tree_debug(bt_tree *tree){
    if (!tree) return;

    bt_tree_debug_node(tree->root,0);
}


bt_node* bt_tree_leftmost(bt_node *node){
    if (!node) return 0;
    if (node->lh) return bt_tree_leftmost(node->lh);
    return node;
}

void* bt_traversal_reset(bt_tree_traversal *traversal){
    memset(traversal, 0, sizeof(bt_tree_traversal));
    return 0;
}

bt_node* bt_tree_next(bt_tree_traversal *traversal){
    print("%llx",traversal->index);
    if (!traversal || !traversal->tree || !traversal->tree->root || traversal->tree->count <= traversal->index) return bt_traversal_reset(traversal);

    if (!traversal->current){
        if (traversal->index != 0) return bt_traversal_reset(traversal);
        traversal->current = bt_tree_leftmost(traversal->tree->root);
        return traversal->current;
    }

    if (traversal->current->rh){
        traversal->index++;
        traversal->current = bt_tree_leftmost(traversal->current->rh);
        return traversal->current;
    }

    i64 key = traversal->current->key;

    if (traversal->current->parent){
        do {
            traversal->current = traversal->current->parent;
        } while (traversal->current && traversal->current->key < key);
        traversal->index++;
        return traversal->current;
    }

    return 0;

}

bool bt_tree_test_ascending(bt_tree *tree){
    bt_tree_traversal traversal = {
        .tree = tree
    };
    
    bt_node *node = 0;
    i64 cur_value = 0;
    int index = 0;
    while ((node = bt_tree_next(&traversal))){
        index = traversal.index;
        if (index){
            assert_false(node->key < cur_value, "Nodes are not in ascending order, value %i is smaller than value %i",node->key,cur_value);
        }
        cur_value = node->key;
    }

    return true;
}

extern bool rb_tree_test();

bool bt_test(){
    bt_tree testtree = bt_tree_create(0, bt_balancing_none);

    bt_tree_insert(&testtree, 0, 13);
    bt_tree_insert(&testtree, 0, 1);
    bt_tree_insert(&testtree, 0, 15);
    bt_tree_insert(&testtree, 0, 6);
    bt_tree_insert(&testtree, 0, 8);
    bt_tree_insert(&testtree, 0, 22);
    bt_tree_insert(&testtree, 0, 11);
    bt_tree_insert(&testtree, 0, 17);
    bt_tree_insert(&testtree, 0, 25);
    bt_tree_insert(&testtree, 0, 27);

    bt_tree_debug(&testtree);

    assert(bt_tree_test_ascending(&testtree));
    
    assert(rb_tree_test());

    return true;

}