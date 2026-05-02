#pragma once

#include "types.h"
#include "string/slice.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMMAND(name, ...) //const char *function_name_##name = #name; 
#define ALIAS(new_name, old_name)
#define REQUIRE(argument, type, shorthand) 
#define OPTIONAL(argument, type, shorthand, default)
#define HARDCODED(argument, type, shorthand, default)

typedef void (*dir_traverse)(const char *directory, const char* name);

void traverse_directory(const char *directory, bool recursive, dir_traverse func);

char* get_current_dir();
COMMAND(get_current_dir);
ALIAS(pwd,get_current_dir);

char* gethomedir();

char* read_full_file(const char *path, size_t *out_size);
COMMAND(read_full_file, "read", REQUIRE(path, char, ""), HARDCODED(out_size, size_t, "", 0));

bool write_full_file(const char *path, void* buf, size_t size);

void read_lines(char *file, void *ctx, void (*handle_line)(void *ctx, string_slice line));
#ifdef __cplusplus
}
#endif