#pragma once

#include "graphic_types.h"
#include "syscalls/syscalls.h"

typedef struct {
    color background;
    color foreground;
    color accent;
    color error;
} theme_palette;

static inline bool get_theme(theme_palette *palette){
    if (!palette) return false;
    return sreadf("/theme", palette, sizeof(theme_palette)) == sizeof(theme_palette);
}