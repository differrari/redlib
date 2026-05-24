#pragma once

#include "types.h"
#include "draw/draw.h"
#include "embedded_fmt/embedded_fmt.h"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0

#define char_width (char_scale*CHAR_SIZE)
#define line_height (char_scale*CHAR_SIZE*2)

class Console{
public:
    Console();

    void initialize();

    void put_char(char c);
    void put_string(const char* str);
    void put_slice(string_slice slice);

    char last_char;

    void newline();
    void scroll();
    void clear();
    void resize();

    void refresh();
    
    void set_active(bool active);
    void delete_last_char();

    void render_glyph(i32 x, i32 y, char c, color foreground, color background, bool half = false);
    
protected:
    bool check_ready();
    void screen_clear();
    void redraw();
    void draw_cursor();
    const char* get_current_line();

    virtual draw_ctx* get_ctx() = 0;
    virtual void flush(draw_ctx *ctx) = 0;
    virtual bool screen_ready() = 0;

    i32 last_drawn_cursor_x, last_drawn_cursor_y;
    u32 columns, rows;
    bool is_initialized;
    
    static constexpr u32 max_rows=128;
    
    i32 scroll_row_offset = 0;
    char* row_data;
    color* row_bg_data;
    color* row_fg_data;
    i32 gap_start, gap_end;
    i32 buffer_data_size;

    draw_ctx *dctx;

    bool active = true;

    u8 char_scale = 1;

    __attribute__((aligned(16))) embedded_fmt current_format = {
        .current_text_color = COLOR_WHITE,
        .default_text_color = COLOR_WHITE,
        .current_bg_color = COLOR_BLACK,
        .default_bg_color = COLOR_BLACK,
        .cursor_x = 0,
        .cursor_y = 0,
        .wipe = false,
        .state_type = 0,
        .state = 0,
    };
};
