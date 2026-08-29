#pragma once

#include "types.h" 
#include "graphic_types.h"

#define FULL_REDRAW_THRESHOLD_PCT 35u

void mark_dirty(draw_ctx *ctx, u32 x, uint32_t y, u32 w, u32 h);