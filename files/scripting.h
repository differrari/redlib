#pragma once

#include "helpers.h"
#include "shell/sheldon/scripting.h"
#include "syscalls/syscalls.h"

#define READ_ARGS {\
    SHELLEY_ARG_POS(path, false, 0),\
    SHELLEY_ARG_POS(size, true, 1),\
}
SHELLEY_CMD(read, 2, READ_ARGS, {
    SHELLEY_GET_ARG_PARSE(size, size_t, 0, parse_int64);
    SHELLEY_GET_ARG(path);
    char *buf;
    if (size){
        buf = zalloc(size);
        sreadf(path, buf, size);
    } else {
        buf = read_full_file(path, 0);
    }
    shell_print(handle, buf);
    release(buf);
    shell_put(handle, "that's the file bro");
    return exit_return_success;
});

SHELLEY_CMD(pwd, 0, {}, {
    shell_put(handle, "Shell has no directories");
    return exit_implementation_error;
});

static shell_handle *print_handler;

static void print_dir_traverse(const char *directory, const char* name){
    shell_print(print_handler,(char*)name);
}

#define LS_ARGS {\
    SHELLEY_ARG_POS(path, false, 0),\
}
SHELLEY_CMD(ls, 1, LS_ARGS, {
    SHELLEY_GET_ARG(path);
    print_handler = handle;
    traverse_directory(path, false, print_dir_traverse);
    return exit_implementation_error;
});