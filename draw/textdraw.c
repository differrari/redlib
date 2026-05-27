#include "textdraw.h"
#include "math/math.h"
#include "draw.h"
#include "syscalls/syscalls.h"

static inline text_format* get_fmt_at(text_format_arr array, size_t index){
    if (!array.fmt) return 0;
    switch (array.array_type){
        case fmt_array_stack:
            return stack_get(array.fmt, index);
        case fmt_array_static:
            return &((text_format*)array.fmt)[index];
        case fmt_array_none: return 0;
    }
}

static inline text_format get_current_format(uptr cpos, text_format default_format, text_format_arr array){
    if (array.array_type == fmt_array_stack){
        array.count = stack_count(array.fmt);
    }
    for (size_t i = 0; i < array.count; i++){
        text_format *current = get_fmt_at(array,i);
        if (!current) continue;
        if (current->bounds.start <= cpos && current->bounds.start + current->bounds.size >= cpos){
            if (current->color) default_format.color = current->color;
            if (current->scale) default_format.scale = current->scale;
        }
    }
    return default_format;
}

static inline void new_line(gpu_point *point, u32 line_size, int indent){
    point->x = indent;
    point->y += line_size;
}

u32 fb_get_line_spacing(int scale){
    return 2 * scale;
}

u32 fb_char_width(u32 scale){
    return fb_get_char_size(scale);
}

u32 fb_line_height(u32 scale){
    return fb_get_char_size(scale) + fb_get_line_spacing(scale);
}

gpu_size fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format default_format, text_format_arr array){
    gpu_point cursor = { .x = 0, .y = 0 };
    int indent = 0;
    bool can_indent = true;
    u32 char_width = 0, line_height = 0;
    size_t current_lookahead = 0;
    gpu_size max_size = {};
    for (size_t i = 0; i < slice.length; i++){
        text_format current_format = get_current_format(i, default_format, array);
        char c = slice.data[i];
        size_t curr_char_width = fb_char_width(current_format.scale);
        size_t curr_line_height = fb_line_height(current_format.scale);
        if (char_width < curr_char_width) char_width = curr_char_width;
        if (line_height < curr_line_height) line_height = curr_line_height;
        wrap_policy current_wrap = default_format.wrap;
        if (c == '\n' || c == '\r' || is_whitespace(c)){
            current_lookahead = 0;
        }
        if (c == '\n'){
            new_line(&cursor, line_height, 0);
            if (cursor.y > max_size.height) max_size.height = cursor.y + line_height;
            char_width = 0;
            line_height = 0;
            indent = 0;
            can_indent = true;
        } else if (c == '\r'){
            cursor.x = 0;
            can_indent = true;
        } else if (can_indent && is_whitespace(c)){
            indent++;
        } else {
            can_indent = false;
        }
        
        if ((current_wrap == wrap_word || current_wrap == wrap_word_preserve_indent) && !is_whitespace(c)){
            if (current_lookahead){
                current_lookahead--;
            } else {
                size_t lookahead = i;
                for (; lookahead < slice.length; lookahead++) if (is_whitespace(slice.data[lookahead])) break;
                size_t word_size = lookahead-i;
                current_lookahead = word_size-1;
                if ((word_size * char_width) + cursor.x > bounds.size.width){
                    new_line(&cursor, line_height, current_wrap == wrap_word_preserve_indent ? indent * char_width : 0);
                    if (cursor.y > max_size.height) max_size.height = cursor.y + line_height;
                }
            }
        }
        
        if (c != '\n' && cursor.x + scroll.x < (i32)bounds.size.width && 
            cursor.y + scroll.y < (i32)bounds.size.height){
            if (ctx->fb) fb_draw_raw_char(ctx, cursor.x + bounds.point.x, cursor.y + bounds.point.y, c, current_format.scale, current_format.color);
            cursor.x += curr_char_width;
            if (cursor.x > max_size.width) max_size.width = cursor.x;
            if (!max_size.height) max_size.height = line_height;
        }
    }
    
    if (ctx->fb) mark_dirty(ctx, bounds.point.x, bounds.point.y, max_size.width, max_size.height);
    
    return (gpu_size){max_size.width, max_size.height};
}

gpu_size fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format format){
    return fb_draw_text(ctx, slice, bounds, scroll, format, (text_format_arr){});
}

u32 lin_col_to_pos(i32 line, i32 col, string_slice content){
    i32 line_number = 0;
    i32 column = 0;
    u32 pos = 0;
    for (u32 i = 0; i < content.length; i++){
        pos = i;
        if (content.data[i] == '\n'){
            if (line_number == line) return i;
            column = 0;
            line_number++;
        } else {
            if (line_number == line && column == col) return i;
            column++;   
        }
    }
    return pos;
}

void pos_to_lin_col(u32 pos, string_slice content, i32 *lin, i32 *col){
    *lin = 0;
    *col = 0;
    for (i32 i = 0; i < min(pos,content.length); i++){
        if (content.data[i] == '\n'){
            *col = 0;
            *lin = (*lin) + 1;
        } else *col = (*col) + 1;
    }
}