#pragma once

#include "types.h"
#include "image.h"

typedef struct bmp_header {
    char signature[2];
    uint32_t file_size;
    uint32_t rsvd;
    uint32_t data_offset;

    //DIB - BITMAPINFOHEADER
    uint32_t dib_size;
    int32_t width;
    int32_t height;
    uint16_t planes;//Must be 1
    uint16_t bpp;
    uint32_t compression;//Table
    uint32_t img_size;
    
    int32_t horizontal_ppm;
    int32_t vertical_ppm;
    
    uint32_t num_colors;//0 is 2^n
    uint32_t important_colors;//0 is all, ignored
}__attribute__((packed)) bmp_header;

image_info bmp_get_info(void * file, size_t size);
void bmp_read_image(void *file, size_t size, uint32_t *buf);

bmp_header* allocate_bmp_file(u32 width, u32 height);

void* load_bmp(char *path, image_info *info);