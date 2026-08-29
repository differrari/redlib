#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "graphic_types.h"
#include "string/slice.h"
#include "point_graph.h"
#include "textdraw.h"
#include "rects.h"

typedef struct {
    uint32_t img_width;
    uint32_t img_height;
    uint32_t start_x;
    uint32_t start_y;
    bool flip_x;
    bool flip_y;
} image_transform;

//TODO: just use the types i provide in these functions to make them more readable
u32 pixel_blend(u32 p1, u32 p2);
void fb_clear(draw_ctx *ctx, uint32_t color);
void fb_draw_pixel(draw_ctx *ctx, uint32_t x, uint32_t y, color color);
void fb_fill_rect(draw_ctx *ctx, int32_t x, int32_t y, uint32_t width, uint32_t height, color color);
void fb_outline_rect(draw_ctx *ctx, i32 x, i32 y, u32 width, u32 height, u32 size, color color);
void fb_fill_partial_rect(draw_ctx *ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height, color color, uint32_t start_x, uint32_t start_y, uint32_t full_width);
void fb_draw_img(draw_ctx *ctx, uint32_t x, uint32_t y, uint32_t *img, uint32_t img_width, uint32_t img_height);
void fb_draw_partial_img(draw_ctx *ctx, uint32_t *img, uint32_t x, uint32_t y, uint32_t full_width, uint32_t full_height, image_transform transform);
gpu_rect fb_draw_line(draw_ctx *ctx, i32 x0, i32 y0, i32 x1, i32 y1, color color);

uint32_t fb_get_char_size(uint32_t scale);//DEPRECATED

void fb_draw_cursor(draw_ctx *ctx, uint32_t color);

#ifdef __cplusplus
}
#endif