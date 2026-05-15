#include "input_keycodes.h"
#include "syscalls/syscalls.h"

char hid_to_char(unsigned char c, u8 modifier){
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