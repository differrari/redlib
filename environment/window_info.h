#pragma once

#include "types.h"

#define window_info_name_lit(n) (window_info_t){ .name = n, .name_length = sizeof(n)-1 }

typedef struct {
    char name[255];
    u8 name_length;
} window_info_t;