#pragma once

typedef enum {
    TINSPECT_NONE = 0, 
    TINSPECT_CONTROL = 1 << 1,
    TINSPECT_TRACE = 1 << 2,
    TINSPECT_INFO = 1 << 3,
    TINSPECT_STATE = 1 << 4,
    TINSPECT_INPUT = 1 << 5,
    TINSPECT_OUTPUT = 1 << 6
} debug_inspect_types;