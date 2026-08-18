#pragma once

#include "types.h"

// typedef union {
//     u32 integer: 16;
//     u32 decimal: 16;
// } ttf_fixed;

typedef struct {
    u16 version_maj;
    u16 version_min;
    u16 num_tables;
    u16 search_range;
    u16 entry_selector;
    u16 range_shift;
} ttf_hdr;

static inline void ttf_hdr_swap(ttf_hdr *hdr){
    if (!hdr) return;
    hdr->version_maj = bswap16(hdr->version_maj);
    hdr->version_min = bswap16(hdr->version_min);
    hdr->num_tables = bswap16(hdr->num_tables);
    hdr->search_range = bswap16(hdr->search_range);
    hdr->entry_selector = bswap16(hdr->entry_selector);
    hdr->range_shift = bswap16(hdr->range_shift);
}

typedef struct {
    char NAME[4];
    u32 checksum;
    u32 offset;
    u32 length;
} ttf_table_hdr;

static inline void ttf_table_hdr_swap(ttf_table_hdr *hdr){
    if (!hdr) return;
    hdr->checksum = bswap32(hdr->checksum);
    hdr->offset = bswap32(hdr->offset);
    hdr->length = bswap32(hdr->length);
}

typedef union {
    struct {
        u16 baseline_at_0: 1;
        u16 idk_1: 1;
        u16 idk_2: 1;
        u16 integer_scaling: 1;
        u16 idk_4: 1;
        u16 idk_5: 1;
        u16 idk_6: 1;
        u16 idk_7: 1;
        u16 idk_8: 1;
        u16 idk_9: 1;
        u16 idk_10: 1;
        u16 idk_11: 3;
        u16 idk_14: 1;
    };
    u16 flags;
} ttf_head_flags;

typedef union {
    struct {
        u16 bold: 1;
        u16 italic: 1;
        u16 underline: 1;
        u16 outline: 1;
        u16 shadow: 1;
        u16 condensed: 1;
        u16 extended: 1;
    };
    u16 mac_style;
} ttf_head_mac_style;

typedef struct {
    u16 version_maj;
    u16 version_min;
    u16 revision_maj;
    u16 revision_min;
    u32 checksum;
    u32 magic;//0x5f0f3cf5
    ttf_head_flags flags;
    u16 unitsPerEm;
    i64 created;
    i64 modified;
    i16 xMin;
    i16 yMin;
    i16 xMax;
    i16 yMax;
    ttf_head_mac_style macStyle;
    u16 lowestReadablePPEM;
    i16 fontDirection;
    i16 indexToLocFormat;//TODO: what are these?
    i16 glyphDataFormat;//TODO: what are these?
} ttf_head;

static inline void ttf_head_swap(ttf_head *head){
    head->version_maj = bswap16(head->version_maj);
    head->version_min = bswap16(head->version_min);
    head->revision_maj = bswap16(head->revision_maj);
    head->revision_min = bswap16(head->revision_min);
    head->checksum = bswap32(head->checksum);
    head->magic = bswap16(head->magic);//0x5f0f3cf5
    head->flags.flags = bswap16(head->flags.flags);
    head->unitsPerEm = bswap16(head->unitsPerEm);
    head->created = bswap64(head->created);
    head->modified = bswap64(head->modified);
    head->xMin = bswap16(head->xMin);
    head->yMin = bswap16(head->yMin);
    head->xMax = bswap16(head->xMax);
    head->yMax = bswap16(head->yMax);
    head->macStyle.mac_style = bswap16(head->macStyle.mac_style);
    head->lowestReadablePPEM = bswap16(head->lowestReadablePPEM);
    head->fontDirection = bswap16(head->fontDirection);
    head->indexToLocFormat = bswap16(head->indexToLocFormat);
    head->glyphDataFormat = bswap16(head->glyphDataFormat);
}

typedef struct {
    u16 version_maj;
    u16 version_min;
    i16 ascent; //Distance from baseline of highest ascender
    i16 descent;//Distance from baseline of lowest descender
    i16	lineGap;//typographic line gap
    u16 advanceWidthMax;// 	must be consistent with horizontal metrics
    i16 minLeftSideBearing;// 	must be consistent with horizontal metrics
    i16 minRightSideBearing;// 	must be consistent with horizontal metrics
    i16 xMaxExtent;// 	max(lsb + (xMax-xMin))
    i16	caretSlopeRise;// 	used to calculate the slope of the caret (rise/run) set to 1 for vertical caret
    i16	caretSlopeRun;// 	0 for vertical
    i16	caretOffset;// 	set value to 0 for non-slanted fonts
    i16	reserved[4];// 	set value to 0
    i16	metricDataFormat;// 	0 for current format
    u16	numOfLongHorMetrics;// 	number of advance widths in metrics table
} ttf_hhea;

