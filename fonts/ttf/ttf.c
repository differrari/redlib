#include "ttf.h"
#include "files/helpers.h"
#include "types.h"
#include "ttf_types.h"
#include "data/struct/chunk_array.h"
#include "data/serialize/binary_scanner.h"
#include "math/math.h"

// #define TTF_DEBUG

#ifdef TTF_DEBUG
#define ttf_print(...) print(__VA_ARGS__)
#else 
#define ttf_print(...)
#endif

bool load_cmap_unicode(ttf_cmap *map, ttf_font *out_font){
    for (int i = 0; i < map->num_subtables; i++){
        ttf_cmap_sub *sub = &map->tables[i];
        ttf_cmap_sub_swap(sub);
        if (sub->platform_id == 0){
            ttf_cmap_table_hdr *cmap_hdr = (ttf_cmap_table_hdr*)((uptr)map + sub->offset);
            ttf_cmap_table_hdr_swap(cmap_hdr);
            if (cmap_hdr->format != 4){
                print("[TTF implementation error] only format 4 is supported %i",cmap_hdr->format);
                return false;
            }
            out_font->character_map = (ttf_cmap_table_fmt4*)cmap_hdr;
            ttf_cmap_table_fmt4_swap(out_font->character_map);
            return true;
        }
    }
    return false;
}

void ttf_parse_head(ttf_font *font, ttf_head *head){
    font->glyph_size_multiplier = head->indexToLocFormat ? sizeof(u32) : sizeof(u16);
}

void ttf_parse_hhea(ttf_font *font, ttf_hhea *hhea){
    font->hdr.num_glyphs = hhea->numOfLongHorMetrics;
    font->ascent = hhea->ascent;
    font->descent = hhea->descent;
    font->advanceWidthMax = hhea->advanceWidthMax;
    font->height_ratio = (float)(font->ascent-font->descent)/font->advanceWidthMax;
    ttf_print(">>>> %i-%i/%i = %f",font->ascent,font->descent,font->advanceWidthMax,font->height_ratio);
}

bool load_ttf(char *path, ttf_font *out_font){
    if (!out_font) return false;
    size_t size = 0;
    u8 *data = (u8*)read_full_file(path, &size);

    out_font->font = data;

    if (!size || !data || size < sizeof(ttf_hdr)){
        return false;
    }

    ttf_hdr *hdr = (ttf_hdr*)data;
    ttf_hdr_swap(hdr);

    ttf_print("version=%i.%i, numtables=%i, searchRange=%i, entrySel=%i, rangeShift=%i",hdr->version_maj,hdr->version_min,hdr->num_tables,hdr->search_range,hdr->entry_selector,hdr->range_shift);

    if (!hdr->num_tables){
        print("[TTF error] no tables");
        return false;
    }

    ttf_table_hdr *tables = (ttf_table_hdr*)(data + sizeof(ttf_hdr));
    
    for (int i = 0; i < hdr->num_tables; i++){
        ttf_table_hdr *table = &tables[i];
        ttf_table_hdr_swap(table);

        ttf_print("%c%c%c%c checksum=%.8x offset=%i len=%i",table->NAME[0],table->NAME[1],table->NAME[2],table->NAME[3],table->checksum,table->offset,table->length);

        if (strncmp(table->NAME, "head", 4) == 0){
            ttf_head *head = (ttf_head*)(data + table->offset);
            ttf_head_swap(head);
            ttf_parse_head(out_font, head);
        }
        if (strncmp(table->NAME, "hhea", 4) == 0){
            ttf_hhea *hhea = (ttf_hhea*)(data + table->offset);
            ttf_hhea_swap(hhea);
            ttf_parse_hhea(out_font, hhea);
        }
        if (strncmp(table->NAME, "loca", 4) == 0){
            out_font->location_map = (u8*)(data + table->offset);
        }
        if (strncmp(table->NAME, "glyf", 4) == 0){
            out_font->glyphs = (void*)(data + table->offset);
        }
        if (strncmp(table->NAME, "cmap", 4) == 0){
            ttf_cmap *cmap = (ttf_cmap*)(data + table->offset);
            ttf_cmap_swap(cmap);
            if (!load_cmap_unicode(cmap, out_font)) return false;
        }
        if (strncmp(table->NAME, "hmtx", 4) == 0){
            ttf_hmetric *hmtx = (ttf_hmetric*)(data + table->offset);
            out_font->metrics = hmtx;
        }

    }
    
    out_font->hdr.type = font_type_ttf;
    return true;
}

