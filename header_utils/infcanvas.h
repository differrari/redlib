#pragma once

#include "graphic_types.h"
#include "draw/draw.h"
#include "mouse_input.h"
#include "math/math.h"
#include "kbd_helper.h"

#ifndef INFCANVAS_NONSTATIC
static float zoom = 1;
static float max_zoom = 10;
static float max_grid = 500;
static float min_zoom = 1;
static float min_grid = 30;
static gpu_point pan = {};
static int grid_size = 100;
static void (*on_canvas_pan)(gpu_point point);
static void (*on_canvas_zoom)(float zoom);
#else
extern float zoom;
extern float max_zoom;
extern float max_grid;
extern float min_zoom;
extern float min_grid;
extern gpu_point pan;
extern int grid_size;
extern void (*on_canvas_pan)(gpu_point point);
extern void (*on_canvas_zoom)(float zoom);
#endif

static void canvas_background(draw_ctx *ctx, color bg, color grid){
    fb_clear(ctx, bg);
    
    if (grid & 0xFF000000){
        int current_grid_level = grid_size*zoom;
        
        gpu_point grid_pan = pan;
        grid_pan.x %= current_grid_level;
        grid_pan.y %= current_grid_level;
        
        for (int x = 0; x < ctx->width; x += current_grid_level){
            fb_draw_line(ctx, x+grid_pan.x, grid_pan.y, x+grid_pan.x, ctx->height + grid_pan.y, grid);
        }
        for (int y = 0; y < ctx->height; y += current_grid_level){
            fb_draw_line(ctx, grid_pan.x, y+grid_pan.y, ctx->width+grid_pan.x, y+grid_pan.y, grid);
        }
    }
}

static void canvas_mouse(mouse_data mouse, float dt){
    if (mouse_button_down(&mouse, MMB)){
        pan.x += mouse.raw.x;
        pan.y += mouse.raw.y;
        if (on_canvas_pan) on_canvas_pan(pan);
    }
    if (mouse.raw.scroll){
        if (is_mod_pressed(KEY_LEFTALT, true)){
            grid_size += mouse.raw.scroll * dt;
            grid_size = clampf(grid_size, min_grid, max_grid);
        } else {
            zoom += mouse.raw.scroll * dt;
            zoom = clampf(zoom, min_zoom, max_zoom);
            if (on_canvas_zoom) on_canvas_zoom(zoom);
        }
    }
}