#include "textdraw.h"
#include "math/math.h"
#include "draw.h"
#include "syscalls/syscalls.h"

static inline text_format* get_fmt_at(text_format_arr array, size_t index){
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

gpu_size fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format default_format, text_format_arr array){
    gpu_point cursor = { .x = bounds.point.x, .y = bounds.point.y };
    int indent = 0;
    bool can_indent = true;
    u32 char_width, line_height = 0;
    for (size_t i = 0; i < slice.length; i++){
        text_format current_format = get_current_format(i, default_format, array);
        char c = slice.data[i];
        size_t curr_char_width = fb_get_char_size(current_format.scale);
        size_t curr_line_height = fb_get_char_size(current_format.scale);
        if (char_width < curr_char_width) char_width = curr_char_width;
        if (line_height < curr_line_height) line_height = curr_line_height;
        wrap_policy current_wrap = default_format.wrap;
        if (c == '\n'){
            new_line(&cursor, line_height, 0);
            char_width = 0;
            line_height = 0;
            indent = 0;
            can_indent = true;
        } else if (c == '\r'){
            cursor.x = 0;
            can_indent = true;
        } else if (can_indent && (c == '\t' || c == ' ')){
            indent++;
        } else {
            can_indent = false;
        }
        
        if ((current_wrap == wrap_word || current_wrap == wrap_word_preserve_indent) && !is_whitespace(c)){
            size_t lookahead = i;
            for (; lookahead < slice.length; lookahead++){
                if (is_whitespace(slice.data[lookahead])) break;
            }
            size_t word_size = lookahead-i;
            if ((word_size * char_width) + cursor.x - bounds.point.x >= bounds.size.width){
                new_line(&cursor, line_height, current_wrap == wrap_word_preserve_indent ? indent * char_width : 0);
            }
        }
        
        if (c != '\n' && cursor.x < (i32)bounds.size.width - bounds.point.x && 
            cursor.y < (i32)bounds.size.height - bounds.point.y){
            fb_draw_raw_char(ctx, cursor.x, cursor.y, c, current_format.scale, current_format.color);
            cursor.x += curr_char_width;
        }
    }
    
    mark_dirty(ctx, bounds.point.x, bounds.point.y, bounds.size.width, bounds.size.height);
    
    return (gpu_size){0,0};
}

gpu_size fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format format){
    return fb_draw_text(ctx, slice, bounds, format, (text_format_arr){});
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