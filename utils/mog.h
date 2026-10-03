#pragma once

typedef enum {
    MOG_ERROR,
    MOG_IMPLEMENTATION_ERROR,
    MOG_WARNING,
    MOG_INFO,
    MOG_VERBOSE,
    MOG_ALL
} MOG_LEVELS;

#define mog(level, fmt, ...) if (MOG_##level <= MOG_LEVEL) print("[" MOG_MODULE " " #level "] " fmt, ##__VA_ARGS__);