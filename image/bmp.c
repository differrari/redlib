#include "bmp.h"
#include "math/math.h"
#include "syscalls/syscalls.h"

image_info bmp_get_info(void * file, size_t size){
    if (size < sizeof(bmp_header)) return (image_info){0,0};
    bmp_header *header = (bmp_header*)file;
    if (header->signature[0] != 'B' || header->signature[1] != 'M') return (image_info){0,0};
    return (image_info){
        .width = header->width,
        .height = abs(header->height)
    };
}

void bmp_read_image(void *file, size_t size, uint32_t *buf){
    if (size < sizeof(bmp_header)){ 
        printf("Wrong file size");
        return;
    }
    bmp_header *header = (bmp_header*)file;
    if (size < header->data_offset + header->img_size || size < header->file_size){ 
        printf("Wrong file size");
        return;
    }
    if (header->signature[0] != 'B' || header->signature[1] != 'M'){
        print("Wrong signature %c%c. Not a BMP",header->signature[0],header->signature[1]);
        return;
    }
    uintptr_t color_data = (uintptr_t)file + header->data_offset;
    uint16_t increment = header->bpp/8;
    uint32_t height = abs(header->height);
    uint32_t width = (uint32_t)header->width;
    bool flipped = header->height > 0;
    uint32_t padding = 4 - (((header->bpp/8) * width) % 4);
    if (padding == 4) padding = 0;
    uint32_t padded = ((header->bpp/8) * width) + padding;
    for (uint32_t y = 0; y < height; y++){
        for (uint32_t x = 0; x < (uint32_t)header->width; x++)
            buf[(y * header->width) + x] = get_bpp_converted_color(header->bpp, color_data + ((flipped ? height - y - 1 : y) * padded) + x * increment);   
    }
}

void* load_bmp(char *path, image_info *info){
    file descriptor = {};
    FS_RESULT res = openf(path, &descriptor);
    void *img = 0;
    image_info img_info;
    if (res == FS_RESULT_SUCCESS){
        void *img_file = (void*)zalloc(descriptor.size);
        readf(&descriptor, img_file, descriptor.size);
        img_info = bmp_get_info(img_file, descriptor.size);
        closef(&descriptor);
        if (img_info.width > 0 && img_info.height > 0){
            size_t image_size = img_info.width * img_info.height * system_bpp;
            img = (void*)zalloc(image_size);
            bmp_read_image(img_file, descriptor.size, img);
            *info = img_info;
            return img;
        } else { 
            printf("Wrong image size %i",img_info.width,img_info.height);
            *info = (image_info){0, 0};
            return 0;
        }
    } else { 
        printf("Failed to open image");
        *info = (image_info){0, 0};
        return 0;
    }
}

bmp_header* allocate_bmp_file(u32 width, u32 height){
    size_t data_offset = sizeof(bmp_header) + 10/* Align */;
    size_t bmp_size = (sizeof(u32) * width * height) + data_offset;
    
    void *bmp = zalloc(bmp_size);
    
    bmp_header *header = bmp;
    
    header->dib_size = 40;
    header->signature[0] = 'B';
    header->signature[1] = 'M';
    header->data_offset = data_offset;
    header->img_size = bmp_size - data_offset;
    header->bpp = 32;
    header->file_size = bmp_size;
    header->height = -height;
    header->width = width;
    header->planes = 1;
    
    return header;
}