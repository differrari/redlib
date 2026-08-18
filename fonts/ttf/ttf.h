#pragma once

#include "data/struct/hashmap.h"
#include "data/struct/chunk_array.h"
#include "types.h"
#include "ttf_types.h"
#include "draw/point_graph.h"

typedef struct {
    u8 *font;
    ttf_cmap_table_fmt4 *character_map;
    size_t glyph_size_multiplier;
    u8 *location_map;
    void *glyphs;
    hash_map_t *graph_cache;
    chunk_array_t *index_cache;
    u16 num_glyphs;
} ttf_font;

typedef struct {
    u32 offset;
    u32 len;
} ttf_glyph;

bool load_ttf(char *path, ttf_font *out_font);

bool ttf_lookup_glyph(ttf_font *font, ttf_glyph *out_glyph, u16 *out_index, u16 glyph);
bool ttf_find_glyph(ttf_font *font, ttf_glyph *glyph, u16 index);

point_graph ttf_read_glyph(ttf_font *font, ttf_glyph *glyph);

point_graph ttf_get_index(ttf_font *font, u16 index);
point_graph ttf_get_character(ttf_font *font, u16 character);

void ttf_tmp_cache_all(ttf_font *font);