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
    vec2 offset;
} point_graph;

typedef enum {
    path_render_fill = 1 << 0,
    path_render_outline = 1 << 1,
    path_render_background = 1 << 2,
} path_render_type;

typedef struct {
    u32 scale;
    color background_color;
    color outline_color;
    color color;
    path_render_type type;
} path_render_options;