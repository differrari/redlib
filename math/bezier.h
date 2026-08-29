#include "math.h"
#include "vector_types.h"

static inline double quad_bezier(double p1, double c1, double p2, double t){
    return pow2(1.f-t)*p1 + 2.f*(1.f-t)*t*c1 + pow2(t)*p2;
}

static inline vec2 vec2_quad_bezier(vec2 p1, vec2 c1, vec2 p2, float t){
    return (vec2){quad_bezier((double)p1.x, (double)c1.x, (double)p2.x, (double)t),quad_bezier((double)p1.y, (double)c1.y, (double)p2.y, (double)t)};
}

static inline double cube_bezier(double p1, double c1, double c2, double p2, double t){
    return pow3(1.f-t)*p1 + 3*pow2(1.f-t)*t*c1 + 3.f*(1.f-t)*pow2(t)*c2 + pow3(t)*p2;
}

static inline vec2 vec2_cube_bezier(vec2 p1, vec2 c1, vec2 c2, vec2 p2, float t){
    return (vec2){cube_bezier((double)p1.x, (double)c1.x, (double)c2.x, (double)p2.x, (double)t),cube_bezier((double)p1.y, (double)c1.y, c2.y, (double)p2.y, (double)t)};
}
