#pragma once

#include "types.h"

void gc_register_alloc(void* ptr);

void gc_run(void* root);

bool gc_run_next();