bool ttf_find_glyph(ttf_font *font, ttf_glyph_loc *glyph, u16 index){
    if (!glyph) return false;
    if (font->glyph_size_multiplier == sizeof(u16)){
        u16 *table = (u16*)font->location_map;
        glyph->offset = bswap16(table[index]) * 2;
        glyph->len = (bswap16(table[index+1]) * 2) - glyph->offset;
    } else {
        u32 *table = (u32*)font->location_map;
        glyph->offset = bswap32(table[index]);
        glyph->len = (bswap32(table[index+1])) - glyph->offset;
    }
    return true;
    // print("Glyph is at %x of length %x",glyph->offset,glyph->len);
}

bool ttf_lookup_glyph_fmt4(ttf_font *font, ttf_glyph_loc *out_glyph, u16 *out_index, u16 glyph){
    if (!out_index) return false;
    ttf_cmap_table_fmt4 *table = font->character_map;
    u16 segments = table->segCountX2/2;
    u16 *endRange = (u16*)((uptr)font->character_map + sizeof(ttf_cmap_table_fmt4));
    u16 *startRange = (u16*)((uptr)endRange + (sizeof(u16)*segments) + sizeof(u16));
    u16 *delta = (u16*)((uptr)startRange + (sizeof(u16)*segments));
    u16 *rangeOffset = (u16*)((uptr)delta + (sizeof(u16)*segments));

    for (int i = 0; segments; i++){
        u16 start = bswap16(startRange[i]);
        if (glyph >= start && glyph <= bswap16(endRange[i])){
            u16 range = bswap16(rangeOffset[i]);
            if (range)
                *out_index = bswap16(*(&rangeOffset[i] + range / 2 + (glyph - start)));
            else 
                *out_index = (bswap16(delta[i]) + glyph) % 65536;
            return ttf_find_glyph(font, out_glyph, *out_index);
        }
    }
    return false;
}

bool ttf_lookup_glyph(ttf_font *font, ttf_glyph_loc *out_glyph, u16 *out_index, u16 glyph){
    switch (font->character_map->hdr.format){
        case 4:
            return ttf_lookup_glyph_fmt4(font, out_glyph, out_index, glyph);
        default: return false;
    }
}

float ttf_get_glyph_advance(ttf_font *font, u16 index){
    return (float)bswap16(font->metrics[index < font->hdr.num_glyphs ? index : 0].advanceWidth)/font->advanceWidthMax;
}

#define ttfbounds(cond) if (!(cond)){ ttf_print("Out of bounds"); }

void ttf_glyph_read_coord(bool is_x, ttf_font *font, point_graph graph, u16 num_points, ttf_glyph_flags flags[], u16 index, ttf_glyph_desc *desc, binary_scanner *scanner){
    i16 coord = 0;
    for (int i = 0; i < num_points; i++){
        ttf_glyph_flags flag = flags[i];
        bool is_short = is_x ? flag.x_short_vector : flag.y_short_vector;
        bool is_same_or_pos = is_x ? flag.x_is_same_or_positive_x_short_vector : flag.y_is_same_or_positive_y_short_vector;
        if (is_short){
            u8 val = 0;
            ttfbounds(bin_scan_u8(scanner, &val));
            coord += val * (is_same_or_pos ? 1 : -1);
        } else if (!is_same_or_pos) {
            i16 val = 0;
            ttfbounds(bin_scan_i16(scanner, &val));
            coord += val;
        }

        point_entry *entry = &graph.graph[i];
        if (is_x){
            entry->pos.x = ((float)(coord)/(font->advanceWidthMax));
        } else  
            entry->pos.y = ((float)(coord-font->ascent)/(font->descent-font->ascent))*font->height_ratio;
    }
}

