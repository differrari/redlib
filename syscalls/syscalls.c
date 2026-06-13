#include "syscalls.h"
#include "string/string.h"
#include "math/math.h"
#include "std/memory.h"
#include "shell/shell.h"

static char log_buf[1024];

int print(const char *fmt, ...){
    __attribute__((aligned(16))) va_list args;
    va_start(args, fmt); 
    memset(log_buf, 0, 1024);
    size_t n = string_format_va_buf(fmt, log_buf, sizeof(log_buf), args);
    va_end(args);
    if (n >= sizeof(log_buf)) log_buf[sizeof(log_buf)-1] = '\0';
    printl(log_buf);
#ifndef CROSS
    file fd2 = { .id = 2 };
    writef(&fd2, log_buf, strlen(log_buf));
    writef(&fd2, "\r\n", 2);
    // current_shell_print(log_buf);
#endif
    return 0;
}

#ifndef CROSS
void put(const char *fmt, ...){
    __attribute__((aligned(16))) va_list args;
    va_start(args, fmt); 
    memset(log_buf, 0, 1024);
    size_t n = string_format_va_buf(fmt, log_buf, sizeof(log_buf), args);
    va_end(args);
    if (n >= sizeof(log_buf)) log_buf[sizeof(log_buf)-1] = '\0';
        file fd2 = { .id = 2 };
        writef(&fd2, log_buf, strlen(log_buf));
        current_shell_put(log_buf);
}
#endif

void seek(file *descriptor, int64_t offset, SEEK_TYPE type){
    uint64_t new_cursor = descriptor->cursor;
    switch (type) {
        case SEEK_ABSOLUTE:
            new_cursor = (uint64_t)offset;
            break;
        case SEEK_RELATIVE:
            new_cursor += offset;
            break;
    }
    if (new_cursor > descriptor->size) return;//TEST: check what happens if we intentionally mess with the descriptor size before changing
    descriptor->cursor = new_cursor;
}

void* realloc_sized(void* old_ptr, size_t old_size, size_t new_size){
    return reallocate(old_ptr, new_size);
}

#ifndef CROSS

void* malloc(size_t size){
    return zalloc(size);
}

extern void free_sized(void *ptr, size_t size){
    return release(ptr);
}

void begin_drawing(draw_ctx *ctx){

}

void destroy_draw_ctx(draw_ctx *ctx){

}

bool should_close_ctx(){
    return false;
}

void* calloc(size_t nitems, size_t size){
    return zalloc(nitems * size);
}

static const char** parse_arguments(char *args, int *count){
    *count = 0;
    const char **argv = (const char**)zalloc(16 * sizeof(uintptr_t));
    char* p = args;

    while (*p && *count < 16){
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        char* start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) { *p = '\0'; p++; }

        argv[*count] = start;
        (*count)++;
    }
    return argv;
}

int system_focus(const char *command, u32 focus_mode){
    const char* args = command;
    while (*args && *args != ' ' && *args != '\t') args++;

    string cmd;
    int argc = 0;
    const char** argv = 0;
    string args_copy = {};

    if (*args == '\0')
        cmd = string_from_literal(command);
    else
        cmd = string_from_literal_length(command, (size_t)(args - command));
    
    const char* argstart = command;
    
    while (*argstart && (*argstart == ' ' || *argstart == '\t')) argstart++;

    args_copy = string_from_literal(argstart);
    argv = parse_arguments(args_copy.data, &argc);
    int res = exec(cmd.data, argc, argv, focus_mode);
    string_free(args_copy);
    string_free(cmd);
    if (argv) release((void*)argv);
    return res;
}

int system(const char *cmd){
    return system_focus(cmd, EXEC_MODE_DEFAULT);
}

#endif
