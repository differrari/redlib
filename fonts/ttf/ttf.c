#include "ttf.h"
#include "files/helpers.h"
#include "types.h"
#include "ttf_types.h"
#include "data/struct/chunk_array.h"
#include "data/serialize/binary_scanner.h"
#include "math/math.h"

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

bool ttf_parse_head(ttf_font *font, ttf_head *head){
    font->glyph_size_multiplier = head->indexToLocFormat ? sizeof(u32) : sizeof(u16);
    return true;
}

bool load_ttf(char *path, ttf_font *out_font){
    if (!out_font) return false;
    size_t size = 0;
    u8 *data = (u8*)read_full_file(path, &size);
    if (!size || !data || size < sizeof(ttf_hdr)){
        print("BAD %s",path);
        return false;
    }

    ttf_hdr *hdr = (ttf_hdr*)data;
    ttf_hdr_swap(hdr);

    print("version=%i.%i, numtables=%i, searchRange=%i, entrySel=%i, rangeShift=%i",hdr->version_maj,hdr->version_min,hdr->num_tables,hdr->search_range,hdr->entry_selector,hdr->range_shift);

    if (!hdr->num_tables){
        print("No tables");
        return false;
    }

    ttf_table_hdr *tables = (ttf_table_hdr*)(data + sizeof(ttf_hdr));
    
    for (int i = 0; i < hdr->num_tables; i++){
        ttf_table_hdr *table = &tables[i];
        ttf_table_hdr_swap(table);

        print("%c%c%c%c checksum=%.8x offset=%i len=%i",table->NAME[0],table->NAME[1],table->NAME[2],table->NAME[3],table->checksum,table->offset,table->length);

        if (strncmp(table->NAME, "head", 4) == 0){
            ttf_head *head = (ttf_head*)(data + table->offset);
            ttf_head_swap(head);
            ttf_parse_head(out_font, head);
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

    }
    
    return true;
}

bool ttf_find_glyph(ttf_font *font, ttf_glyph *glyph, u16 index){
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
    print("Glyph is at %x of length %x",glyph->offset,glyph->len);
}

bool ttf_lookup_glyph_fmt4(ttf_font *font, ttf_glyph *out_glyph, u16 glyph){
    ttf_cmap_table_fmt4 *table = font->character_map;
    u16 segments = table->segCountX2/2;
    u16 *endRange = (u16*)((uptr)font->character_map + sizeof(ttf_cmap_table_fmt4));
    u16 *startRange = (u16*)((uptr)endRange + (sizeof(u16)*segments) + sizeof(u16));
    u16 *delta = (u16*)((uptr)startRange + (sizeof(u16)*segments));
    u16 *rangeOffset = (u16*)((uptr)delta + (sizeof(u16)*segments));
    print("Range %llx",(uptr)rangeOffset-(uptr)table);

    uptr end = (uptr)rangeOffset + (sizeof(u16)*segments);
    for (int i = 0; segments; i++){
        if (glyph >= bswap16(startRange[i]) && glyph <= bswap16(endRange[i])){
            u16 range = bswap16(rangeOffset[i]);
            u16 index = 0;
            if (range){
                index = range + 2 * (glyph - bswap16(startRange[i]));
            } else {
                index = bswap16(delta[i]) + glyph;
            }
            print("Location %llx with range %i",index,range);
            return ttf_find_glyph(font, out_glyph, index);
        }
    }
    return false;
}

bool ttf_lookup_glyph(ttf_font *font, ttf_glyph *out_glyph, u16 glyph){
    switch (font->character_map->hdr.format){
        case 4:
            return ttf_lookup_glyph_fmt4(font, out_glyph, glyph);
        default: return false;
    }
}

#define tffbounds(cond) if (!(cond)){ print("Failed to read"); }

void ttf_glyph_read_coord(bool is_x, point_graph graph, u16 num_points, ttf_glyph_flags flags[], ttf_glyph_desc *desc, binary_scanner *scanner){
    i16 coord = 0;
    i16 max = is_x ? desc->xMax : desc->yMax;
    i16 min = is_x ? desc->xMin : desc->yMin;
    for (int i = 0; i < num_points; i++){
        ttf_glyph_flags flag = flags[i];
        bool is_short = is_x ? flag.x_short_vector : flag.y_short_vector;
        bool is_same_or_pos = is_x ? flag.x_is_same_or_positive_x_short_vector : flag.y_is_same_or_positive_y_short_vector;
        if (is_short){
            u8 val = 0;
            tffbounds(bin_scan_u8(scanner, &val));
            coord += val * (is_same_or_pos ? 1 : -1);
        } else if (!is_same_or_pos) {
            i16 val = 0;
            tffbounds(bin_scan_i16(scanner, &val));
            coord += val;
        }

        point_entry *entry = &graph.graph[i];
        if (is_x)
            entry->x = (float)(coord-min)/max;
        else  
            entry->y = 1.f-((float)(coord-min)/max);
    }
}

point_graph ttf_read_glyph(ttf_font *font, ttf_glyph *glyph){
    if (!font || !glyph || !font->glyphs) return (point_graph){};//Return hardcoded or retrieved unknown char

    binary_scanner scanner = bin_scan_create((void*)((uptr)font->glyphs + glyph->offset), glyph->len);
    scanner.swap_endian = true;

    ttf_glyph_desc *desc = (void*)((uptr)font->glyphs + glyph->offset);
    ttf_glyph_desc_swap(desc);

    tffbounds(bin_scan_skip(&scanner, sizeof(ttf_glyph_desc)));

    if (desc->numberOfCountours < 0){
        print("[TTF implementation error] compound glyphs not supported yet");
        return (point_graph){};//Return hardcoded or retrieved unknown char
    }

    if (!desc->numberOfCountours) return (point_graph){.num_points = 0,.num_slices = 0};

    print("Number of contours %i. Bounds %i,%i - %i,%i",desc->numberOfCountours,desc->xMin,desc->yMin,desc->xMax,desc->yMax);

    point_graph graph = {
        .num_slices = desc->numberOfCountours,
        .slices = zalloc(desc->numberOfCountours * sizeof(point_graph_slice))
    };

    u16 num_points = 0;

    u16 last_countour = 0;
    for (int i = 0; i < desc->numberOfCountours; i++){
        u16 new_countour = 0;
        tffbounds(bin_scan_u16(&scanner, &new_countour));
        new_countour++;
        num_points = max(new_countour,num_points);
        graph.slices[i].start = last_countour;
        graph.slices[i].end = new_countour;
        print("countour %i: %i - %i",i,graph.slices[i].start,graph.slices[i].end);
        last_countour = new_countour;
    }

    graph.num_points = num_points;
    graph.graph = zalloc(num_points * sizeof(point_entry));

    u16 instrLength = 0;
    tffbounds(bin_scan_u16(&scanner, &instrLength));

    print("Instruction length %x",instrLength);

    tffbounds(bin_scan_skip(&scanner, instrLength));
    
    ttf_glyph_flags flags[num_points] = {};
    for (int i = 0; i < num_points; i++){
        ttf_glyph_flags *current = &flags[i];
        bin_scan_u8(&scanner, &current->flags);

        graph.graph[i].on_curve = current->on_curve;

        if (current->repeat){
            u8 amount = 0;
            tffbounds(bin_scan_u8(&scanner, &amount));

            for (int j = 0; j < amount; j++){
                
                i++;
                graph.graph[i].on_curve = current->on_curve;
                flags[i] = *current;
            }
        }
    }

    ttf_glyph_read_coord(true, graph, num_points, flags, desc, &scanner);
    ttf_glyph_read_coord(false, graph, num_points, flags, desc, &scanner);

    // for (int i = 0; i < num_points; i++){
    //     point_entry *entry = chunk_array_get(points, i);
    //     print("%i - %f,%f",entry->on_curve,entry->x,entry->y);
    // }

    return graph;
}