point_graph ttf_simple_glyph(binary_scanner *scanner, ttf_font *font, u16 index, ttf_glyph_desc *desc){
    point_graph glyph = {
        .num_slices = desc->numberOfCountours,
        .slices = zalloc(desc->numberOfCountours * sizeof(point_graph_slice)),
        .size = {ttf_get_glyph_advance(font,index), font->height_ratio}
    };

    ttf_print("Size %i,%i %f",glyph.size.x,glyph.size.y,font->height_ratio);

    u16 num_points = 0;

    u16 last_countour = 0;
    for (int i = 0; i < desc->numberOfCountours; i++){
        u16 new_countour = 0;
        ttfbounds(bin_scan_u16(scanner, &new_countour));
        new_countour++;
        num_points = max(new_countour,num_points);
        glyph.slices[i].start = last_countour;
        glyph.slices[i].end = new_countour;
        glyph.slices[i].close = true;
        glyph.slices[i].curve_type = point_graph_curve_quad_only;
        glyph.slices[i].max_curve_points = 32;
        // print("countour %i: %i - %i",i,graph.slices[i].start,graph.slices[i].end);
        last_countour = new_countour;
    }

    glyph.num_points = num_points;
    glyph.graph = zalloc(num_points * sizeof(point_entry));

    u16 instrLength = 0;
    ttfbounds(bin_scan_u16(scanner, &instrLength));

    ttf_print("Instruction length %x",instrLength);

    ttfbounds(bin_scan_skip(scanner, instrLength));
    
    ttf_glyph_flags flags[num_points] = {};
    for (int i = 0; i < num_points; i++){
        ttf_glyph_flags *current = &flags[i];
        bin_scan_u8(scanner, &current->flags);

        glyph.graph[i].on_curve = current->on_curve;

        if (current->repeat){
            u8 amount = 0;
            ttfbounds(bin_scan_u8(scanner, &amount));

            for (int j = 0; j < amount; j++){
                
                i++;
                glyph.graph[i].on_curve = current->on_curve;
                flags[i] = *current;
            }
        }
    }

    ttf_glyph_read_coord(true, font, glyph, num_points, flags, index, desc, scanner);
    ttf_glyph_read_coord(false, font, glyph, num_points, flags, index, desc, scanner);

    // for (int i = 0; i < num_points; i++){
    //     point_entry entry = graph.graph[i];
    //     print("%i - %f,%f",entry.on_curve,entry.pos.x,entry.pos.y);
    // }

    return glyph;
}

point_graph ttf_compound_glyph(ttf_font *font, binary_scanner *scanner, u16 index, ttf_glyph_desc *desc){
    bool repeat = false;
    point_graph glyph = {};
    do {
        ttf_compound_flags flags = {};
        ttfbounds(bin_scan_u16(scanner, &flags.flags));
        u16 glyphIndex = 0;
        ttf_print("%llx flags %x",scanner->data+scanner->cursor-font->font,flags.flags);
        ttfbounds(bin_scan_u16(scanner, &glyphIndex));
        ttf_print("Glyph index %i",glyphIndex);
        ttf_glyph_loc glyphL = {};
        ttf_find_glyph(font, &glyphL, glyphIndex);
        i16 off_x = 0;
        i16 off_y = 0;

        if (flags.args_are_words){
            ttfbounds(bin_scan_i16(scanner,&off_x));
            ttfbounds(bin_scan_i16(scanner,&off_y));
        } else {
            if (flags.args_are_values){
                i8 ox = 0;
                i8 oy = 0;
                ttfbounds(bin_scan_i8(scanner,&ox));
                ttfbounds(bin_scan_i8(scanner,&oy));
                off_x = ox;
                off_y = oy;
            } else {
                u8 ox = 0;
                u8 oy = 0;
                ttfbounds(bin_scan_u8(scanner,&ox));
                ttfbounds(bin_scan_u8(scanner,&oy));
                off_x = ox;
                off_y = oy;
            }
        }

        i16 scale_x = 1, scale_y = 1, scale_01 = 1, scale_10 = 1;

        // if (flags.args_are_words && flags.args_are_values){
        //     ttfbounds(bin_scan_i16(scanner, &scale_x));
        //     ttfbounds(bin_scan_i16(scanner, &scale_x));
        // } else if (!flags.args_are_words && flags.args_are_values){
        //     i8 f, u= 0;
        //     ttfbounds(bin_scan_i8(scanner, &f));
        //     ttfbounds(bin_scan_i8(scanner, &u));
        // } else if (flags.args_are_words && !flags.args_are_values){
        //     ttfbounds(bin_scan_i16(scanner, &scale_x));
        //     ttfbounds(bin_scan_i16(scanner, &scale_x));
        //     // 1st short contains the index of matching point in compound being constructed
        //     // 2nd short contains index of matching point in component
        // } else if (!flags.args_are_words && !flags.args_are_values){
        //     i8 f, u= 0;
        //     // 1st byte containing index of matching point in compound being constructed
        //     // 2nd byte containing index of matching point in component
        //     ttfbounds(bin_scan_i8(scanner, &f));
        //     ttfbounds(bin_scan_i8(scanner, &u));
        // }
        if (flags.have_scale){
            ttfbounds(bin_scan_i16(scanner, &scale_x));
            scale_y = scale_x;
        } else if (flags.separate_scale) {
            ttfbounds(bin_scan_i16(scanner, &scale_x));
            ttfbounds(bin_scan_i16(scanner, &scale_y));
        } else if (flags.two_by_two){
            ttfbounds(bin_scan_i16(scanner, &scale_x));
            ttfbounds(bin_scan_i16(scanner, &scale_01));
            ttfbounds(bin_scan_i16(scanner, &scale_10));
            ttfbounds(bin_scan_i16(scanner, &scale_y));
        }
        repeat = flags.more_components;
        glyph = ttf_read_glyph(font, glyphIndex, &glyphL);
        return glyph;
    } while (repeat);

    return glyph;
}

