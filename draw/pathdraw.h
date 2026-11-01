#pragma once

#include "graphic_types.h"
#include "point_graph.h"
#include "draw/draw.h"

static inline int path_autosize(path_render_options options){
    switch (options.sizing_policy) {
        case path_render_sizing_char: return CHAR_SIZE;
        case path_render_sizing_none: return 1;
    }
}

#define PATH_SIZE {(options.scale*path_autosize(options)*graph.size.x), (options.scale*path_autosize(options)*graph.size.y)}

gpu_size fb_fill_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options);
gpu_size fb_draw_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options);

static inline gpu_size fb_render_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options){
    if (graph.offset.x != 0)
        point.x += graph.offset.x*options.scale*path_autosize(options);
    gpu_size size = PATH_SIZE;
    if (options.type & path_render_background){
        fb_fill_rect(ctx, point.x, point.y, size.width, size.height, options.background_color);
    }
    if (options.type & path_render_fill){
        fb_fill_path(ctx, point, graph, options);
    }
    if (options.type & path_render_outline){
        fb_draw_path(ctx, point, graph, options);
    }
    return size;
}