#pragma once

#include "helpers.h"
#include "shell/sheldon/scripting.h"
#include "syscalls/syscalls.h"

SHELLEY_CMD_PRINT(pwd, get_current_dir);
SHELLEY_CMD_FWD_1ARG(cd, change_current_dir, path);

SHELLEY_CMD(read, 
{
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
    return exit_return_success;
}, 
SHELLEY_ARG_POS(path, false, 0),
SHELLEY_ARG_POS(size, true, 1)
);

static void print_dir_traverse(const char *directory, const char* name){
    print((char*)name);
}

SHELLEY_CMD(ls, {
    SHELLEY_GET_ARG_OR_DEFAULT(path, handle->common_ctx->current_directory.data);
    traverse_directory(path, false, print_dir_traverse);
    return exit_return_success;
}, SHELLEY_ARG_POS(path, true, 0));