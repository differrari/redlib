#include "helpers.h"
#include "syscalls/syscalls.h"

#ifndef CROSS

void traverse_directory(char *directory, bool recursive, dir_traverse func){
    print("[FS implementation error] Traverse directory not implemented for [REDACTED]");
}

char* get_current_dir(){
    print("[FS implementation error] Current directory not implemented for [REDACTED]");
    return 0;
}

char* gethomedir(){
    print("[FS implementation error] Home directory not implemented for [REDACTED]");
    return 0;
}

char *read_full_file(const char *path, size_t *out_size){
    
    file fd = {};
    if (openf(path, &fd) != FS_RESULT_SUCCESS) return false;
    char *fcontent = (char*)malloc(fd.size + 1);
    
    if (out_size) *out_size = fd.size;
    
    readf(&fd, fcontent, fd.size);
    
    closef(&fd);
    
    return fcontent;
}

bool write_full_file(const char *path, void* buf, size_t size){
    file fd = {};
    if (openf(path, &fd) != FS_RESULT_SUCCESS) return false;
    
    size_t res = writef(&fd, buf, size);
    
    closef(&fd);
    
    return res > 0;
}

#endif