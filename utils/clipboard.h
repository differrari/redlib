#pragma once

#include "types.h"
#include "files/helpers.h"
#include "syscalls/syscalls.h"
#ifdef CROSS
#include <GLFW/glfw3.h>

extern GLFWwindow* _window;

#endif

static inline void clipboard_copy(void *buf, size_t size){
#ifdef CROSS
    glfwSetClipboardString(_window, buf);
#else
    swritef("/clipboard",buf,size);
#endif
}

static inline void* clipboard_paste(size_t *out_size){
#ifdef CROSS
    glfwGetClipboardString(_window);
#else
    return read_full_file("/clipboard",out_size);
#endif
}