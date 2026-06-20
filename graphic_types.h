#pragma once

#include "types.h"
#include "alloc/allocate.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t color;

typedef union argbcolor {
    struct {
        uint32_t blue: 8;
        uint32_t green: 8;
        uint32_t red: 8;
        uint32_t alpha: 8;
    };
    uint32_t color;
} argbcolor;

typedef struct {
    i32 x;
    i32 y;
} gpu_point;

typedef struct {
    i32 x;
    i32 y;
} int_point;

typedef struct {
    u32 width;
    u32 height;
} gpu_size;

typedef struct {
    gpu_point point;
    gpu_size size;
} gpu_rect;

#define MAX_DIRTY_RECTS 64

typedef struct draw_ctx {
    gpu_rect dirty_rects[MAX_DIRTY_RECTS];
    uint32_t* fb;
    uint32_t stride;
    uint32_t width;
    uint32_t height;
    uint32_t dirty_count;
    bool full_redraw;
} draw_ctx;

static inline draw_ctx buffer_to_draw_ctx(void *buf, i32 width, i32 height){
    return (draw_ctx){
        .dirty_rects = {},
        .fb = (u32*)buf,
        .stride = (u32)width * (u32)sizeof(color),
        .width = (u32)width,
        .height = (u32)height,
        .dirty_count = 0,
        .full_redraw = 0,
    };
}

static inline draw_ctx dummy_draw_ctx(i32 width, i32 height){
    return buffer_to_draw_ctx(zalloc(width * height * sizeof(color)), width, height);
}


#ifdef __cplusplus
}
#endif