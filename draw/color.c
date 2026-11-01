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

argbcolor ahsv_to_argb(ahsvcolor col){
    col.alpha = clampf(col.alpha, 0, 1);
    col.hue = clampf(col.hue, 0, 360);
    col.saturation = clampf(col.saturation, 0, 1);
    col.value = clampf(col.value, 0, 1);
    float h = col.hue/360.f;
	float s = col.saturation;
	float v = col.value;
	
	int i = floor_to_int(h * 6);
	float f = h * 6 - i;
	float p = v * (1-s);
	float q = v * (1-f*s);
	float t = v * (1-(1-f)*s);
	
	float r = 0, g = 0, b = 0;
	
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
    
    return (argbcolor){.alpha = col.alpha*255, .red = r*255, .green = g*255, .blue = b*255};
}

ahsvcolor argb_to_ahsv(argbcolor col){
    
    float r = col.red/255.f;
    float g = col.green/255.f;
    float b = col.blue/255.f;
    
    float cmax = maxf(r,maxf(g,b));
    float cmin = minf(r,minf(g,b));
    float diff = cmax - cmin;
    float h = 0;
    
    if (diff != 0){
        if (cmax == r)
             h = 60 * ((g-b)/diff);
        else if (cmax == g)
             h = 60 * ((b-r)/diff) + 120;
        else h = 60 * ((r-g)/diff) + 240;
        
        if (h < 0) h += 360;
        else if (h >= 360) h -= 360;
    }
    
    return (ahsvcolor){.alpha = col.alpha/255.f, .hue = h, .saturation = cmax == 0 ? 0 : (diff/cmax), .value = cmax};
}