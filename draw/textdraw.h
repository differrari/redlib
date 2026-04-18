#pragma once

#include "types.h"
#include "graphic_types.h"
#include "string/slice.h"

typedef struct {
    uptr start;
    size_t size;
} range;

typedef enum { wrap_none, wrap_word, wrap_word_preserve_indent } wrap_policy;

typedef struct {
    u32 scale;
    color color;
    range bounds;
    wrap_policy wrap;
} text_format;

gpu_size fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format format);
gpu_size fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format default_format, text_format *formats, size_t formats_count);

u32 lin_col_to_pos(i32 line, i32 col, string_slice content);
void pos_to_lin_col(u32 pos, string_slice content, i32 *lin, i32 *col);