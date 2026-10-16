#include "color.h"
#include "math/math.h"

argbcolor argb_color_mix(argbcolor a, argbcolor b){
    u16 al = (a.alpha+b.alpha)/2;
    u16 r = (a.red+b.red)/2;
    u16 g = (a.green+b.green)/2;
    u16 bl = (a.blue+b.blue)/2;
    return (argbcolor){ .alpha = al, .red = r, .green = g, .blue = bl };
}

argbcolor argb_color_add(argbcolor a, argbcolor b){
    u16 al = min(255,a.alpha+b.alpha);
    u16 r = min(255,a.red+b.red);
    u16 g = min(255,a.green+b.green);
    u16 bl = min(255,a.blue+b.blue);
    return (argbcolor){ .alpha = al, .red = r, .green = g, .blue = bl };
}
