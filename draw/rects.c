#include "rects.h"

int try_merge(gpu_rect* a, gpu_rect* b) {
    u32 ax1 = a->point.x;
    u32 ay1 = a->point.y;
    u32 ax2 = ax1 + a->size.width;
    u32 ay2 = ay1 + a->size.height;
    u32 bx1 = b->point.x;
    u32 by1 = b->point.y;
    u32 bx2 = bx1 + b->size.width;
    u32 by2 = by1 + b->size.height;

    if (ax1 >= bx2 || bx1 >= ax2 || ay1 >= by2 || by1 >= ay2)
        return false;

    u32 min_x = a->point.x < b->point.x ? a->point.x : b->point.x;
    u32 min_y = a->point.y < b->point.y ? a->point.y : b->point.y;
    u32 max_x = ax2 > bx2 ? ax2 : bx2;
    u32 max_y = ay2 > by2 ? ay2 : by2;

    a->point.x = min_x;
    a->point.y = min_y;
    a->size.width = max_x - min_x;
    a->size.height = max_y - min_y;

    return true;
}

void mark_dirty(draw_ctx *ctx, u32 x, u32 y, u32 w, u32 h) {
    if (ctx->full_redraw) return;
    if (x >= ctx->width || y >= ctx->height) return;

    if (x + w > ctx->width) w = ctx->width - x;
    if (y + h > ctx->height) h = ctx->height - y;
    if (w == 0 || h == 0) return;

    gpu_rect new_rect = (gpu_rect){{x, y}, {w, h}};

    int merged = 0;
    for (u32 i = 0; i < ctx->dirty_count; i++) {
        if (try_merge(&ctx->dirty_rects[i], &new_rect)) {
            merged = 1;
            break;
        }
    }
    if (!merged) {
        if (ctx->dirty_count < MAX_DIRTY_RECTS) {
            ctx->dirty_rects[ctx->dirty_count++] = new_rect;
        } else {
            ctx->full_redraw = 1;
            return;
        }
    }

    for (u32 i = 0; i < ctx->dirty_count; ++i) {
        for (u32 j = i + 1; j < ctx->dirty_count; ) {
            if (try_merge(&ctx->dirty_rects[i], &ctx->dirty_rects[j])) {
                for (u32 k = j + 1; k < ctx->dirty_count; ++k)
                    ctx->dirty_rects[k - 1] = ctx->dirty_rects[k];
                ctx->dirty_count--;
            } else {
                ++j;
            }
        }
    }

    uint64_t area_sum = 0;
    for (u32 r = 0; r < ctx->dirty_count; ++r)
        area_sum += (uint64_t)ctx->dirty_rects[r].size.width * (uint64_t)ctx->dirty_rects[r].size.height;

    const uint64_t screen_area = (uint64_t)ctx->width * (uint64_t)ctx->height;
    if (area_sum * 100 >= screen_area * FULL_REDRAW_THRESHOLD_PCT) ctx->full_redraw = 1;
}