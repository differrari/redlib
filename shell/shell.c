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

bool shell_interpret(shell_handle *handle, string_slice cmd){
    if (handle->scripting.eval && (!handle->scripting.is_script || handle->scripting.is_script(cmd)))
        if (handle->scripting.eval(cmd, handle->scripting.ctx)) return true;
    return run_cmd(handle, cmd);
}

bool run_cmd(shell_handle *handle, string_slice cmd){
    if (!handle->cmd_input) return false;
    return handle->cmd_input(handle,cmd);
}
