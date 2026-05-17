#include "binary_serial.h"
#include "binary_scanner.h"
#include "syscalls/syscalls.h"
#include "memory/memory.h"

binary_serializer make_binary_serializer(char *structure, size_t length){
    binary_serializer ser = {};
    bin_ser_define_structure(&ser,structure, length);
    return ser;
}

void bin_ser_define_structure(binary_serializer *serializer, char *structure, size_t length){
    if (serializer->structure){
        release(serializer->structure);
        serializer->field_count = 0;
    }
    arr_stack_t *stack = stack_create(sizeof(structdef), 3);

    binary_scanner scanner = bin_scan_create((u8*)structure, length);

    bool success = true;
    while (true){
        i32 struct_type = 0;
        if (!bin_scan_i32(&scanner, &struct_type)) break;
        if (!struct_type){ success = false; break; }
        string name = {};
        if (!bin_scan_string(&scanner, &name) || !name.length){ success = false; break; }
        stack_push(stack, &(structdef){
            .name = name,
            .type = struct_type
        });
    }

    if (success){
        serializer->field_count = stack_count(stack);
        serializer->structure = zalloc(sizeof(structdef) * serializer->field_count);
        size_t chunks = serializer->field_count/stack->chunk_capacity;
        size_t remainded = serializer->field_count % stack->chunk_capacity;
        if (remainded) chunks++;
        arr_stack_t *next_stack = stack;
        for (size_t i = 0; i < chunks; i++){
            size_t amount = remainded && i == chunks-1 ? remainded : stack->chunk_capacity;
            memcpy((void*)((uptr)serializer->structure + (i * stack->chunk_capacity * sizeof(structdef))), stack_get(next_stack,0), amount * sizeof(structdef));
            next_stack = stack->next;
            if (!next_stack) break;
        }
    }
    stack_destroy(stack);
}

size_t bin_ser_calc_structure_size(structdef *items, size_t amount){
    size_t total_size = 0;
    for (u64 i = 0; i < amount; i++){
        total_size += items[i].name.length + sizeof(i64);
        total_size += sizeof(binary_types);
    }
    return total_size;
}

#define BIN_SER_SET(buf, value, type) *(type*)buf = value; buf += sizeof(type);
#define BIN_SER_CPY(buf, value, size) memcpy(buf, value, size); buf += size;

sizedptr bin_ser_emit_structure(structdef *items, size_t amount){
    size_t len = bin_ser_calc_structure_size(items,amount);
    char *buf = zalloc(len);
    char *bufstart = buf;
    for (size_t i = 0; i < amount; i++){
        BIN_SER_SET(buf, items[i].type, binary_types);
        BIN_SER_SET(buf, 0, i32);
        BIN_SER_SET(buf, items[i].name.length, i32);
        BIN_SER_CPY(buf, items[i].name.data, items[i].name.length);
    }
    return (sizedptr){.ptr = (uptr)bufstart, .size = len};
}

size_t bin_ser_calc_data_size(structdef *items, size_t amount){
    size_t total_size = 0;
    for (u64 i = 0; i < amount; i++){
        
    }
    return total_size;
}

//TODO: alignment
buffer bin_ser_serialize(binary_serializer *serializer, void* data, size_t length, size_t count){
    buffer buf = buffer_create(length * count, buffer_can_grow);
    for (size_t item = 0; item < count; item++){
        for (size_t field = 0; field < serializer->field_count; field++){
            if (serializer->structure[field].type == binary_type_string){
                int reserved = 0;
                buffer_write_lim(&buf, (char*)&reserved, sizeof(i32));
                string_slice slice;
                memcpy(&slice, data, sizeof(string_slice));
                buffer_write_lim(&buf, (char*)&slice.length, sizeof(i32));
                buffer_write_lim(&buf, slice.data, slice.length);
                data += sizeof(string_slice);
            } else {
                size_t field_size = get_bin_type_size(serializer->structure[field].type);
                buffer_write_lim(&buf, data, field_size);
                data += field_size;
            }
        }
    }
    return buf;
}

hash_map_t* bin_ser_deserialize(binary_serializer *serializer, string_slice data){
    
}