#pragma once

#include "data/struct/hashmap.h"
#include "data/struct/chunk_array.h"
#include "types.h"
#include "ttf_types.h"
#include "draw/point_graph.h"
#include "fonts/font.h"
#include "cache/cache.h"

typedef struct {
    font_hdr hdr;
    u8 *font;
    ttf_cmap_table_fmt4 *character_map;
    size_t glyph_size_multiplier;
    u8 *location_map;
    void *glyphs;
    ttf_hmetric *metrics;

    hash_map_t *graph_cache;
    chunk_array_t *index_cache;
    
    i16 ascent;
    i16 descent;

    u16 max_number_contours_compound;

    u16 unitsPerEm;

    float height_ratio;

    cache_config character_cache;

} ttf_font;

typedef struct {
    u32 offset;
    u32 len;
} ttf_glyph_loc;

bool load_ttf(const char *path, ttf_font *out_font);
point_graph ttf_get_index(ttf_font *font, u16 index);
point_graph ttf_get_character(ttf_font *font, u16 character);
void ttf_font_destroy(ttf_font *font);

// Internal util functions
bool ttf_lookup_glyph(ttf_font *font, ttf_glyph_loc *out_glyph, u16 *out_index, u16 glyph);
bool ttf_find_glyph(ttf_font *font, ttf_glyph_loc *glyph, u16 index);
point_graph ttf_read_glyph(ttf_font *font, u16 index, ttf_glyph_loc *glyph);