#pragma once

#include "types.h"

typedef struct {
    float x;
    float y;
    u8 on_curve;
    bool rsvd[3];
} point_entry;

typedef struct {
    int start;
    int end;
} point_graph_slice;

typedef struct {
    int num_points;
    point_entry *graph;
    int num_slices;
    point_graph_slice *slices;
} point_graph;