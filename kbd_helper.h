#pragma once

#include "input_keycodes.h"
#include "keyboard_input.h"

static u8 current_modifier;

static inline bool handle_modifier(kbd_event *event){
    switch (event->type) {
        case MOD_PRESS:
            current_modifier |= event->modifier;
            return true;
        case MOD_RELEASE:
            current_modifier &= ~event->modifier;
            return true;
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