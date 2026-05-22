#pragma once

#include "types.h"
#include "graphic_types.h"
#include "files/data_signatures.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { embedded_fmt_invalid, embeddef_fmt_color, embeddef_fmt_bg, embeddef_fmt_cursor_abs, embeddef_fmt_cursor_rel, embeddef_fmt_screen, embedded_fmt_count } embedded_fmt_feature;

typedef struct {
    color current_text_color;
    color default_text_color;
    color current_bg_color;
    color default_bg_color;
    u32 cursor_x, cursor_y;
    bool wipe;
    data_signature state_type;
    void* state;
} embedded_fmt;

bool embedded_fmt_parse(embedded_fmt *formatter, char c);

#ifdef __cplusplus
}
#endif