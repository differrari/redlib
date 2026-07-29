#include "color.h"

uint32_t text_color_for_base(uint32_t base){
    uint8_t r = (base & 0xFF);
    uint8_t g = ((base << 8) & 0xFF);
    uint8_t b = ((base << 16) & 0xFF);
    uint8_t avg = (r+g+b)/3;
    if (avg < 0x77) avg = 255-avg;
    return (0xFF << 24) | (avg << 16) | (avg << 8) | avg; 
}

color complementary_color(color base){
    u8 r = 255 - (base & 0xFF);
    u8 g = 255 - ((base << 8) & 0xFF);
    u8 b = 255 - ((base << 16) & 0xFF);
    return (0xFF << 24) | (r << 16) | (g << 8) | b; 
}

argbcolor saturate_argb(argbcolor col, u32 t){
    //TODO: t seems to be getting interpreted as the opposite of what it should. Probably because of module. This function was originally desaturate
    u32 i = ((col.red + col.green + col.blue)/3) % 256;

    u32 dr = i - col.red;
    u32 dg = i - col.green;
    u32 db = i - col.blue;

    u32 r = col.red     + dr * t;
    u32 g = col.green   + dg * t;
    u32 b = col.blue    + db * t;

    return (argbcolor){.red = r % 256, .green = g % 256, .blue = b % 256, .alpha = col.alpha};
}