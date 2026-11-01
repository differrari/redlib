#pragma once

#include "graphic_types.h"
#include "math/math.h"

typedef struct {
    float alpha;
    float hue;
    float saturation;
    float value;
} ahsvcolor;

ahsvcolor argb_to_ahsv(argbcolor col);

extern int print(const char *fmt, ...);

argbcolor ahsv_to_argb(ahsvcolor col);

argbcolor argb_color_mix(argbcolor a, argbcolor b);

static inline color color_mix(color a, color b){
    return argb_color_mix((argbcolor){ .color = a }, (argbcolor){ .color = b }).color;
}

argbcolor argb_color_add(argbcolor a, argbcolor b);

static inline color color_add(color a, color b){
    return argb_color_add((argbcolor){ .color = a }, (argbcolor){ .color = b }).color;
}