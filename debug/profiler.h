#pragma once

#include "types.h"
#include "syscalls/syscalls.h"

static u64 profiler_time;

static inline void profiler_init(){
    profiler_time = get_time();
}

static inline u64 profiler_delta(){
    if (!profiler_time){
        profiler_init();
        return 0;
    }
    u64 new_t = get_time();
    u64 delta = new_t - profiler_time;
    profiler_time = new_t;
    return delta;
}