#include "binary_scanner.h"
#include "memory/memory.h"
#include "syscalls/syscalls.h"

bool bin_scan_ok(binary_scanner *scanner, size_t op_len){
    return !scanner || !scanner->data || scanner->size >= scanner->cursor + op_len;
}

bool bin_scan_size(binary_scanner *scanner, size_t size, void *out_val){
    if (!out_val) return false;
    if (!bin_scan_ok(scanner, size)) return false;
    memcpy(out_val,(void*)(scanner->data + scanner->cursor),size);
    scanner->cursor += size;
    return true;
}

bool bin_scan_size_buf(binary_scanner *scanner, size_t size, buffer *buf){
    if (!buf || !buf->buffer || buf->options & buffer_read_only) return false;
    if (!bin_scan_ok(scanner, size)) return false;
    if (buffer_write_lim(buf, (void*)(scanner->data + scanner->cursor), size) != size) return false;
    scanner->cursor += size;
    return true;
}

static inline i8 bswap8(i8 v){
    return v;
}

#define bin_scan_type(_type,_bits) bool bin_scan_##_type(binary_scanner *scanner, _type *out_val){\
    bool res = bin_scan_size(scanner, sizeof(_type), out_val);\
    if (!res) return res;\
    if (scanner->swap_endian) *out_val = bswap##_bits(*out_val);\
    return true;\
}

bin_scan_type(i8,8)
bin_scan_type(i16,16)
bin_scan_type(i32,16)
bin_scan_type(i64,64)
bin_scan_type(float,32)
bin_scan_type(double,64)

bool bin_scan_string(binary_scanner *scanner, string *out_val){
    if (!out_val) return false;
    i32 signature = 0;
    if (!bin_scan_i32(scanner, &signature))
        return false;
    i32 size = 0;
    if (!bin_scan_i32(scanner, &size)) return false;
    if (!size) return true;
    if (!bin_scan_ok(scanner, size))
        return false;
    char *string_start = (char*)(scanner->data + scanner->cursor);
    scanner->cursor += size;
    *out_val = string_from_literal_length(string_start, size);
    return true;
}

bool bin_scan_skip(binary_scanner *scanner, size_t amount){
    if (!amount) return true;
    if (scanner->cursor + amount >= scanner->size) return false;
    scanner->cursor += amount;
    return true;
}