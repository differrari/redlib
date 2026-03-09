#pragma once

#include "types.h"

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

#ifdef __cplusplus
}
#endif