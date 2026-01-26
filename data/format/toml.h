#pragma once

#include "types.h"
#include "string/slice.h"

typedef void (*toml_handler)(string_slice key, string_slice value, void *context);

void read_toml(char *info, toml_handler on_kvp, void *context);