static inline void ttf_hhea_swap(ttf_hhea *hhea){
    hhea->version_maj = bswap16(hhea->version_maj);
    hhea->version_min = bswap16(hhea->version_min);
    hhea->ascent = bswap16(hhea->ascent);
    hhea->descent = bswap16(hhea->descent);
    hhea->lineGap = bswap16(hhea->lineGap);
    hhea->advanceWidthMax = bswap16(hhea->advanceWidthMax);
    hhea->minLeftSideBearing = bswap16(hhea->minLeftSideBearing);
    hhea->minRightSideBearing = bswap16(hhea->minRightSideBearing);
    hhea->xMaxExtent = bswap16(hhea->xMaxExtent);
    hhea->caretSlopeRise = bswap16(hhea->caretSlopeRise);
    hhea->caretSlopeRun = bswap16(hhea->caretSlopeRun);
    hhea->caretOffset = bswap16(hhea->caretOffset);
    hhea->metricDataFormat = bswap16(hhea->metricDataFormat);
    hhea->numOfLongHorMetrics = bswap16(hhea->numOfLongHorMetrics);
}

typedef struct {
    u16 advanceWidth;
    i16 leftSideBearing;
} ttf_hmetric;

typedef struct {
    u16 platform_id;
    u16 platform_specific_id;
    u32 offset;
} ttf_cmap_sub;

static inline void ttf_cmap_sub_swap(ttf_cmap_sub *sub){
    sub->platform_id = bswap16(sub->platform_id);
    sub->platform_specific_id = bswap16(sub->platform_specific_id);
    sub->offset = bswap32(sub->offset);
}

typedef struct {
    u16 version;
    u16 num_subtables;
    ttf_cmap_sub tables[];
} ttf_cmap;

static inline void ttf_cmap_swap(ttf_cmap *map){
    map->version = bswap16(map->version);
    map->num_subtables = bswap16(map->num_subtables);
}

typedef struct {
    u16 format;//NOTE: only format 4 currently supported
} ttf_cmap_table_hdr;

static inline void ttf_cmap_table_hdr_swap(ttf_cmap_table_hdr *hdr){
    hdr->format = bswap16(hdr->format);
}

typedef struct {
    ttf_cmap_table_hdr hdr;
    u16	length;// 	Length of subtable in bytes
    u16	language;// 	Language code (see above)
    u16	segCountX2;// 	2 * segCount
    u16	searchRange;// 	2 * (2**FLOOR(log2(segCount)))
    u16	entrySelector;// 	log2(searchRange/2)
    u16	rangeShift;// 	(2 * segCount) - searchRange
} ttf_cmap_table_fmt4;

// u16	endCode;//[segCount] 	Ending character code for each segment, last = 0xFFFF.
// u16	reservedPad;// 	This value should be zero
// u16	startCode;//[segCount] 	Starting character code for each segment
// u16	idDelta;//[segCount] 	Delta for all character codes in segment
// u16	idRangeOffset;//[segCount] 	Offset in bytes to glyph indexArray, or 0
// u16	glyphIndexArray;//[variable] 	Glyph index array

static inline void ttf_cmap_table_fmt4_swap(ttf_cmap_table_fmt4 *hdr){
    hdr->length = bswap16(hdr->length);
    hdr->language = bswap16(hdr->language);
    hdr->segCountX2 = bswap16(hdr->segCountX2);
    hdr->searchRange = bswap16(hdr->searchRange);
    hdr->entrySelector = bswap16(hdr->entrySelector);
    hdr->rangeShift = bswap16(hdr->rangeShift);
}

typedef struct {
    i16 numberOfCountours; // < 0 compound, 0 none, > 0 n
    i16 xMin;
    i16 yMin;
    i16 xMax;
    i16 yMax;
}__attribute__((packed)) ttf_glyph_desc;

static inline void ttf_glyph_desc_swap(ttf_glyph_desc *desc){
    desc->numberOfCountours = bswap16(desc->numberOfCountours);
    desc->xMin = bswap16(desc->xMin);
    desc->yMin = bswap16(desc->yMin);
    desc->xMax = bswap16(desc->xMax);
    desc->yMax = bswap16(desc->yMax);
}

typedef union {
    struct {
        u8 on_curve: 1;
        u8 x_short_vector: 1;
        u8 y_short_vector: 1;
        u8 repeat: 1;
        u8 x_is_same_or_positive_x_short_vector: 1;
        u8 y_is_same_or_positive_y_short_vector: 1;
        u8 overlap_simple: 1;
        u8 rsvd: 1;
    };
    u8 flags;
} ttf_glyph_flags;

typedef union {
    struct {
        u16 args_are_words: 1;
        u16 args_are_values: 1;
        u16 round_to_grid: 1;
        u16 have_scale: 1;
        u16 rsvd: 1;
        u16 more_components: 1;
        u16 separate_scale: 1;
        u16 two_by_two: 1;
        u16 instructions: 1;
        u16 metrics: 1;
        u16 overlap: 1;
        u16 rsvd2: 5;
    };
    u16 flags;
} ttf_compound_flags;