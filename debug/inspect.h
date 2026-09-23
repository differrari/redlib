#pragma once

typedef enum {
    TINSPECT_NONE = 0, 
    TINSPECT_CONTROL = 1 << 1,
    TINSPECT_TRACE = 1 << 2,
    TINSPECT_INFO = 1 << 3,
    TINSPECT_STATE = 1 << 4,
} debug_inspect_types;