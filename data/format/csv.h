#pragma once

#include "string/slice.h"

typedef void (*csv_handler)(string_slice value, void *ctx);

void read_csv(string_slice slice, csv_handler on_val, void *ctx);