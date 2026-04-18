#include "textdraw.h"
#include "math/math.h"
#include "draw.h"
#include "syscalls/syscalls.h"

static inline text_format get_current_format(uptr cpos, text_format default_format, text_format *formats, size_t formats_count){
    for (size_t i = 0; i < formats_count; i++){
        text_format current = formats[i];
        if (current.bounds.start <= cpos && current.bounds.start + current.bounds.size >= cpos){
            if (current.color) default_format.color = current.color;
            if (current.scale) default_format.scale = current.scale;
        }
    }
    return default_format;
}

static inline void new_line(gpu_point *point, u32 line_size, int indent){
    point->x = indent;
    point->y += line_size;
}

gpu_size fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format default_format, text_format *formats, size_t formats_count){
    gpu_point cursor = { .x = bounds.point.x, .y = bounds.point.y };
    int indent = 0;
    bool can_indent = true;
    for (size_t i = 0; i < slice.length; i++){
        text_format current_format = get_current_format(i, default_format, formats, formats_count);
        char c = slice.data[i];
        u32 size = fb_get_char_size(current_format.scale);
        wrap_policy current_wrap = default_format.wrap;
        if (c == '\n'){
            new_line(&cursor, size, 0);
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
            if ((word_size * size) + cursor.x - bounds.point.x >= bounds.size.width){
                new_line(&cursor, size, current_wrap == wrap_word_preserve_indent ? indent * size : 0);
            }
        }
        
        if (c != '\n' && cursor.x < (i32)bounds.size.width - bounds.point.x && 
            cursor.y < (i32)bounds.size.height - bounds.point.y){
            fb_draw_raw_char(ctx, cursor.x, cursor.y, c, current_format.scale, current_format.color);
            cursor.x += size;
        }
    }
    
    mark_dirty(ctx, bounds.point.x, bounds.point.y, bounds.size.width, bounds.size.height);
    
    return (gpu_size){0,0};
}

gpu_size fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, text_format format){
    return fb_draw_text(ctx, slice, bounds, format, 0, 0);
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