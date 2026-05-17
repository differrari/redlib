#pragma once

#include "syscalls/syscalls.h"

typedef enum { env_display_raw, env_display_table, env_display_document } env_display_type;
typedef enum { env_behavior_scroll, env_behavior_wipe } env_behavior;

static inline void environment_sync(){
    print("\[");
}

static inline void switch_environment_display(env_display_type type){
    swritef("/environment/display", &type, sizeof(type));
    environment_sync();
}