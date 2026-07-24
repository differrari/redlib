#pragma once

#include "sheldon.h"
#include "memory/memory.h"

cmd_arg* find_option(string_slice option, cmd_def *def);
cmd_arg* find_position_arg(string_slice option, int position, cmd_def *def);
void builtin_print_usage(shell_handle *handle, cmd_def *def);
hash_map_t* argument_parser(shell_handle *handle, cmd_def *def, string_slice arguments);
void register_builtin(shell_handle *handle, char *name, cmd_def *def);
bool call_sheldon_builtin(shell_handle *handle, string_slice cmd, string_slice arg, cmd_returns *out_state);