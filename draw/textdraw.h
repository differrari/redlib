#pragma once

#include "types.h"
#include "graphic_types.h"
#include "string/slice.h"
#include "data/struct/stack.h"
#include "fonts/font.h"
#include "rects.h"

#define CHAR_SIZE 8

typedef enum { wrap_none, wrap_word, wrap_word_preserve_indent } wrap_policy;

typedef struct {
    path_render_options glyph;
    font_hdr *font;
    bool crop;
    float line_spacing;
    float character_spacing;
} simple_text_format;

typedef struct {
    u32 scale;
    color foreground;
    color background;
    range_t bounds;
    wrap_policy wrap;
} text_format;

typedef enum { fmt_array_none, fmt_array_static, fmt_array_stack } fmt_array_type;

typedef struct {
    fmt_array_type array_type;
    void *fmt;
    size_t count;
} text_format_arr;

static inline text_format_arr stack_to_text_format(arr_stack_t *s){
    return (text_format_arr){ .array_type = fmt_array_stack, .fmt = s, .count = stack_count(s) }; 
}

static inline text_format_arr array_to_text_format(text_format *ptr, size_t count){
    return (text_format_arr){ .array_type = fmt_array_static, .fmt = ptr, .count = count }; 
}

u32 fb_get_line_spacing(int scale);
u32 fb_char_width(u32 scale);
u32 fb_line_height(u32 scale);

typedef enum { draw_text_render, draw_text_delete, draw_text_rerender } draw_text_op;

typedef struct {
    gpu_size size;
    gpu_point cursor;
} text_draw_result;

void fb_continuous_draw_text(draw_ctx *ctx, draw_text_op operation, gpu_point *cursor, string_slice slice, range_t *render_range, gpu_rect bounds, gpu_size *out_size, gpu_point scroll, text_format default_format, text_format_arr array);
text_draw_result fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format format);
text_draw_result fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format default_format, text_format_arr array);

u32 lin_col_to_pos(i32 line, i32 col, string_slice content);
void pos_to_lin_col(u32 pos, string_slice content, i32 *lin, i32 *col);

gpu_size fb_write_slice(draw_ctx *ctx, string_slice slice, gpu_point loc, simple_text_format options);

void fb_draw_raw_char(draw_ctx *ctx, uint32_t x, uint32_t y, char c, uint32_t scale, uint32_t color);
static inline void fb_draw_char(draw_ctx *ctx, uint32_t x, uint32_t y, char c, uint32_t scale, uint32_t color){
    fb_draw_raw_char(ctx, x, y, c, scale, color);
    mark_dirty(ctx, x,y,CHAR_SIZE*scale,CHAR_SIZE*scale);
}

static inline gpu_size fb_draw_slice(draw_ctx *ctx, string_slice slice, uint32_t x0, uint32_t y0, uint32_t scale, uint32_t color){
    return fb_write_slice(ctx, slice, (gpu_point){(i32)x0, (i32)y0}, (simple_text_format){
        .glyph = {
            .scale = scale,
            .background_color = 0,
            .outline_color = 0,
            .color = color,
            .type = path_render_fill,
        },
        .font = 0,
        .crop = 0,
        .line_spacing = 0,
        .character_spacing = 0
    });
}

static inline gpu_size fb_draw_string(draw_ctx *ctx, const char* s, uint32_t x0, uint32_t y0, uint32_t scale, uint32_t color){
    return fb_draw_slice(ctx, slice_from_literal(s), x0, y0, scale, color);
}