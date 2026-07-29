#pragma once

#include "graphic_types.h"

color text_color_for_base(color base);
color complementary_color(color base);
argbcolor saturate_argb(argbcolor col, u32 t);

static inline u32 saturate(color col, u32 t){
    return saturate_argb((argbcolor){.color = col}, t).color;
}