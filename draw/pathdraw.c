#include "pathdraw.h"
#include "math/vector.h"
#include "math/bezier.h"
#include "draw.h"

void fb_draw_bezier_quadratic(draw_ctx *ctx, vec2 p1, vec2 c1, vec2 p2, color color){
    vec2 last = {};
    for (int i = 0; i <= 10; i++){
        vec2 v = vec2_quad_bezier(p1,c1,p2,i/10.f);
        if (i > 0)
            fb_draw_line(ctx, last.x, last.y, v.x, v.y, color);
        last = v;
    }
}

void fb_draw_bezier_cubic(draw_ctx *ctx, vec2 p1, vec2 c1, vec2 c2, vec2 p2, color color){
    vec2 last = {}; 
    for (int i = 0; i <= 10; i++){
        vec2 v = vec2_cube_bezier(p1,c1,c2,p2,i/10.f);
        if (i > 0)
            fb_draw_line(ctx, last.x, last.y, v.x, v.y, color);
        last = v;
    }
}

gpu_size fb_draw_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options){
    for (int s = 0; s < graph.num_slices; s++){
        size_t count = min(graph.num_points,graph.slices[s].end); 
        vec2 loc = {};
        i8 max_points = graph.slices[s].max_curve_points;
        point_graph_curve_type curve = graph.slices[s].curve_type;
        if (graph.slices[s].close){
            point_entry entry = graph.graph[count-1];
            loc = (vec2){point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
        } else {
            point_entry entry = graph.graph[graph.slices[s].start];
            loc = (vec2){point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
        }
        vec2 controls[max_points] = {};
        int num_controls = 0;
        // print("Slice %i - %i to %i",s, graph.slices[s].start,count);
        for (size_t i = max(0,graph.slices[s].start) + !graph.slices[s].close; i < count; i++){
            point_entry entry = graph.graph[i];
            vec2 nloc = {point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
            // print("%i,%i -> %i,%i",loc.x,loc.y,nloc.x,nloc.y);
            if (nloc.x < 0 || nloc.y < 0) continue;
            if (entry.on_curve || curve == point_graph_curve_none || i == count-1){

                if (curve == point_graph_curve_none || num_controls == 0 || num_controls > max_points){
                    fb_draw_line(ctx, loc.x, loc.y, nloc.x, nloc.y, options.color);
                } else if (num_controls == 2 && curve == point_graph_curve_quad_cube){
                    fb_draw_bezier_cubic(ctx, loc, controls[0], controls[1], nloc, options.color);
                } else {
                    if (num_controls > 1){
                        vec2 last = loc;
                        for (int j = 1; j < num_controls; j++){
                            vec2 l2 = (vec2){(controls[j-1].x+controls[j].x)/2.f, (controls[j-1].y+controls[j].y)/2.f};
                            fb_draw_bezier_quadratic(ctx, last, controls[j-1], l2, options.color);
                            last = l2;
                        }
                        fb_draw_bezier_quadratic(ctx, last, controls[num_controls-1], nloc, options.color);
                    } else {
                        fb_draw_bezier_quadratic(ctx, loc, controls[0], nloc, options.color);
                    }
                }

                num_controls = 0;
                loc = nloc;
            } else {
                if (num_controls >= max_points){
                    return (gpu_size){};
                }
                controls[num_controls++] = nloc;
            }
        }
    }
    return (gpu_size)PATH_SIZE;
}

static inline vec2 quad_roots(float a, float b, float c){
    vec2 roots = {INFINITY,INFINITY };
    if (absf(a) < EPSILON){
        if (b != 0) roots.x = -c/b;
    } else {
        float discriminant = pow2(b) - 4*a*c;

        if (discriminant > -EPSILON) {
            float s = sqrtf(maxf(0, discriminant));
            roots.x = (-b+s)/(2*a);
            roots.y = (-b-s)/(2*a);
        }
    }
    return roots;
}

static inline void line_bezier_coverage(vec2 p1, vec2 c1, vec2 p2, float *coverages, u32 line_size){
    bool is_downward = p1.y > 0 || p2.y < 0;
    if (is_downward) {
        if (p1.y < 0 && p2.y <= 0) return;
        if (p1.y > 0 && p2.y >= 0) return;
    } else {
        if (p1.y <= 0 && p2.y < 0) return;
        if (p1.y >= 0 && p2.y > 0) return;
    }

    vec2 a = 
    vector2_sub(
        vector2_add(p1,p2),
        vector2_scale(c1,2)
    );
    vec2 b = vector2_scale(vector2_sub(c1, p1),2);
    vec2 c = p1;

    vec2 roots = quad_roots(a.y, b.y, c.y);

    bool on0 = roots.x >= -EPSILON && roots.x <= 1 + EPSILON;
    bool on1 = roots.y >= -EPSILON && roots.y <= 1 + EPSILON;

    float t0 = saturate(roots.x);
    float t1 = saturate(roots.y);

    float intersection0 = on0 ? (a.x * pow2(t0)) + (b.x * t0) + c.x : -1000;
    float intersection1 = on1 ? (a.x * pow2(t1)) + (b.x * t1) + c.x : 1000;
    
    int sign = is_downward ? 1 : -1;
    
    for (u32 start = 0; start < line_size; start++){
        if (on0 && intersection0 <= start) coverages[start] += saturate(0.5f + (start-intersection0)) * sign;
        if (on1 && intersection1 <= start) coverages[start] += saturate(0.5f + (start-intersection1)) * sign;
    }
}

#define line_int_check(a,b,c) line_bezier_coverage(vector2_sub(a,start),vector2_sub(b,start),vector2_sub(c,start), alphas, size.width)

gpu_size fb_fill_path(draw_ctx *ctx, gpu_point point, point_graph graph, path_render_options options){
    gpu_size size = PATH_SIZE;
    // 
    u8 base_alpha = ((options.color >> 24) & 0xFF);
    u32 base_color = options.color & 0xFFFFFF;
    for (u32 y = 0; y < size.height; y++){
        float alphas[size.width] = {};
        vec2 start = {point.x,point.y + y };
        // fb_draw_line(ctx, start.x, start.y, end.x, end.y, 0xFFFF0000);
        for (int s = 0; s < graph.num_slices; s++){
            size_t count = min(graph.num_points,graph.slices[s].end); 
            vec2 loc = {};
            i8 max_points = graph.slices[s].max_curve_points;
            point_graph_curve_type curve = graph.slices[s].curve_type;
            if (graph.slices[s].close){
                point_entry entry = graph.graph[count-1];
                loc = (vec2){point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
            } else {
                point_entry entry = graph.graph[graph.slices[s].start];
                loc = (vec2){point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
            }
            vec2 controls[max_points] = {};
            int num_controls = 0;
            for (size_t i = max(0,graph.slices[s].start) + !graph.slices[s].close; i < count; i++){
                point_entry entry = graph.graph[i];
                vec2 nloc = {point.x+(entry.pos.x*options.scale*CHAR_SIZE),point.y+(entry.pos.y*options.scale*CHAR_SIZE)};
                // print("%i,%i -> %i,%i",loc.x,loc.y,nloc.x,nloc.y);
                if (nloc.x < 0 || nloc.y < 0) continue;
                if (entry.on_curve || curve == point_graph_curve_none || i == count-1){

                    if (curve == point_graph_curve_none || num_controls == 0 || num_controls > max_points){
                        vec2 c = (vec2){(loc.x+nloc.x)/2.f, (loc.y+nloc.y)/2.f};
                        line_int_check(loc,c,nloc);
                        // fb_draw_line(ctx, 0, loc.y, 0, nloc.y, 0xFFb4dd13);
                    } else if (num_controls == 2 && curve == point_graph_curve_quad_cube){
                        // print("[DRAW error] fill path does not support cubic beziers");
                        return (gpu_size){};
                    } else {
                        if (num_controls > 1){
                            vec2 last = loc;
                            for (int j = 1; j < num_controls; j++){
                                vec2 l2 = (vec2){(controls[j-1].x+controls[j].x)/2.f, (controls[j-1].y+controls[j].y)/2.f};
                                line_int_check(last,controls[j-1],l2);
                                last = l2;
                            }
                            line_int_check(last,controls[num_controls-1],nloc);
                        } else {
                            line_int_check(loc,controls[0],nloc);
                        }
                    }

                    num_controls = 0;
                    loc = nloc;
                } else {
                    if (num_controls >= max_points){
                        return (gpu_size){};
                    }
                    controls[num_controls++] = nloc;
                }
            }
        }
        for (u32 x = 0; x < size.width; x++){
            float alpha = alphas[x];
            if (alpha){
                alpha = saturate(alpha);
                u8 al = alpha * base_alpha;
                fb_draw_pixel(ctx, start.x + x, start.y, (al << 24) | base_color);
            }
        }
    }
    // fb_draw_path(ctx, point, scale, graph);
    return size;
}