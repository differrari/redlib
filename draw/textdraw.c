#include "textdraw.h"
#include "math/math.h"
#include "draw.h"
#include "fonts/font.h"
#include "font8x8_basic.h"

static inline text_format* get_fmt_at(text_format_arr array, size_t index){
    if (!array.fmt) return 0;
    switch (array.array_type){
        case fmt_array_stack:
            return stack_get(array.fmt, index);
        case fmt_array_static:
            return &((text_format*)array.fmt)[index];
        case fmt_array_none: return 0;
    }
    return 0;
}

static inline text_format get_current_format(uptr cpos, text_format default_format, text_format_arr array){
    if (array.array_type == fmt_array_stack){
        array.count = stack_count(array.fmt);
    }
    for (size_t i = 0; i < array.count; i++){
        text_format *current = get_fmt_at(array,i);
        if (!current) continue;
        if (current->bounds.start <= cpos && current->bounds.start + current->bounds.size >= cpos){
            if (current->background) default_format.background = current->background;
            if (current->foreground) default_format.foreground = current->foreground;
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

//TODO: use font
void fb_continuous_draw_text(draw_ctx *ctx, draw_text_op operation, gpu_point *cursor, string_slice slice, range_t *render_range, gpu_rect bounds, gpu_size *out_size, gpu_point scroll, text_format default_format, text_format_arr array){
    int indent = 0;
    bool can_indent = true;
    u32 char_width = 0, line_height = 0;
    size_t current_lookahead = 0;
    bounds.point.x += scroll.x;
    bounds.point.y += scroll.y;
    if (operation != draw_text_delete){
        if (render_range->start > slice.length-1|| render_range->size > slice.length-render_range->start) return;
    }
    for (size_t i = render_range->start; i < render_range->start + render_range->size; i++){
        text_format current_format = get_current_format(i, default_format, array);
        char c = {};
        if (operation != draw_text_delete || (render_range->start < slice.length-1 && render_range->size < slice.length-render_range->start)) c = slice.data[i];
        size_t curr_char_width = fb_char_width(current_format.scale);
        size_t curr_line_height = fb_line_height(current_format.scale);
        if (char_width < curr_char_width) char_width = curr_char_width;
        if (line_height < curr_line_height) line_height = curr_line_height;
        wrap_policy current_wrap = default_format.wrap;
        if (c == '\n' || c == '\r' || is_whitespace(c)){
            current_lookahead = 0;
        }
        if (c == '\t'){
            cursor->x += 4 * curr_char_width;
        }
        if (c == '\n'){
            new_line(cursor, line_height, 0);
            if (cursor->y + line_height > out_size->height) out_size->height = cursor->y + line_height;
            char_width = 0;
            line_height = 0;
            indent = 0;
            can_indent = true;
        } else if (can_indent && is_whitespace(c)){
            indent++;
        } else if (operation != draw_text_delete) {
            can_indent = false;
        }
        
        if (operation != draw_text_delete && (current_wrap == wrap_word || current_wrap == wrap_word_preserve_indent) && !is_whitespace(c)){
            if (current_lookahead){
                current_lookahead--;
            } else {
                size_t lookahead = i;
                for (; lookahead < slice.length; lookahead++) if (is_whitespace(slice.data[lookahead])) break;
                size_t word_size = lookahead-i;
                current_lookahead = word_size-1;
                if ((word_size * char_width) + cursor->x > bounds.size.width){
                    new_line(cursor, line_height, current_wrap == wrap_word_preserve_indent ? indent * char_width : 0);
                    if (cursor->y + line_height > out_size->height) out_size->height = cursor->y + line_height;
                }
            }
        }
        
        if (c != '\n' && cursor->x + scroll.x < (i32)bounds.size.width && 
            cursor->y + scroll.y < (i32)bounds.size.height){
            if (ctx->fb){
                if (operation == draw_text_delete) {
                    cursor->x -= curr_char_width;
                    //TODO: handle moving to previous line with pos_to_lin_col
                }
                if (current_format.background) fb_fill_rect(ctx, cursor->x + bounds.point.x, cursor->y + bounds.point.y, char_width, line_height, current_format.background);
                if (c) fb_draw_raw_char(ctx, cursor->x + bounds.point.x, cursor->y + bounds.point.y, c, current_format.scale, current_format.foreground);
            }
            if (c && operation == draw_text_render) cursor->x += curr_char_width;
            if (cursor->x > (i32)out_size->width) out_size->width = cursor->x;
            if (!out_size->height) out_size->height = line_height;
        }
    }
}

text_draw_result fb_draw_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format default_format, text_format_arr array){
    text_draw_result result = {};

    range_t string_range = {.start = 0, .size = slice.length};
    
    fb_continuous_draw_text(ctx, false, &result.cursor, slice, &string_range, bounds, &result.size, scroll, default_format, array);
    if (ctx->fb) mark_dirty(ctx, bounds.point.x, bounds.point.y, result.size.width, result.size.height);
    
    return result;
}

text_draw_result fb_draw_single_text(draw_ctx *ctx, string_slice slice, gpu_rect bounds, gpu_point scroll, text_format format){
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
    if (!lin || !col) return;
    *lin = 0;
    *col = 0;
    for (i32 i = 0; i < min(pos,content.length); i++){
        if (content.data[i] == '\n'){
            *col = 0;
            *lin = (*lin) + 1;
        } else *col = (*col) + 1;
    }
}


void fb_draw_raw_char(draw_ctx *ctx, uint32_t x, uint32_t y, char c, uint32_t scale, uint32_t color){
    const uint8_t* glyph = font8x8_basic[(u8)c];
    const uint32_t char_size = CHAR_SIZE * scale;
    if (x >= ctx->width || y >= ctx->height) return;

    uint32_t max_w = ctx->width - x;
    uint32_t max_h = ctx->height - y;
    uint32_t draw_w = char_size <= max_w ? char_size : max_w;
    uint32_t draw_h = char_size <= max_h ? char_size : max_h;

    const uint32_t row_pitch = ctx->stride >> 2;

    for (uint32_t gy = 0; gy < 8; ++gy) {
        uint32_t base_y = gy * scale;
        if (base_y >= draw_h) break;
        uint32_t ry_lim = scale;
        if (base_y + ry_lim > draw_h) ry_lim = draw_h - base_y;
        uint8_t bits = glyph[gy];
        for (uint32_t ry = 0; ry < ry_lim; ++ry) {
            uint32_t* dst = ctx->fb + (y + base_y + ry) * row_pitch + x;
            for (uint32_t gx = 0; gx < 8; ++gx) {
                uint32_t base_x = gx * scale;
                if (base_x >= draw_w) break;
                if (bits & (1u << (7 - gx))) {
                    uint32_t rx_lim = scale;
                    if (base_x + rx_lim > draw_w) rx_lim = draw_w - base_x;
                    uint32_t* p = dst + base_x;
                    for (uint32_t rx = 0; rx < rx_lim; ++rx) { 
                        if (((color >> 24) & 0xFF) < 0xFF)
                            p[rx] = pixel_blend(color, p[rx]);
                        else
                            p[rx] = color;
                    }
                }
            }
        }
    }
}

gpu_size fb_write_slice(draw_ctx *ctx, string_slice slice, gpu_point loc, simple_text_format options){
    uint32_t xoff = 0;
    uint32_t xSize = 0;
    uint32_t xRowSize = 0;
    uint32_t ySize = 0;
    u32 y_line_size = 0;

    const uint32_t start_y = loc.y;
    uint32_t rows = 1;

    for (size_t i = 0; i < slice.length; ++i){    
        char c = slice.data[i];
        if (c == '\n'){
            if (xRowSize > xSize)
                xSize = xRowSize;
            xRowSize = 0;
            xoff = 0;
            loc.y += y_line_size;
            rows++;
            if (options.crop && loc.y >= (i32)ctx->height) break;
        } else {
            gpu_size size = font_render_glyph_character(options.font, options.glyph, ctx, (gpu_point){loc.x + xoff,loc.y }, c);
            xoff += (size.width * (1 + options.character_spacing));
            y_line_size = (size.height * (1 + options.line_spacing));
            xRowSize += (size.width * (1 + options.character_spacing));
        }
    }
    if (xRowSize > xSize)
        xSize = xRowSize;

    ySize = rows * y_line_size;

    uint32_t bbox_w = xSize;
    uint32_t bbox_h = ySize;

    if (loc.x + bbox_w > ctx->width) bbox_w = ctx->width - loc.x;
    if (start_y + bbox_h > ctx->height) bbox_h = ctx->height - start_y;

    if (bbox_w && bbox_h) mark_dirty(ctx, loc.x,start_y,bbox_w,bbox_h);

    return (gpu_size){bbox_w,bbox_h};
}