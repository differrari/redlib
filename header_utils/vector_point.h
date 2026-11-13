#pragma once

#include "types.h"
#include "math/vector.h"
#include "data/struct/linked_list.h"
#include "math/aabb2.h"

typedef struct {
    vec2 location;
    bool control;
} vector_point;

static i64 insert_vector_point(linked_list_t *list, vector_point *p, i64 latest_point){
    i64 inserted_index = -1;
    if (linked_list_count(list) > 1){//TODO: This needs to handle bezier curves, it needs to keep track of up to two points besides the current and sample on the bezier
        linked_list_node_t *last_node = list->head;
        for (linked_list_node_t *node = list->head->next; node; node = node->next){
            vec2 last = ((vector_point*)last_node->data)->location;
            vec2 next = vector2_sub(((vector_point*)node->data)->location,last);
            float dist = 0;
            if (aabb2_line_intersect((aabb2){
                .min = (vector2){p->location.x-15,p->location.y-15},
                .max = (vector2){p->location.x+15,p->location.y+15}
            }, last, next, &dist, 0) && dist <= 1.f ){
                linked_list_node_t *new_node = linked_list_insert_after(list, last_node, p);
                inserted_index = linked_list_index_of_node(list, new_node);
                break;
            }
            last_node = node;
        }
    }
    if (inserted_index < 0) {
        if (latest_point >= 0){
            linked_list_node_t *latest = linked_list_get_node(list, latest_point);
            if (latest) {
                linked_list_node_t *new_node = linked_list_insert_after(list, latest, p);
                inserted_index = linked_list_index_of_node(list, new_node);
            }
        }
    }
    if (inserted_index < 0) {
        linked_list_node_t *new_node = linked_list_push(list, p);
        inserted_index = linked_list_index_of_node(list, new_node);
    }
    return inserted_index;
}

static linked_list_node_t* find_vector_point(linked_list_t *list, vec2 v, int handle_size){
    for (linked_list_node_t *node = list->head; node; node = node->next){
        vector_point *p = node->data;
        if (!p) continue;
        aabb2 ab = ((aabb2){.min = vector2_sub(p->location, (vector2){handle_size/2.f, handle_size/2.f}), .max = vector2_add(p->location, (vector2){handle_size/2.f, handle_size/2.f})});
        if (aabb_point_intersect(ab, v))
            return node;
    }
    return 0;
}