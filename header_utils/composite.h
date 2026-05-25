#pragma once

#include "graphic_types.h"
#include "types.h"

static void composite(draw_ctx *in_ctx, int_point offset, int zoom_scale, draw_ctx *ex_ctx){

    int32_t sx = offset.x;
    int32_t sy = offset.y;

    sx /= zoom_scale;
    sy /= zoom_scale;
    
    if (sx >= (int32_t)ex_ctx->width || sy >= (int32_t)ex_ctx->height || sx + in_ctx->width <= 0 || sy + in_ctx->height <= 0){
        return;
    }

    int32_t w = in_ctx->width;
    int32_t h = in_ctx->height;
    
    w /= zoom_scale;
    h /= zoom_scale;

    uint32_t ox = 0;
    uint32_t oy = 0;
    
    if (sx < 0){
        w -= -sx;
        ox = -sx;
        sx = 0;
    }
    if (sy < 0){
        h -= -sy;
        oy = -sy;
        sy = 0;
    }

    if (sx + w > (i32)ex_ctx->width) w = ex_ctx->width - sx;
    else if (sx < 0){ w += sx; ox = -sx; sx = 0; }
    if (sy + h > (i32)ex_ctx->height) h = ex_ctx->height - sy;
    else if (sy < 0){ h += sy; oy = -sy; sy = 0; }
    if (w <= 0 || h <= 0){
        return;
    } 

    if (in_ctx->full_redraw){
        for (i32 dy = 0; dy < h; dy++)
            memcpy(ex_ctx->fb + ((sy + dy) * ex_ctx->width) + sx, in_ctx->fb + ((dy + oy) * in_ctx->width) + ox, w * sizeof(color));
        mark_dirty(ex_ctx, sx, sy, w, h);
    } else {
        for (uint32_t dr = 0; dr < in_ctx->dirty_count; dr++){
            gpu_rect r = in_ctx->dirty_rects[dr];
            for (u32 dy = 0; dy < r.size.height; dy++)
                memcpy(ex_ctx->fb + ((sy + dy + r.point.y) * ex_ctx->width) + sx + r.point.x, in_ctx->fb + ((dy + oy + r.point.y) * in_ctx->width) + r.point.x + ox, r.size.width * sizeof(color));
            mark_dirty(ex_ctx, sx + r.point.x, sy + r.point.y, r.size.width, r.size.height);
        }
    }
}