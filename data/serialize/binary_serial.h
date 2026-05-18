#pragma once

#include "string/slice.h"
#include "data/struct/stack.h"
#include "data/struct/hashmap.h"
#include "binary_scanner.h"
#include "files/buffer.h"

typedef struct {
    string name;
    binary_types type;
} structdef;

typedef struct {
    structdef* structure;
    size_t field_count;
} binary_serializer;

static inline binary_serializer make_binary_serializer(structdef *structure, size_t count){
    return (binary_serializer){
        .structure = structure,
        .field_count = count
    };
}

#ifdef __cplusplus
extern "C" {
#endif
//Array of: 32 bits for binary_types, string for name
bool bin_ser_define_structure(binary_serializer *serializer, char *structure, size_t length);

buffer bin_ser_serialize(binary_serializer *serializer, void* data, size_t length, size_t count);

sizedptr bin_ser_emit_structure(structdef *items, size_t amount);

bool bin_ser_deserialize(binary_serializer *serializer, sizedptr data, void (*on_read)(structdef field, sizedptr data, bool is_allocated));
#ifdef __cplusplus
}
#endif