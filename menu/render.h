#pragma once

#include "types.h"
#include "graphic_types.h"
#include "load.h"

#if CROSS
#define MENU_HEIGHT 50
#define BORDER_SIZE 5
#endif

void set_menu_color(color color);
bool draw_menu(draw_ctx *screen_ctx, gpu_point mouse_pos, bool mouse_pressed);