#pragma once

#include "sheldon.h"
#include "data/struct/hashmap.h"
#include "syscalls/syscalls.h"
#include "files/helpers.h"

static hash_map_t *sheldon_builtin_hashmap;

static cmd_returns echo(shell_handle *handle, string_slice arguments){
    shell_put(handle, "%v", arguments);
    return exit_return_success;
}

static cmd_returns pwd(shell_handle *handle, string_slice arguments){
    shell_put(handle, "Shell has no directories");
    return exit_implementation_error;
}

static cmd_returns read(shell_handle *handle, string_slice arguments){
    string_splitter splitter = make_string_splitter_slice(arguments, ' ', false);
    if (!string_splitter_advance(&splitter))
        return false;
    string_slice path = string_splitter_get_current(&splitter);
    size_t exp_size = 0;
    if (string_splitter_advance(&splitter)){
        string_slice size_str = string_splitter_get_current(&splitter);
        exp_size = parse_int64(size_str.data, size_str.length);
    }
    string path_string = string_from_slice(path);
    char *buf;
    if (exp_size){
        buf = zalloc(exp_size);
        sreadf(path_string.data, buf, exp_size);
    } else {
        buf = read_full_file(path_string.data, 0);
    }
    shell_print(handle, buf);
    release(buf);
    string_free(path_string);
    shell_put(handle, "that's the file bro");
    return exit_return_success;
}

#define REG_BUILTIN(fn) hash_map_put_dictionary(sheldon_builtin_hashmap,#fn,fn);

void register_sheldon_builtins(){
    sheldon_builtin_hashmap = hash_map_create(256);
    REG_BUILTIN(echo);
    REG_BUILTIN(pwd);
    REG_BUILTIN(read);
}

bool call_sheldon_builtin(shell_handle *handle, string_slice cmd, string_slice arg, cmd_returns *out_state){
    if (!sheldon_builtin_hashmap) return false;
    builtin_entry_point ep = hash_map_get(sheldon_builtin_hashmap, cmd.data, cmd.length);
    if (!ep) return false;
    cmd_returns ret = ep(handle, arg);
    if (out_state) *out_state = ret;
    return true;
}