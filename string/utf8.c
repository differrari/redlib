#include "utf8.h"

#define UTF8_READ(data,i) (data[i])

#define UTF_HAS_1_EXTRA_MASK 0b11100000
#define UTF_HAS_1_EXTRA_VALUE 0b11000000
#define UTF_HAS_2_EXTRA_MASK 0b11110000
#define UTF_HAS_2_EXTRA_VALUE 0b11100000
#define UTF_HAS_3_EXTRA_MASK 0b11111000
#define UTF_HAS_3_EXTRA_VALUE 0b11110000

#define UTF_1_BYTE_FIRST_MASK 0b00011111
#define UTF_2_BYTE_FIRST_MASK 0b00001111
#define UTF_3_BYTE_FIRST_MASK 0b00000111

#define UTF_EXTRA_BITE_MASK 0b00111111

utf_codepoint utf8_get_character(char *start){//TODO: follow bounds, adding a limit to how many bytes can be read
    if (!start) return (utf_codepoint){};
    u32 character = 0;
    u8 skip = 0;
    char c = *start;
    if (c & UTF_HAS_EXTRA_BYTES_MASK){
        if ((c & UTF_HAS_1_EXTRA_MASK) == UTF_HAS_1_EXTRA_VALUE){
            u8 fb = c & UTF_1_BYTE_FIRST_MASK;
            u8 sb = start[1] & UTF_EXTRA_BITE_MASK;
            character = ((u32)fb << 6) | sb;
            skip = 1;
        } else if ((c & UTF_HAS_2_EXTRA_MASK) == UTF_HAS_2_EXTRA_VALUE){
            u8 fb = c & UTF_2_BYTE_FIRST_MASK;
            u8 sb = start[1] & UTF_EXTRA_BITE_MASK;
            u8 tb = start[2] & UTF_EXTRA_BITE_MASK;
            character = ((u32)fb << 12) | ((u32)sb << 6) | tb;
            skip = 2;
        } else if ((c & UTF_HAS_3_EXTRA_MASK) == UTF_HAS_3_EXTRA_VALUE){
            u8 fb = c & UTF_3_BYTE_FIRST_MASK;
            u8 sb = start[1] & UTF_EXTRA_BITE_MASK;
            u8 tb = start[2] & UTF_EXTRA_BITE_MASK;
            u8 ftb = start[3] & UTF_EXTRA_BITE_MASK;
            character = ((u32)fb << 18) | ((u32)sb << 12) | (tb << 6) | ftb;
            skip = 3;
        }
    } else character = c & 0xFF;
    return (utf_codepoint){
        .character = character,
        .extra_byte_size = skip
    };
}

size_t utf8_count_slice_characters(string_slice sl){
    size_t num_chars = 0;
    for (u64 i = 0; i < sl.length; i++){
        char c = sl.data[i];
        if (c & UTF_HAS_EXTRA_BYTES_MASK){
            if ((c & UTF_HAS_1_EXTRA_MASK) == UTF_HAS_1_EXTRA_VALUE){
                i++;
            } else if ((c & UTF_HAS_2_EXTRA_MASK) == UTF_HAS_2_EXTRA_VALUE){
                i += 2;
            } else if ((c & UTF_HAS_3_EXTRA_MASK) == UTF_HAS_3_EXTRA_VALUE){
                i += 3;
            }
        }
        num_chars++;
    }
    return num_chars;
}