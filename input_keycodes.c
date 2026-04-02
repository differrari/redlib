#include "input_keycodes.h"

char hid_to_char(unsigned char c, u8 modifier){
    switch (modifier) {
        case KEY_MOD_LSHIFT:
            return hid_keycode_to_shift_char[c];
        default:
            return hid_keycode_to_char[c];
    }
}