#pragma once

#include "types.h"
#include "mouse_input.h"

typedef enum { click_none, click_start, click_continue, click_release } click_status_t;
#ifndef MOUSE_HELPER_NONSTATIC
static click_status_t click_status;
#else
extern click_status_t click_status;
#endif

static inline void process_mouse(mouse_data data){
    if (mouse_button_down(&data, LMB)){
        if (click_status == click_none) click_status = click_start;
        else if (click_status == click_start) click_status = click_continue;
    } else {
        if (click_status == click_release) click_status = click_none;
        if (click_status != click_none) click_status = click_release;
    }
}

static inline bool mouse_down(){
    return click_status == click_start || click_status == click_continue;
}

static inline bool mouse_up(){
    return !mouse_down();
}