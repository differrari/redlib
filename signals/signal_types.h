#pragma once

#include "types.h"

typedef enum {
    SIG_NONE,
    
    SIG_KILL,
    SIG_QUIT,
    
    SIG_STOP,
    SIG_CONT,

    SIG_KBD,
    SIG_MOUSE,

    NUMBER_SIGNALS
} signal_types;

typedef struct {
    signal_types type;
    u16 sender;
    i64 value;
} signal_info_t;

typedef bool (*signal_handler)(signal_info_t *signal);

static inline bool can_signal_be_handled(signal_types type){
    return 
        type != SIG_KILL &&
        type != SIG_STOP &&
        type != SIG_CONT &&
        true;
}

static inline bool signal_is_immediate(signal_types type){
    return !can_signal_be_handled(type);
}