#pragma once

#include "types.h"
#include "files/helpers.h"
#include "syscalls/syscalls.h"
#include "files/data_signatures.h"
#ifdef CROSS
#include <GLFW/glfw3.h>

extern GLFWwindow* _window;

#endif

static inline void clipboard_copy(void *buf, size_t size, data_signature type){
#ifdef CROSS
    if (type != DATA_SIG_UNKNOWN && type != DATA_SIG_TEXT) return;
    glfwSetClipboardString(_window, buf);
#else
    file fd = {};
    if (openf("/clipboard", &fd) != FS_RESULT_SUCCESS) return;
    fd.data_type = type;
    
    writef(&fd, (char*)buf, size);
    
    closef(&fd);
#endif
}

static inline void* clipboard_paste(data_signature expected_sig, size_t *out_size){
#ifdef CROSS
    if (expected_sig != DATA_SIG_UNKNOWN && expected_sig != DATA_SIG_TEXT) return 0;
    return glfwGetClipboardString(_window);
#else
    file fd = {};
    if (openf("/clipboard", &fd) != FS_RESULT_SUCCESS) return 0;
    if (fd.data_type != 0 && fd.data_type != expected_sig) return 0;
    char *fcontent = (char*)zalloc(fd.size + 1);
    
    if (out_size) *out_size = fd.size;
    
    readf(&fd, fcontent, fd.size);
    
    closef(&fd);
    
    return fcontent;
#endif
}