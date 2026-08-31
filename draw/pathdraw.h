#pragma once

#include "graphic_types.h"
#include "point_graph.h"
#include "draw/draw.h"
#include "syscalls/syscalls.h"

#define PATH_SIZE {(options.scale*CHAR_SIZE*graph.size.x), (options.scale*CHAR_SIZE*graph.size.y)}

gpu_size fb_fill_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options);
gpu_size fb_draw_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options);

static inline gpu_size fb_render_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options){
    if (graph.offset.x != 0)
        point.x += graph.offset.x*options.scale*CHAR_SIZE;
    gpu_size size = PATH_SIZE;
    if (options.type & path_render_background){
        fb_fill_rect(ctx, point.x, point.y, size.width, size.height, options.background_color);
    }
    if (options.type & path_render_fill){
        fb_fill_path(ctx, point, graph, options);
    }
    if (options.type & path_render_outline){
        path_render_options new_options = options;
        new_options.color = new_options.outline_color;
        fb_draw_path(ctx, point, graph, new_options);
    }
    return size;
}