#ifdef CROSS

#include "std/string.h"
#include "ui/draw/draw.h"
#include "raylib.h"
#include "syscalls/syscalls.h"

#define CONVERT_COLOR(color) GetColor(((color & 0xFFFFFF) << 8) | ((color >> 24) & 0xFF))

void begin_drawing(draw_ctx *ctx){
    BeginDrawing();
}

void destroy_draw_ctx(draw_ctx *ctx){
    CloseWindow();
}

void commit_draw_ctx(draw_ctx *ctx){
    EndDrawing();
}

void resize_draw_ctx(draw_ctx *ctx, uint32_t width, uint32_t height){
    SetWindowSize(width, height);
    ctx->width = width;
    ctx->height = height;
}

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ? ctx->width : 600;
    uint32_t h = ctx->height ? ctx->height : 300;
    InitWindow(w, h, "RedXLib");
    SetExitKey(0);
    ctx->width = w;
    ctx->height = h;
    ctx->stride = 4 * w;
}

bool should_close_ctx(){
    return WindowShouldClose();
}

void fb_clear(draw_ctx *ctx, uint32_t color) {
    ClearBackground(CONVERT_COLOR(color));
}

void fb_draw_raw_pixel(draw_ctx *ctx, uint32_t x, uint32_t y, color color){

}

void fb_draw_pixel(draw_ctx *ctx, uint32_t x, uint32_t y, color color){
    fb_draw_raw_pixel(ctx,x,y,color);
}

void fb_fill_rect(draw_ctx *ctx, int32_t x, int32_t y, uint32_t width, uint32_t height, color color){
    DrawRectangle(x, y, width, height, CONVERT_COLOR(color));
}

void fb_draw_img(draw_ctx *ctx, uint32_t x, uint32_t y, uint32_t *img, uint32_t img_width, uint32_t img_height){
    fb_draw_partial_img(ctx, img, x, y, img_width, img_height, (image_transform){});
}

void fb_draw_partial_img(draw_ctx *ctx, uint32_t *img, uint32_t x, uint32_t y, uint32_t full_width, uint32_t full_height, image_transform transform){
    Texture2D *tex = (Texture2D*)img;
    DrawTexture(*tex, x, y, WHITE);
}

gpu_rect fb_draw_line(draw_ctx *ctx, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, color color){
    DrawLine(x0, y0, x1, y1, CONVERT_COLOR(color));
    return (gpu_rect){{0,0},{0,0}};
}

void fb_draw_raw_char(draw_ctx *ctx, uint32_t x, uint32_t y, char c, uint32_t scale, uint32_t color){
    string s = string_from_char(c);
    DrawText(s.data, x, y, fb_get_char_size(scale), CONVERT_COLOR(color));
    string_free(s);
}

void fb_draw_char(draw_ctx *ctx, uint32_t x, uint32_t y, char c, uint32_t scale, uint32_t color){
    fb_draw_raw_char(ctx, x, y, c, scale, color);
}

gpu_size fb_draw_string(draw_ctx *ctx, const char* s, uint32_t x0, uint32_t y0, uint32_t scale, uint32_t color){
    DrawText(s, x0, y0, fb_get_char_size(scale), CONVERT_COLOR(color));
    return (gpu_size){0,0};
}

uint32_t fb_get_char_size(uint32_t scale){
    return CHAR_SIZE * scale;
}

#endif