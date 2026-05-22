#include "console.hpp"
#include "memory/memory.h"
#include "syscalls/syscalls.h"
#include "math/math.h"

#define char_width (char_scale*CHAR_SIZE)
#define line_height (char_scale*CHAR_SIZE*2)

static inline u32 line_len(const char* line, u32 max_cols){
    u32 n = 0;
    while (n < max_cols && line[n]) n++;
    return n;
}

Console::Console() : cursor_x(0), cursor_y(0), is_initialized(false){
    last_drawn_cursor_x = -1;
    last_drawn_cursor_y = -1;
    columns = 0;
    rows = 0;
    scroll_row_offset = 0;
    row_data = nullptr;
    gap_start = 0;
    gap_end = 0;
    buffer_data_size = 0;
    dctx = nullptr;
}

void Console::initialize(){
    is_initialized = true;
    dctx = get_ctx();
    resize();
    screen_clear();
}

bool Console::check_ready(){
    if (!screen_ready()) return false;
    if (!is_initialized){
        initialize();
    }
    return true;
}

void Console::resize(){
    if (!dctx){
        rows = columns = 0;
        return;
    }

    gpu_size screen_size = { dctx->width, dctx->height };
    u32 new_columns = screen_size.width / char_width;
    u32 new_rows = screen_size.height / line_height;

    if (new_rows == 0 || new_columns == 0) { rows = columns = 0; return; }

    char* old_data = row_data;
    color* old_bg_data = row_bg_data;
    color* old_fg_data = row_fg_data;
    u32 old_rows = rows;
    u32 old_cols = columns;
    u32 old_offset = scroll_row_offset;

    u32 new_buf_size = new_rows * new_columns;
    char* new_data = (char*)zalloc(new_buf_size);
    color* new_bg_data = (color*)zalloc(new_buf_size * sizeof(color));
    color* new_fg_data = (color*)zalloc(new_buf_size * sizeof(color));
    if (!new_data) return;

    if (old_data && old_rows && old_cols) {
        u32 copy_rows = min(old_rows, new_rows);

        for (u32 y = 0; y < copy_rows; y++) {
            const char* src = old_data + (((old_offset + y) % old_rows) * old_cols);
            char* dst = new_data + (y * new_columns);
            u32 len = line_len(src, old_cols);
            u32 w = min(len, new_columns);
            if (w){
                memcpy(dst, src, w);
                memcpy(new_bg_data, old_bg_data, w * sizeof(color));
                memcpy(new_fg_data, old_fg_data, w * sizeof(color));
            }
            if (w < new_columns) dst[w] = 0;
        }
    }

    if (old_data) release(old_data);

    row_data = new_data;
    row_bg_data = new_bg_data;
    row_fg_data = new_fg_data;
    rows = new_rows;
    columns = new_columns;
    buffer_data_size = new_buf_size;
    scroll_row_offset = 0;

    if ((i32)cursor_y < 0) cursor_y = 0;
    if ((i32)cursor_x < 0) cursor_x = 0;
    if (cursor_y >= rows) cursor_y = rows ? rows - 1 : 0;
    if (cursor_x >= columns) cursor_x = columns ? columns - 1 : 0;

    last_drawn_cursor_x = -1;
    last_drawn_cursor_y = -1;
}

void Console::render_glyph(i32 x, i32 y, char c, color foreground, color background, bool half){
    if (!(background & 0xFF000000)) background = bg_color;
    fb_fill_rect(dctx, x, y, char_width, line_height, background);
    if (c) fb_draw_char(dctx, x, y + (half ? (line_height/2) : 0), c, char_scale, foreground);
}

void Console::put_char(char c){
    if (!check_ready()) return;
    last_char = c;

    bool render = true;
    if (embedded_fmt_parse(&current_format, c)) render = false;
    if (current_format.wipe){
        current_format.wipe = false;
        clear();
    }
    if (!render) return;
    if (c == '\r'){
        cursor_x = 0;
        draw_cursor();
        return;
    }
    if (c == '\t'){
        cursor_x += 4;
        if (cursor_x >= columns) newline();
        return;
    }
    if (c == '\n'){
        newline(); 
        draw_cursor();
        flush(dctx);
        return;
    }
    if (cursor_x >= columns) newline();

    char* line = row_data + (((scroll_row_offset + cursor_y) % rows) * columns);
    line[cursor_x] = c;
    color* line_bg = &row_bg_data[((scroll_row_offset + cursor_y) % rows) * columns];
    line_bg[cursor_x] = bg_color;
    color* line_fg = &row_fg_data[((scroll_row_offset + cursor_y) % rows) * columns];
    line_fg[cursor_x] = current_format.current_text_color;
    if (cursor_x + 1 < columns) line[cursor_x + 1] = 0;

    render_glyph(cursor_x * char_width, (cursor_y * line_height)+(line_height/2), c, current_format.current_text_color, bg_color);
    cursor_x++;
    draw_cursor();
}

