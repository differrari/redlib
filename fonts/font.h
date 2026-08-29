#pragma once

#include "types.h"
#include "graphic_types.h"
#include "draw/point_graph.h"

typedef enum { font_type_embedded/*, font_type_bitmap*/, font_type_ttf } font_type;

typedef struct {
    font_type type;

    int num_glyphs;
} font_hdr;

typedef struct {
    path_render_options glyph;
    font_hdr *font;
    bool crop;
    float line_spacing;
    float character_spacing;
} simple_text_render_options;

font_hdr* font_load(const char *location, font_type type);

void font_destroy(font_hdr *hdr);

gpu_size font_render_glyph_index(font_hdr *hdr, path_render_options options, draw_ctx *ctx, gpu_point location, u32 index);
gpu_size font_render_glyph_character(font_hdr *hdr, path_render_options options, draw_ctx *ctx, gpu_point location, u32 character);