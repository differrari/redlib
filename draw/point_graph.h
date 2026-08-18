#pragma once

#include "types.h"
#include "math/vector_types.h"
#include "graphic_types.h"

typedef struct {
    vec2 pos;
    u8 on_curve;
    bool rsvd[3];
} point_entry;

typedef enum { point_graph_curve_none, point_graph_curve_quad_only, point_graph_curve_quad_cube } point_graph_curve_type;

typedef struct {
    int start;
    int end;
    bool close;
    point_graph_curve_type curve_type;
    i8 max_curve_points;
} point_graph_slice;

typedef struct {
    int num_points;
    point_entry *graph;
    int num_slices;
    point_graph_slice *slices;
    vec2 size;
} point_graph;