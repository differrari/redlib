#ifdef CROSS

#include "string/string.h"
#include "ui/draw/draw.h"
#include "raylib.h"
#include "syscalls/syscalls.h"

#define CONVERT_COLOR(color) ((color & 0xFF00FF00) | ((color & 0xFF) << 16) | ((color >> 16) & 0xFF))

Texture2D _screen_tex;

void begin_drawing(draw_ctx *ctx){
    BeginDrawing();
}

void destroy_draw_ctx(draw_ctx *ctx){
    CloseWindow();
}

void commit_draw_ctx(draw_ctx *ctx){
    BeginDrawing();
    ClearBackground(GetColor(0));
    for (uint64_t i = 0; i < ctx->width * ctx->height; i++) ctx->fb[i] = CONVERT_COLOR(ctx->fb[i]);//TODO: sux
    UpdateTexture(_screen_tex, ctx->fb);
    DrawTexture(_screen_tex, 0, 0, WHITE);
    EndDrawing();
}

void resize_draw_ctx(draw_ctx *ctx, uint32_t width, uint32_t height){
    free_sized(ctx->fb, ctx->width*ctx->height*sizeof(color));
    UnloadTexture(_screen_tex);
    ctx->width = width;
    ctx->height = height;
    ctx->fb = zalloc(width*height*sizeof(color));
    ctx->stride = 4 * width;
    _screen_tex = LoadTextureFromImage((Image){
        .data = ctx->fb,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    });
    SetWindowSize(width, height);
}

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ? ctx->width : 600;
    uint32_t h = ctx->height ? ctx->height : 300;
    ctx->fb = zalloc(w*h*sizeof(color));
    ctx->width = w;
    ctx->height = h;
    ctx->stride = 4 * w;
    InitWindow(w, h, "RedXLib");
    _screen_tex = LoadTextureFromImage((Image){
        .data = ctx->fb,
        .width = w,
        .height = h,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    });
    SetExitKey(0);
}

bool should_close_ctx(){
    return WindowShouldClose();
}

#endif