#pragma once

#include "types.h"

typedef void (*dir_traverse)(const char *directory, const char* name);


void traverse_directory(const char *directory, bool recursive, dir_traverse func);

char* get_current_dir();

char* gethomedir();

char *read_full_file(const char *path, size_t *out_size);

bool write_full_file(const char *path, void* buf, size_t size);