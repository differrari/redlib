#pragma once

#include "types.h"
#include "files/buffer.h"

typedef struct shell_handle shell_handle;

typedef struct {
    void (*console_output)(shell_handle*, string_slice output);
    // void (*fn2)(kbd_event event);
} shell_bindings;
 
typedef struct shell_handle {
    buffer out_buffer;
    shell_bindings bindings;
    void *ctx;
    bool (*cmd_input)(shell_handle*, string_slice input);
} shell_handle;

#ifdef __cplusplus
extern "C" {
#endif
void new_shell(shell_handle *, shell_bindings bindings, void (*init_shell)(shell_handle *handle));
void register_func();
bool run_cmd(shell_handle *, string_slice);
#ifdef __cplusplus
}
#endif

static inline void shell_print_specify_newline(shell_handle *handle, bool newline, char *fmt, ...){
    if (!handle || !handle->out_buffer.buffer) return;
    __attribute__((aligned(16))) va_list args;
    va_start(args, fmt); 
    uptr cursor = handle->out_buffer.cursor;
    size_t size = buffer_write_va(&handle->out_buffer, fmt, args);
    if (newline) size += buffer_write_const(&handle->out_buffer, "\n");
    va_end(args);
    if (!handle->bindings.console_output) return;
    handle->bindings.console_output(handle, (string_slice){ .data = (char*)((uptr)handle->out_buffer.buffer + cursor), .length = size});
}

#define shell_print(handle, fmt, ...) shell_print_specify_newline(handle, true, fmt, ##__VA_ARGS__)
#define shell_put(handle, fmt, ...) shell_print_specify_newline(handle, false, fmt, ##__VA_ARGS__)