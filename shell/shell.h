#pragma once

#include "types.h"
#include "files/buffer.h"

typedef struct shell_handle shell_handle;

typedef enum  {
    console_ctrl_none,
    console_ctrl_close
} console_ctrls;

typedef struct {
    void (*console_output)(shell_handle*, char output);
    void (*console_flush)(shell_handle*);
    void (*console_clean)(shell_handle*);
    void (*console_bell)(shell_handle*);
    void (*console_ascii_cmd)(shell_handle*, char cmd, u16 proc_id);
    void (*console_control)(shell_handle*, console_ctrls);
} shell_bindings;

typedef struct {
    string current_directory;
} shell_ctx;
 
struct shell_handle {
    buffer out_buffer;
    shell_bindings bindings;
    shell_ctx *common_ctx;
    void *local_ctx;
    bool (*cmd_input)(shell_handle*, string_slice input);
};

#ifdef __cplusplus
extern "C" {
#endif
extern shell_handle* current_shell;

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
    for (u32 i = 0; i < size; i++)
        handle->bindings.console_output(handle, ((char*)handle->out_buffer.buffer)[cursor + i]);
    if (handle->bindings.console_flush) handle->bindings.console_flush(handle);
}

static inline void current_shell_print(char *lit){
    if (current_shell) shell_print_specify_newline(current_shell, true, lit);
}

static inline void current_shell_put(char *lit){
    if (current_shell) shell_print_specify_newline(current_shell, false, lit);
}

#define shell_print(handle, fmt, ...) shell_print_specify_newline(handle, true, fmt, ##__VA_ARGS__)
#define shell_put(handle, fmt, ...) shell_print_specify_newline(handle, false, fmt, ##__VA_ARGS__)
