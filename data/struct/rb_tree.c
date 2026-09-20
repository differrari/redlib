#include "bt_tree.h"
#include "debug/assert.h"
#include "math/math.h"

typedef enum { bt_tree_red, bt_tree_black } bt_tree_color;

static inline bool bt_tree_is_red(bt_node *node){
    return node && node->metadata == bt_tree_red;
}

void rb_left_rotate(bt_tree *tree, bt_node *x){
    bt_node* y = x->rh;
    x->rh = y->lh;

    if (y->lh) y->lh->parent = x;
    y->parent = x->parent;

    if (!x->parent) tree->root = y;
    else if (x == x->parent->lh) x->parent->lh = y;
    else x->parent->rh = y;

    y->lh = x;
    x->parent = y;
}

void rb_right_rotate(bt_tree *tree, bt_node *x){
    bt_node* y = x->lh;
    x->lh = y->rh;

    if (y->rh) y->rh->parent = x;
    y->parent = x->parent;

    if (!x->parent) tree->root = y;
    else if (x == x->parent->rh) x->parent->rh = y;
    else x->parent->lh = y;

    y->rh = x;
    x->parent = y;
}

void rb_tree_balance(bt_tree *tree, bt_node *node){
    if (!node->parent) {
        node->metadata = bt_tree_black; 
    }
    while (node != tree->root && node->parent->metadata == bt_tree_red){
        if (!node->parent->parent) break;
        if (node->parent == node->parent->parent->lh){
            bt_node *unc = node->parent->parent->rh;
            if (unc && unc->metadata == bt_tree_red){
                node->parent->metadata = bt_tree_black;
                unc->metadata = bt_tree_black;
                node->parent->parent->metadata = bt_tree_red;
                node = node->parent->parent;
            } else {
                if (node == node->parent->rh){
                    node = node->parent;
                    rb_left_rotate(tree,node);
                }
                node->parent->metadata = bt_tree_black;
                node->parent->parent->metadata = bt_tree_red;
                rb_right_rotate(tree,node->parent->parent);
            }
        } else {
            bt_node *unc = node->parent->parent->lh;
            if (unc && unc->metadata == bt_tree_red){
                node->parent->metadata = bt_tree_black;
                unc->metadata = bt_tree_black;
                node->parent->parent->metadata = bt_tree_red;
                node = node->parent->parent;
            } else {
                if (node == node->parent->lh){
                    node = node->parent;
                    rb_right_rotate(tree,node);
                }
                node->parent->metadata = bt_tree_black;
                node->parent->parent->metadata = bt_tree_red;
                rb_left_rotate(tree,node->parent->parent);
            }
        }
    }
    if (tree->root) tree->root->metadata = bt_tree_black;
}

void rb_tree_swap_balance(bt_tree *tree, bt_node *node, bt_node *replacement){
    if (node->metadata == replacement->metadata) return;
    print("Need to rebalance");
    //TODO: rebalance rb tree
}

extern bool bt_tree_test_ascending(bt_tree *tree);

bool rb_tree_test(){
    bt_tree testtree = bt_tree_create(0, bt_balancing_rb);

    rng_t rng = {};
    rng_seed(&rng, get_time());

    u8 node_count = rng_next8(&rng);

    for (int i = 0; i < node_count; i++){
        bt_tree_insert(&testtree, 0, rng_next64(&rng));
    }

    bt_tree_debug(&testtree);

    assert(bt_tree_test_ascending(&testtree));

    assert_eq(node_count, (u8)testtree.count, "Wrong number of nodes, inserted %i, count was %i",node_count,testtree.count);

    bt_tree_traversal traversal = {
        .tree = &testtree
    };
    
    bt_node *node = 0;
    int black_count = -1;
    while ((node = bt_tree_next(&traversal))){
        assert_true(node->metadata == bt_tree_red || node->metadata == bt_tree_black, "Node %i has wrong color %i",node->key,node->metadata);
        if (node->parent){
            assert_false(node->metadata == bt_tree_red && node->parent->metadata == bt_tree_red,  "Node %i is red and has red child %i",node->parent->key,node->key);
        } else 
            assert_true(node->metadata == bt_tree_black && traversal.tree->root == node, "Wrong state of root %i",node->metadata);
        int count_children = 0;
        bool has_black = false;
        for (int i = 0; i < 2; i++){
            if (node->children[i]){
                count_children++;
                has_black |= node->children[i]->metadata == bt_tree_black;
            } else {
                int local_black_count = 1;
                bt_node *lnode = node;
                while (lnode){
                    if (lnode->metadata == bt_tree_black) local_black_count++;
                    lnode = lnode->parent;
                }
                if (black_count < 0) black_count = local_black_count;
                else assert_eq(black_count,local_black_count,"Mismatched number of black nodes. Expected %i got %i in node %i",black_count,local_black_count,node->key);
            }
        }
        assert_false(count_children == 1 && has_black, "Node %i has only one child but it is black",node->key);
    }

    return true;
}