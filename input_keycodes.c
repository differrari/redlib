#include "input_keycodes.h"

bool is_alnum_hid(unsigned char c){
    return c >= 0x04 && c <= 0x1D;
}

char hid_to_char(unsigned char c, u8 modifier, u8 special){
    if (special == KEY_CAPSLOCK && is_alnum_hid(c))
        return hid_keycode_to_shift_char[c];
    switch (modifier) {
        case KEY_MOD_LSHIFT:
        case KEY_MOD_RSHIFT:
            return hid_keycode_to_shift_char[c];
        case KEY_MOD_LCTRL:
        case KEY_MOD_RCTRL:
            return hid_keycode_to_char[c] & 0b11111;
        default:
            return hid_keycode_to_char[c];
    }
}