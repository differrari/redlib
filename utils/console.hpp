#pragma once

#include "types.h"
#include "draw/draw.h"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0

class Console{
public:
    Console();

    void initialize();

    void put_char(char c);
    void put_string(const char* str);

    void newline();
    void scroll();
    void clear();
    void resize();

    void refresh();
    
    void set_active(bool active);
    void delete_last_char();
    
protected:
    bool check_ready();
    void screen_clear();
    void redraw();
    void draw_cursor();
    const char* get_current_line();

    virtual draw_ctx* get_ctx() = 0;
    virtual void flush(draw_ctx *ctx) = 0;
    virtual bool screen_ready() = 0;

    void set_text_color(u32 color);

    u32 cursor_x, cursor_y;
    i32 last_drawn_cursor_x, last_drawn_cursor_y;
    u32 columns, rows;
    bool is_initialized;
    
    static constexpr u32 max_rows=128;

    u32 default_bg_color = COLOR_BLACK;
    u32 default_text_color = COLOR_WHITE;
    u32 bg_color = default_bg_color;
    u32 text_color = default_text_color;
    
    i32 scroll_row_offset = 0;
    char* row_data;
    i32 gap_start, gap_end;
    i32 buffer_data_size;

    draw_ctx *dctx;

    bool active = true;

    u8 char_scale = 1;
};

