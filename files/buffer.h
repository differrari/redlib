#pragma once

#include "types.h"
#include "std/args.h"

typedef enum {
    buffer_opt_none = 0,
    buffer_can_grow = 1 << 0,//Once full, expands
    buffer_circular = 1 << 1,//Once full, loops
    buffer_static = 1 << 2,//Always read/write starting from 0, ignoring cursor
} buffer_options;

typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t limit;
    buffer_options options;
    uintptr_t cursor;
} buffer;

buffer buffer_create(size_t size, buffer_options options);
void buffer_write(buffer *buf, char* fmt, ...);
void buffer_write_va(buffer *buf, char* fmt, va_list args);
void buffer_write_const(buffer *buf, char *lit);
void buffer_write_lim(buffer *buf, char *lit, size_t size);
void buffer_write_space(buffer *buf);
void buffer_destroy(buffer *buf);

bool buffer_test();