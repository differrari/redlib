#pragma once

#include "graphic_types.h"

argbcolor argb_color_mix(argbcolor a, argbcolor b);

static inline color color_mix(color a, color b){
    return argb_color_mix((argbcolor){ .color = a }, (argbcolor){ .color = b }).color;
}

argbcolor argb_color_add(argbcolor a, argbcolor b);

static inline color color_add(color a, color b){
    return argb_color_add((argbcolor){ .color = a }, (argbcolor){ .color = b }).color;
}