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

#define bin_scan_type(_type) bool bin_scan_##_type(binary_scanner *scanner, _type *out_val){\
    return bin_scan_size(scanner, sizeof(_type), out_val);\
}

bin_scan_type(i8)
bin_scan_type(i16)
bin_scan_type(i32)
bin_scan_type(i64)
bin_scan_type(float)
bin_scan_type(double)

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
