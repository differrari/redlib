#pragma once

#include "input_keycodes.h"
#include "keyboard_input.h"

static u8 current_modifier = 0;

static u8 special_key = 0;

#ifdef __cplusplus
extern "C" {
#endif

#include "syscalls/syscalls.h"

static inline bool handle_modifier(kbd_event *event){
    switch (event->type) {
        case MOD_PRESS:
            current_modifier |= event->modifier;
            return true;
        case MOD_RELEASE:
            current_modifier &= ~event->modifier;
            return true;
        case KEY_PRESS:
            if (event->key == KEY_CAPSLOCK){
                print("Current special key %x",special_key);
                if (!special_key)
                    special_key = event->key;
                else if (special_key == KEY_CAPSLOCK)
                    special_key = 0;
                return true;
            }
            return false;
        default: return false;
    }
}

static inline bool handle_copy(kbd_event *ev, void (*on_copy)(void *ctx)){
    if (current_modifier & KEY_MOD_LMETA && ev->type == KEY_PRESS && ev->key == KEY_C){
        on_copy(ev);
        return true;
    }
    return false;
}

static inline bool handle_paste(kbd_event *ev, void (*on_paste)(void *ctx)){
    if (current_modifier & KEY_MOD_LMETA && ev->type == KEY_PRESS && ev->key == KEY_V){
        on_paste(ev);
        return true;
    }
    return false;
}

#ifdef __cplusplus
}
#endif