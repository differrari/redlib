#include "font.h"
#include "ttf/ttf.h"
#include "draw/pathdraw.h"
#include "cache/cache.h"
#include "header_utils/composite.h"

cache_config font_make_cache(int num_glyphs){
    return cache_configure((cache_config){
        .allow_overwrite = false,
        .initial_capacity = num_glyphs,
        .keylen = sizeof(u64),
        .timeout_len = 60000,
        .policy = cache_invalidate_timeout | cache_invalidate_manual
    });
}

font_hdr* font_load(const char *location, font_type type){
    switch (type) {
        case font_type_embedded: return 0;
        case font_type_ttf: {
            ttf_font *font = zalloc(sizeof(ttf_font));
            load_ttf(location, font); 
            font->character_cache = font_make_cache(font->hdr.num_glyphs);
            
            return (font_hdr*)font;
        }
    }
}

void font_destroy(font_hdr *hdr){
    if (!hdr) return;
    switch (hdr->type) {
        case font_type_ttf: {
            ttf_font *font = (ttf_font*)hdr;
            ttf_font_destroy(font);
            release(font);
            break;
        }
        default: return;
    }
}

gpu_size font_render_glyph_index(font_hdr *hdr, path_render_options options, draw_ctx *ctx, gpu_point location, u32 index){
    if (!options.type) options.type = path_render_fill;
    if (!options.scale) options.scale = 1;
    if (!hdr || hdr->type == font_type_embedded){
        fb_draw_raw_char(ctx, location.x, location.y, index, options.scale, options.color);
        return (gpu_size){CHAR_SIZE * options.scale, CHAR_SIZE * options.scale};
    }
    if (hdr->type == font_type_ttf){
        point_graph graph = ttf_get_index((ttf_font*)hdr, index);
        return fb_render_path(ctx, location, graph, options);
    }
    return (gpu_size){};
}

gpu_size font_render_glyph_character(font_hdr *hdr, path_render_options options, draw_ctx *ctx, gpu_point location, u32 character){
    if (!options.type) options.type = path_render_fill;
    if (!options.scale) options.scale = 1;
    if (!hdr || hdr->type == font_type_embedded){
        fb_draw_raw_char(ctx, location.x, location.y, character, options.scale, options.color);
        return (gpu_size){CHAR_SIZE * options.scale, CHAR_SIZE * options.scale};
    }
    if (hdr->type == font_type_ttf){
        ttf_font *font = (ttf_font*)hdr;
        u64 key = ((u64)options.scale << 32) | character;
        sizedptr cptr = cache_get(font->character_cache, &key);
        if (cptr.ptr){
            gpu_size size = *(gpu_size*)cptr.ptr;
            draw_ctx newctx = buffer_to_draw_ctx((void*)cptr.ptr + sizeof(gpu_size), size.width, size.height);
            fb_draw_img(ctx, location.x, location.y, newctx.fb, newctx.width, newctx.height);
            return size;
        } else {
            point_graph graph = ttf_get_character(font, character);
            gpu_size size = PATH_SIZE;
            size_t len = (size.width * size.height * sizeof(color)) + sizeof(gpu_size);
            cptr = cache_prealloc(font->character_cache, &key, len);
            draw_ctx newctx = buffer_to_draw_ctx((void*)cptr.ptr + sizeof(gpu_size), size.width, size.height);
            fb_render_path(&newctx, (gpu_point){}, graph, options);
            *(gpu_size*)cptr.ptr = size;
            fb_draw_img(ctx, location.x, location.y, newctx.fb, newctx.width, newctx.height);
            return size;
        }
    }
    return (gpu_size){};
}
