#include "font.h"
#include "ttf/ttf.h"
#include "draw/pathdraw.h"

font_hdr* font_load(const char *location, font_type type){
    switch (type) {
        case font_type_embedded: return 0;
        case font_type_ttf: {
            ttf_font *font = zalloc(sizeof(ttf_font));

            load_ttf("ibmplexmono/IBMPlexMono-Regular.ttf", font); 

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
        fb_draw_char(ctx, location.x, location.y, index, options.scale, options.color);
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
        fb_draw_char(ctx, location.x, location.y, character, options.scale, options.color);
        return (gpu_size){CHAR_SIZE * options.scale, CHAR_SIZE * options.scale};
    }
    if (hdr->type == font_type_ttf){
        point_graph graph = ttf_get_character((ttf_font*)hdr, character);
        return fb_render_path(ctx, location, graph, options);
    }
    return (gpu_size){};
}
