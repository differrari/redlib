#pragma once

#include "types.h"
#include "graphic_types.h"
#include "string/slice.h"
#include "data/struct/stack.h"

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

typedef enum { fmt_array_none, fmt_array_static, fmt_array_stack } fmt_array_type;

typedef struct {
    fmt_array_type array_type;
    void *fmt;
    size_t count;
} text_format_arr;

static inline text_format_arr stack_to_text_format(stack_t *s){
    return (text_format_arr){ .array_type = fmt_array_stack, .fmt = s, .count = stack_count(s) }; 
}

static inline text_format_arr array_to_text_format(text_format *ptr, size_t count){
    return (text_format_arr){ .array_type = fmt_array_static, .fmt = ptr, .count = count }; 
}

gpu_size fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format format);
gpu_size fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format default_format, text_format_arr array);

u32 lin_col_to_pos(i32 line, i32 col, string_slice content);
void pos_to_lin_col(u32 pos, string_slice content, i32 *lin, i32 *col);