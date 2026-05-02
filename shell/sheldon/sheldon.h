#pragma once

#include "shell/shell.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { 
    exit_implementation_error = -1,
    exit_return_success = 0,
    exit_generic_error = 1,
} cmd_returns;

typedef cmd_returns (*builtin_entry_point)(shell_handle* ctx, string_slice args);

typedef struct {
    string_slice name;
    cmd_returns (*entry_point)(shell_handle* ctx, string_slice args);
} sheldon_builtin;

shell_handle* create_sheldon(shell_bindings bindings);
#ifdef __cplusplus
}
#endif