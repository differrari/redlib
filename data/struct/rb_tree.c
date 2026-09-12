#include "bt_tree.h"

typedef enum { tree_color_black, tree_color_red } tree_color;

typedef enum { bt_tree_red, bt_tree_black } bt_tree_color;

static inline bool bt_tree_is_red(btnode *node){
    return node && node->metadata == bt_tree_red;
}

void rb_left_rotate(bt_tree *tree, btnode *x){
    btnode* y = x->rh;
    x->rh = y->lh;

    if (y->lh) y->lh->parent = x;
    y->parent = x->parent;

    if (!x->parent) tree->root = y;
    else if (x == x->parent->lh) x->parent->lh = y;
    else x->parent->rh = y;

    y->lh = x;
    x->parent = y;
}

void rb_right_rotate(bt_tree *tree, btnode *x){
    btnode* y = x->lh;
    x->lh = y->rh;

    if (y->rh) y->rh->parent = x;
    y->parent = x->parent;

    if (!x->parent) tree->root = y;
    else if (x == x->parent->rh) x->parent->rh = y;
    else x->parent->lh = y;

    y->rh = x;
    x->parent = y;
}

void rb_tree_balance(bt_tree *tree, btnode *node){
    if (!node->parent) {
        node->metadata = bt_tree_black; 
        return;
    }
    while (node != tree->root && node->parent->metadata == bt_tree_red){
        if (!node->parent->parent) return;
        if (node->parent == node->parent->parent->lh){
            btnode *unc = node->parent->parent->rh;
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
            btnode *unc = node->parent->parent->lh;
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