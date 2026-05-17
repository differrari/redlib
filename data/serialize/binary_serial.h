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

binary_serializer make_binary_serializer(char *structure, size_t length);

//Array of: 32 bits for binary_types, string for name
void bin_ser_define_structure(binary_serializer *serializer, char *structure, size_t length);

buffer bin_ser_serialize(binary_serializer *serializer, void* data, size_t length, size_t count);

sizedptr bin_ser_emit_structure(structdef *items, size_t amount);

hash_map_t* bin_ser_deserialize(binary_serializer *serializer, string_slice data);