//TODO: generalize this function to handle movement in general
void Console::delete_last_char(){
    if (!check_ready()) return;

    if (cursor_x > 0){
        cursor_x--;
    } else if (cursor_y > 0){
        cursor_y--;
        char* line0 = row_data + (((scroll_row_offset + cursor_y) % rows) * columns);
        cursor_x = line_len(line0, columns);
        draw_cursor();
        flush(dctx);
        return;
    } else return;

    char* line = &row_data[((scroll_row_offset + cursor_y) % rows) * columns];
    line[cursor_x] = 0;
    color* line_bg = &row_bg_data[((scroll_row_offset + cursor_y) % rows) * columns];
    line_bg[cursor_x] = default_bg_color;
    color* line_fg = &row_fg_data[((scroll_row_offset + cursor_y) % rows) * columns];
    line_fg[cursor_x] = current_format.default_text_color;
    fb_fill_rect(dctx, cursor_x*char_width, cursor_y * line_height, char_width, line_height, default_bg_color);
    draw_cursor();
    flush(dctx);
}

void Console::draw_cursor(){
    if (last_drawn_cursor_x >= 0 && last_drawn_cursor_y >= 0){
        if ((u32)last_drawn_cursor_x < columns && (u32)last_drawn_cursor_y < rows){
            char *prev_line = &row_data[((scroll_row_offset + (u32)last_drawn_cursor_y) % rows) * columns];
            color *prev_line_fg = &row_fg_data[((scroll_row_offset + (u32)last_drawn_cursor_y) % rows) * columns];
            color *prev_line_bg = &row_bg_data[((scroll_row_offset + (u32)last_drawn_cursor_y) % rows) * columns];
            render_glyph(last_drawn_cursor_x*char_width, last_drawn_cursor_y * line_height, prev_line[last_drawn_cursor_x], prev_line_fg[last_drawn_cursor_x], prev_line_bg[last_drawn_cursor_x], true);
        }
    }
    fb_fill_rect(dctx, cursor_x*char_width, cursor_y * line_height, char_width, line_height, current_format.current_text_color);
    last_drawn_cursor_x = (i32)cursor_x;
    last_drawn_cursor_y = (i32)cursor_y;
}

void Console::put_slice(string_slice slice){
    if (!check_ready()) return;
    for (u32 i = 0; i < slice.length; i++) put_char(slice.data[i]);
    flush(dctx);
}

void Console::put_string(const char* str){
    put_slice(slice_from_literal(str));
}

void Console::newline(){
    if (!check_ready()) return;
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= rows - 1){
        scroll();
        cursor_y = rows - 1;
    } else {
        char* line = row_data + (((scroll_row_offset + cursor_y) % rows) * columns);
        memset(line, 0, columns);
    }
}

void Console::scroll(){
    if (!check_ready()) return;

    scroll_row_offset = (scroll_row_offset + 1) % rows;
    u32 clear_index = (scroll_row_offset + rows - 1) % rows;
    char* line = row_data + clear_index * columns;
    memset(line, 0, columns);

    fb_clear(dctx, bg_color);
    for (u32 y = 0; y < rows; y++) {
        char* l = &row_data[((scroll_row_offset + y) % rows) * columns];
        color *line_fg = &row_fg_data[((scroll_row_offset + y) % rows) * columns];
        color *line_bg = &row_bg_data[((scroll_row_offset + y) % rows) * columns];
        u32 len = line_len(l, columns);
        if (!len) continue;
        u32 ypix = (y * line_height) + (line_height / 2);
        for (u32 x = 0; x < len; x++) {
            render_glyph(x * char_width, ypix, l[x], line_fg[x], line_bg[x]);
        }
    }
    draw_cursor();
}

void Console::refresh(){
    if (!check_ready()) return;
    resize();
    screen_clear();
    redraw();
    flush(dctx);
}

void Console::redraw(){
    if (!check_ready()) return;

    for (u32 y = 0; y < rows; y++){
        char* l = &row_data[((scroll_row_offset + y) % rows) * columns];
        color *line_fg = &row_fg_data[((scroll_row_offset + y) % rows) * columns];
        color *line_bg = &row_bg_data[((scroll_row_offset + y) % rows) * columns];
        u32 len = line_len(l, columns);
        if (!len) continue;
        u32 ypix = (y * line_height) + (line_height / 2);
        for (u32 x = 0; x < len; x++){
            render_glyph(x * char_width, ypix, l[x], line_fg[x], line_bg[x]);
        }
    }
    draw_cursor();
}

void Console::screen_clear(){
    if (dctx) fb_clear(dctx, bg_color);
    last_drawn_cursor_x = -1;
    last_drawn_cursor_y = -1;
}

void Console::clear(){
    screen_clear();
    if (row_data && buffer_data_size) memset(row_data, 0, buffer_data_size);
    cursor_x = cursor_y = 0;
}

const char* Console::get_current_line(){
    return row_data + (((scroll_row_offset + cursor_y) % rows) * columns);
}