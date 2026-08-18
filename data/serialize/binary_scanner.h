#pragma once

#include "types.h"
#include "string/slice.h"
#include "files/buffer.h"

typedef enum {
    binary_type_invalid,
    binary_type_i8,
    binary_type_i16,
    binary_type_i32,
    binary_type_i64,
    binary_type_float,
    binary_type_double,
    binary_type_string,//32 bit reserved, 32 bits for size, then characters
} binary_types;

static inline size_t get_bin_type_size(binary_types type){
    switch (type) {
        case binary_type_invalid: return 0;
        case binary_type_i8: return sizeof(i8);
        case binary_type_i16: return sizeof(i16);
        case binary_type_i32: return sizeof(i32);
        case binary_type_i64: return sizeof(i64);
        case binary_type_float: return sizeof(float);
        case binary_type_double: return sizeof(double);
        case binary_type_string: return sizeof(string_slice);
    }
    return 0;
}

static inline size_t get_bin_type_align(binary_types type){
    switch (type) {
        case binary_type_invalid: return 0;
        case binary_type_i8: return sizeof(i8);
        case binary_type_i16: return sizeof(i16);
        case binary_type_i32: return sizeof(i32);
        case binary_type_i64: return sizeof(i64);
        case binary_type_float: return sizeof(float);
        case binary_type_double: return sizeof(double);
        case binary_type_string: return sizeof(string);
    }
    return 0;
}

typedef struct {
    u8 *data;
    size_t size;
    uptr cursor;
    bool swap_endian;
} binary_scanner;

static inline binary_scanner bin_scan_create(u8* data, size_t size){
    return (binary_scanner){
        .data = data,
        .size = size,
        .cursor = 0
    };
}

bool bin_scan_size(binary_scanner *scanner, size_t size, void *out_val);
bool bin_scan_size_buf(binary_scanner *scanner, size_t size, buffer *buf);

bool bin_scan_i8(binary_scanner *scanner, i8 *out_val);
static inline bool bin_scan_u8(binary_scanner *scanner, u8 *out_val){
    return bin_scan_i8(scanner, (i8*)out_val);
}

bool bin_scan_i16(binary_scanner *scanner, i16 *out_val);
static inline bool bin_scan_u16(binary_scanner *scanner, u16 *out_val){
    return bin_scan_i16(scanner, (i16*)out_val);
}

bool bin_scan_i32(binary_scanner *scanner, i32 *out_val);
static inline bool bin_scan_u32(binary_scanner *scanner, u32 *out_val){
    return bin_scan_i32(scanner, (i32*)out_val);
}

bool bin_scan_i64(binary_scanner *scanner, i64 *out_val);
static inline bool bin_scan_u64(binary_scanner *scanner, u64 *out_val){
    return bin_scan_i64(scanner, (i64*)out_val);
}

bool bin_scan_float(binary_scanner *scanner, float *out_val);
bool bin_scan_double(binary_scanner *scanner, double *out_val);
bool bin_scan_string(binary_scanner *scanner, string *out_val);
bool bin_scan_skip(binary_scanner *scanner, size_t amount);