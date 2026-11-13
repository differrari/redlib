#pragma once

#include "types.h"
#include "slice.h"

typedef struct {
    u32 character;
    u8 extra_byte_size;
} utf_codepoint;

utf_codepoint utf8_get_character(char *start);
size_t utf8_count_slice_characters(string_slice c);

static inline size_t utf8_count_characters(char *c){
    return utf8_count_slice_characters((string_slice){ .data = c, .length = strlen(c) });
}

