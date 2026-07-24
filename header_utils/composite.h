#pragma once

#include "graphic_types.h"
#include "types.h"
#include "draw/draw.h"
#include "memory/memory.h"
#include "syscalls/syscalls.h"
#include "math/math.h"

//Can this be made more optimized if we consider it's meant to tile, not overlap?
static void composite(draw_ctx *source_ctx, int_point offset, float zoom_scale, draw_ctx *destination_ctx, gpu_rect drawable_area){
    
    i32 sx = offset.x;
    i32 sy = offset.y;

    sx /= zoom_scale;
    sy /= zoom_scale;

    if (drawable_area.size.width > destination_ctx->width) drawable_area.size.width = destination_ctx->width;
    if (drawable_area.size.height > destination_ctx->height) drawable_area.size.height = destination_ctx->height;
    
    if (sx >= (i32)drawable_area.size.width || sy >= (i32)destination_ctx->height || sx + source_ctx->width <= 0 || sy + source_ctx->height <= 0){
        return;
    }

    i32 w = source_ctx->width;
    i32 h = source_ctx->height;

    w /= zoom_scale;
    h /= zoom_scale;

    i32 ox = 0;
    i32 oy = 0;
    
    if (sx < drawable_area.point.x){
        w -= drawable_area.point.x-sx;
        ox = drawable_area.point.x-sx;
        sx = drawable_area.point.x;
    }
    if (sy < drawable_area.point.y){
        h -= drawable_area.point.y-sy;
        oy = drawable_area.point.y-sy;
        sy = drawable_area.point.y;
    }

    if ((sx-drawable_area.point.x) + w > (i32)drawable_area.size.width) w = drawable_area.size.width - (sx-drawable_area.point.x);
    if ((sy-drawable_area.point.y) + h > (i32)drawable_area.size.height) h = drawable_area.size.height - (sy-drawable_area.point.y);

    if (w <= 0 || h <= 0){
        return;
    } 

    if (zoom_scale != 1){
        for (i32 dy = 0; dy < h; dy++)
            for (i32 dx = 0; dx < w; dx++)
                destination_ctx->fb[((sy + dy) * destination_ctx->width) + (sx + dx)] = source_ctx->fb[((floor_to_int(dy * zoom_scale) + oy) * source_ctx->width) + (floor_to_int(dx * zoom_scale) + ox)];
        mark_dirty(destination_ctx, sx, sy, w, h);
    } else {
        if (source_ctx->full_redraw){
            for (i32 dy = 0; dy < h; dy++)
                memcpy(destination_ctx->fb + ((sy + dy) * drawable_area.size.width) + sx, source_ctx->fb + ((dy + oy) * source_ctx->width) + ox, w * sizeof(color));
            mark_dirty(destination_ctx, sx, sy, w, h);
        } else {
            for (u32 dr = 0; dr < source_ctx->dirty_count; dr++){
                gpu_rect r = source_ctx->dirty_rects[dr];
                for (u32 dy = 0; dy < r.size.height; dy++)
                    memcpy(destination_ctx->fb + ((sy + dy + r.point.y) * drawable_area.size.width) + sx + r.point.x, source_ctx->fb + ((dy + oy + r.point.y) * source_ctx->width) + r.point.x + ox, r.size.width * sizeof(color));
                mark_dirty(destination_ctx, sx + r.point.x, sy + r.point.y, r.size.width, r.size.height);
            }
        }
    }

    source_ctx->dirty_count = 0;
    source_ctx->full_redraw = false;
}