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