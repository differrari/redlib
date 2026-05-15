#include "shell.h"
#include "alloc/allocate.h"
#include "syscalls/syscalls.h"

shell_handle* current_shell;

void new_shell(shell_handle *handle, shell_bindings bindings, void (*init_shell)(shell_handle *handle)){
    if (!handle || !handle->common_ctx) return;
    handle->out_buffer = buffer_create(0x10000, buffer_can_grow);
    handle->bindings = bindings;
    handle->common_ctx->current_directory = string_from_literal("/home");
    if (init_shell) init_shell(handle);
}

bool run_cmd(shell_handle *handle, string_slice cmd){
    if (!handle->cmd_input) return false;
    return handle->cmd_input(handle,cmd);
}
