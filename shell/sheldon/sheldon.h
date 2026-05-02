#pragma once

#include "shell/shell.h"
#include "data/struct/hashmap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { 
    exit_implementation_error = -1,
    exit_return_success = 0,
    exit_generic_error = 1,
} cmd_returns;

typedef cmd_returns (*builtin_entry_point)(shell_handle* ctx, hash_map_t *arguments);

typedef struct {
    string_slice name;
    struct {
        bool use_option;
        int position;
        bool ignore_spaces;
        string_slice option;
        bool use_value;
    } indicator;
    bool optional;
} cmd_arg;

typedef struct {
    builtin_entry_point entry_point;
    string_slice name;
    int argc;
    cmd_arg arguments[];
} cmd_def;

shell_handle* create_sheldon(shell_bindings bindings);

#ifdef __cplusplus
}
#endif