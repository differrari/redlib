#ifdef CROSS

#include "image/image.h"
#include "raylib.h"
#include "std/string.h"
#include "syscalls/syscalls.h"
#include "std/memory.h"

void* load_image(char *path, image_info *info, IMAGE_FORMATS format){
    if (strstart_case(path,"/resources",true) == 10) path++;
    Texture2D *ptr = malloc(sizeof(Texture2D));
    Texture2D t2d = LoadTexture(path);
    memcpy(ptr, &t2d, sizeof(Texture2D));
    info->width = t2d.width;
    info->height = t2d.height;
    return ptr;
}

void rescale_image(uint32_t old_width, uint32_t old_height, uint32_t new_width, uint32_t new_height, uint32_t *old_img, uint32_t* new_img){
    memcpy(new_img, old_img, sizeof(Texture2D));
    Texture2D *img = (Texture2D*)new_img;
    img->width = new_width;
    img->height = new_height;
}

#endif