point_graph ttf_read_glyph(ttf_font *font, u16 index, ttf_glyph_loc *glyph){
    if (!font || !glyph || !font->glyphs) return (point_graph){};//Return hardcoded or retrieved unknown char

    ttf_print("Glyph at %llx",(uptr)glyph->offset+(uptr)font->glyphs-(uptr)font->font);

    binary_scanner scanner = bin_scan_create((void*)((uptr)font->glyphs + glyph->offset), glyph->len, true);

    ttf_glyph_desc *desc = (void*)((uptr)font->glyphs + glyph->offset);
    ttf_glyph_desc_swap(desc);

    ttfbounds(bin_scan_skip(&scanner, sizeof(ttf_glyph_desc)));

    if (desc->numberOfCountours < 0){
        // return (point_graph){};
        return ttf_compound_glyph(font, &scanner, index, desc);
    }

    if (!desc->numberOfCountours) return (point_graph){.size = {.x = ttf_get_glyph_advance(font, index), .y = font->height_ratio}};

    ttf_print("Number of contours %i. Bounds %i,%i - %i,%i",desc->numberOfCountours,desc->xMin,desc->yMin,desc->xMax,desc->yMax);

    return ttf_simple_glyph(&scanner, font, index, desc);
}

typedef struct {
    bool active;
    point_graph glyph;
} cached_glyph;

void ttf_cache_graph_index(ttf_font *font, u16 index, point_graph glyph){
    if (!font->index_cache) font->index_cache = chunk_array_create(sizeof(cached_glyph), font->hdr.num_glyphs);
    cached_glyph *cached = CHUNK_ARRAY_ITEM(font->index_cache, index);
    cached->glyph = glyph;
    cached->active = true;
    if (font->index_cache->count < index+1) font->index_cache->count = index+1;
}

point_graph ttf_get_index(ttf_font *font, u16 index){
    cached_glyph *cached = chunk_array_get(font->index_cache, index);
    if (!cached || !cached->active){
        ttf_glyph_loc glyph = {};
        ttf_find_glyph(font, &glyph, index);
        
        point_graph graph = ttf_read_glyph(font, index, &glyph);
        ttf_cache_graph_index(font, index, graph);
        return graph;
    }
    return cached->glyph;
}

void ttf_cache_graph_char(ttf_font *font, u16 character, u16 index){
    if (!font->graph_cache) font->graph_cache = hash_map_create(font->hdr.num_glyphs);
    hash_map_put(font->graph_cache, &character, sizeof(u16), (void*)(u64)((1 << 16) | index));
}

point_graph ttf_get_character(ttf_font *font, u16 character){
    if (font->graph_cache){
        u64 val = (u64)hash_map_get(font->graph_cache, &character, sizeof(u16));
        if ((val >> 16))
            return ttf_get_index(font, (u16)(val & 0xFFFF));
    }
    ttf_glyph_loc glyph = {};
    u16 index = 0;
    if (!ttf_lookup_glyph(font, &glyph, &index, character))
        return ttf_get_index(font, 0);
    ttf_cache_graph_char(font, character, index);
    point_graph graph = ttf_read_glyph(font, index, &glyph);
    ttf_cache_graph_index(font, index, graph);
    return graph;
}

void ttf_font_destroy(ttf_font *font){
    size_t count = chunk_array_count(font->index_cache);
    for (u64 i = 0; i < count; i++){
        cached_glyph *graph = CHUNK_ARRAY_ITEM(font->index_cache, i);
        if (!graph->active) continue;
        release(graph->glyph.graph);
        release(graph->glyph.slices);
    }
    hash_map_destroy(font->graph_cache);
    chunk_array_destroy(font->index_cache);
    *font = (ttf_font){};
}