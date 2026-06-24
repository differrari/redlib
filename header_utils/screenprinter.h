#include "types.h"
#include "graphic_types.h"
#include "draw/textdraw.h"
#include "draw/draw.h"
#include "files/buffer.h"
#include "utils/embedded_fmt/embedded_fmt.h"

static draw_ctx printer_ctx = {};
static buffer screen_printer_buffer = {};

static embedded_fmt screen_printer_formatting = {};

static u32 char_width;
static u32 line_height;
static u32 char_height;

static gpu_point printer_scroll = {};

static range_t printer_rerender_range = { };

draw_text_op printer_operation = draw_text_render;

#define TEXT_SCALE 3

text_format embedded_fmt_to_text(embedded_fmt fmt){
    return (text_format){ .background = fmt.current_bg_color, .foreground = fmt.current_text_color, .scale = TEXT_SCALE, .wrap = wrap_word };
}

void screen_printer_init(draw_ctx ctx){
    printer_ctx = ctx;
    screen_printer_buffer = buffer_create(0x1000, buffer_can_grow);
    
    char_width = fb_char_width(TEXT_SCALE);
    char_height = fb_char_width(TEXT_SCALE);
    line_height = fb_line_height(TEXT_SCALE);
}

void screen_printer_clear(){
    fb_clear(&printer_ctx, screen_printer_formatting.current_bg_color);
}

void screen_print_flush(bool can_scroll);

void screen_print_scroll(int amount, bool can_rescroll){
    printer_scroll.y += amount * line_height;
    if (printer_scroll.y > 0) {
        printer_scroll.y = 0;
        return;
    }
    screen_printer_formatting.cursor_x = 0;
    screen_printer_formatting.cursor_y = 0;
    screen_printer_clear();
    printer_rerender_range.start = 0;
    printer_rerender_range.size = screen_printer_buffer.buffer_size;
    screen_print_flush(can_rescroll);
}

void screen_print_flush(bool can_scroll){

    gpu_rect rect = (gpu_rect){ 
        { 
            printer_scroll.x,
            printer_scroll.y,
        }, { 
            printer_ctx.width,
            printer_ctx.height
        }
    };
    
    gpu_point cursor = {.x = screen_printer_formatting.cursor_x*char_width,.y = screen_printer_formatting.cursor_y*line_height};
    gpu_size out_size = {};
    fb_continuous_draw_text(&printer_ctx, printer_operation, &cursor, slice_from_buffer(&screen_printer_buffer), &printer_rerender_range, rect, &out_size, printer_scroll, embedded_fmt_to_text(screen_printer_formatting), (text_format_arr){});
    
    if (cursor.y + printer_scroll.y > (i32)rect.size.height && can_scroll)
        screen_print_scroll(-line_height, true);
    
    screen_printer_formatting.cursor_x = cursor.x/char_width;
    screen_printer_formatting.cursor_y = cursor.y/line_height;
    
    printer_rerender_range = (range_t){ .start = screen_printer_buffer.cursor };
    //TODO: we might be able to auto-calculate the rerender range by storing the last cursor we saved and comparing it to the current one. Maybe not for scroll though
    printer_operation = draw_text_render;
}

void screen_printer_append(char *fmt, ...){
    __attribute__((aligned(16))) va_list args;
    va_start(args, fmt); 
    printer_rerender_range = (range_t){ screen_printer_buffer.cursor, 0 };
    size_t size = buffer_write_va(&screen_printer_buffer, fmt, args);
    va_end(args);
    printer_rerender_range.size = size;
    screen_print_flush(true);
}

void screen_printer_put_char(char c){
    printer_rerender_range.size += buffer_write_lim(&screen_printer_buffer, &c, 1);
}