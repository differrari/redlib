#include "gc.h"
#include "allocate.h"
#include "data/struct/linked_list.h"
#define MOG_MODULE "GC"
#define MOG_LEVEL MOG_VERBOSE
#include "utils/mog.h"

linked_list_t *gc_white = 0;
linked_list_t *gc_black = 0;
linked_list_t *gc_gray = 0;

void gc_register_alloc(void *ptr){
    mog(VERBOSE,"registering alloc %llx",ptr);
    if (!gc_white) gc_white = linked_list_create();
    if (!gc_black) gc_black = linked_list_create();
    if (!gc_gray) gc_gray = linked_list_create();
    if (!ptr) return;
    linked_list_push_front(gc_black, ptr);
}

void gc_run_next_node(void* node){
    for (linked_list_node_t *n = gc_gray->head; n; n = n->next){
        if (n->data == node){
            mog(VERBOSE,"Removing from gray %llx",node);
            linked_list_remove(gc_gray, node);
        }
    }
    size_t obj_size = *(size_t*)((uptr)node - INDIVIDUAL_HDR);
    for (u64 i = 0; i < obj_size; i += 8){
        u64 val = *(u64*)((uptr)node + i);
        if (!val) continue;
        if (node <= (void*)val && node + obj_size >= (void*)val) continue;
        for (linked_list_node_t *n = gc_white->head; n; n = n->next){
            if (n->data == (void*)val){
                mog(VERBOSE,"%llx -> %llx",node,val);
                linked_list_remove(gc_white, n);
                linked_list_push_front(gc_gray, (void*)val);
                break;
            }
        }
    }
    linked_list_push_front(gc_black, node);
}

void gc_run(void* root){
    if (!gc_white) gc_white = linked_list_create();
    if (!gc_black) gc_black = linked_list_create();
    if (!gc_gray) gc_gray = linked_list_create();
    void *obj = 0;
    while ((obj = linked_list_pop_front(gc_gray))) linked_list_push_front(gc_white, obj);
    while ((obj = linked_list_pop_front(gc_black))){ if (obj != root) linked_list_push_front(gc_white, obj); }
    gc_run_next_node(root);
}

void gc_collect_condemned(){
    void *obj = 0;
    while ((obj = linked_list_pop_front(gc_white))){ 
        mog(VERBOSE,"Releasing unused object %llx",obj);
        release(obj);
    }
}

bool gc_run_next(){
    if (!gc_white && !gc_black && !gc_gray) return false;
    void *obj = linked_list_pop_front(gc_gray);
    if (!obj){
        gc_collect_condemned();
        return false;
    }
    gc_run_next_node(obj);
    return true;
}