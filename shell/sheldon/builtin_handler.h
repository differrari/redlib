#pragma once

#include "sheldon.h"
#include "memory/memory.h"

cmd_arg* find_option(string_slice option, cmd_def *def){
    for (int i = 0; i < def->argc; i++){
        if (!def->arguments[i].indicator.use_option) continue;
        if (slices_equal(def->arguments[i].indicator.option, option, false))
            return &def->arguments[i];
    }
    return 0;
}

cmd_arg* find_position_arg(string_slice option, int position, cmd_def *def){
    for (int i = 0; i < def->argc; i++){
        if (def->arguments[i].indicator.use_option) continue;
        if (def->arguments[i].indicator.position == position)
            return &def->arguments[i];
    }
    return 0;
}

void builtin_print_usage(shell_handle *handle, cmd_def *def){
    shell_put(handle, "Usage: %v ", def->name);
    for (int i = 0; i < def->argc; i++){
        if (def->arguments[i].optional)
            shell_put(handle, "[");
        if (def->arguments[i].indicator.use_option)
            shell_put(handle, "%v (", def->arguments[i].indicator.option);
        shell_put(handle, "%v", def->arguments[i].name);
        if (def->arguments[i].indicator.use_option)
            shell_put(handle, ")");
        if (def->arguments[i].optional)
            shell_put(handle, "]");
        shell_put(handle, " ");
    }
    shell_print(handle, "");
}

hash_map_t* argument_parser(shell_handle *handle, cmd_def *def, string_slice arguments){
    string_splitter splitter = make_string_splitter_slice(arguments, ' ', false);
    int count = 0;
    hash_map_t* arg_values = hash_map_create(64);
    while (string_splitter_advance(&splitter)){
        string_slice current_arg = string_splitter_get_current(&splitter);
        if (!current_arg.length) continue;
        if (*current_arg.data == '-'){
            cmd_arg *argument = find_option(current_arg, def);
            if (!argument){
                shell_print(handle, "Unknown option %v",current_arg);
                builtin_print_usage(handle, def);
                hash_map_destroy(arg_values);
                return false;
            }
            if (argument->indicator.use_value){
                string_slice argument_value;
                if (argument->indicator.ignore_spaces){
                    argument_value = string_splitter_remaining(&splitter);
                } else {
                    string_splitter_advance(&splitter);
                    argument_value = string_splitter_get_current(&splitter);
                }
                if (!argument_value.length){
                    shell_print(handle, "No value for argument %v",current_arg);
                    builtin_print_usage(handle, def);
                    hash_map_destroy(arg_values);
                    return false;
                }
                hash_map_put(arg_values, current_arg.data, current_arg.length, string_from_slice(argument_value).data);
            } else 
                hash_map_put(arg_values, current_arg.data, current_arg.length, 0);
            if (argument->indicator.ignore_spaces) break;
        } else {
            cmd_arg *argument = find_position_arg(current_arg, count, def);
            if (!argument){
                shell_print(handle, "Unknown option %v",current_arg);
                builtin_print_usage(handle, def);
                hash_map_destroy(arg_values);
                return false;
            }
            if (argument->indicator.ignore_spaces){
                size_t len = string_splitter_remaining(&splitter).length;
                current_arg.length += len + (len ? 1 : 0);
            }
            hash_map_put(arg_values, argument->name.data, argument->name.length, string_from_slice(current_arg).data);
            if (argument->indicator.ignore_spaces) break;
        }
        count++;
    }
    return arg_values;
}

void register_builtin(shell_handle *handle, char *name, cmd_def *def){
    if (!handle || !handle->local_ctx)
        return;
    hash_map_t *builtins = ((sheldon_ctx*)handle->local_ctx)->builtins;
    if (!builtins)
        return;
    if (def->argc < 0 || def->argc > 64) return;
    size_t full_size = sizeof(cmd_def) + (sizeof(cmd_arg) * def->argc);
    cmd_def *new_def = zalloc(full_size);
    memcpy(new_def, def, full_size);
    hash_map_put_dictionary(builtins, name, new_def);
}

bool call_sheldon_builtin(shell_handle *handle, string_slice cmd, string_slice arg, cmd_returns *out_state){
    if (!handle || !handle->local_ctx) return false;
    hash_map_t *builtins = ((sheldon_ctx*)handle->local_ctx)->builtins;
    if (!builtins) return false;
    cmd_def *definition = hash_map_get(builtins, cmd.data, cmd.length);
    if (!definition || !definition->entry_point) return false;
    hash_map_t *args = argument_parser(handle, definition, arg);
    if (!args)
        return true;
    cmd_returns ret = definition->entry_point(handle, args);
    hash_map_destroy(args);//TODO: eliminate entries
    if (out_state) *out_state = ret;
    return true;
}