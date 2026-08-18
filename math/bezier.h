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

#include "vector.h"

static inline vec2 quad_roots(float a, float b, float c){
    vec2 roots = {INFINITY,INFINITY };
    if (absf(a) < EPSILON){
        if (b != 0) roots.x = -c/b;
    } else {
        float discriminant = pow2(b) - 4*a*c;

        if (discriminant > -EPSILON) {
            float s = __builtin_sqrtf(maxf(0, discriminant));
            roots.x = (-b+s)/(2*a);
            roots.y = (-b-s)/(2*a);
        }
    }
    return roots;
}

static inline float line_bezier_coverage(vec2 p1, vec2 c1, vec2 p2){
    float coverage = 0;
    bool is_downward = p1.y > 0 || p2.y < 0;
    if (is_downward) {
        if (p1.y < 0 && p2.y <= 0) return 0;
        if (p1.y > 0 && p2.y >= 0) return 0;
    } else {
        if (p1.y <= 0 && p2.y < 0) return 0;
        if (p1.y >= 0 && p2.y > 0) return 0;
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

    float intersection0 = (a.x * pow2(t0)) + (b.x * t0) + c.x;
    float intersection1 = (a.x * pow2(t1)) + (b.x * t1) + c.x;

    int sign = is_downward ? 1 : -1;

    if (on0) coverage += saturate(0.5f + intersection0) * sign;
    if (on1) coverage += saturate(0.5f + intersection1) * sign;

    return coverage;
}
