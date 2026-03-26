#include "helpers.h"
#include "syscalls/syscalls.h"

#ifndef CROSS

void traverse_directory(const char *directory, bool recursive, dir_traverse func){//TODO: not yet capable of getting all data
    if (recursive){
        print("[FS implementation error] recursion is not supported on [REDACTED]");
        return;
    }
    size_t listsize = 0x1000;
    void *listptr = zalloc(listsize);
    uint64_t offset = 0;
    size_t read_size = dir_list(directory, listptr, listsize, &offset);
    (void)read_size;
    string_list *list = (string_list*)listptr;
    if (list){
        char* reader = (char*)list->array;
        for (uint32_t i = 0; i < list->count; i++){
            char *file = reader;
            if (*file){
                func(directory, file);
                while (*reader) reader++;
                reader++;
            }
        }
    }
    release(listptr);
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
    char *fcontent = (char*)zalloc(fd.size + 1);
    
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

void read_lines(char *file, void *ctx, void (*handle_line)(void *ctx, string_slice line)){
    char *point = file;
    do {
        char *new_point = (char*)seek_to(point, '\n');
        if (new_point == point) break;
        int red = 1;
        if (*(new_point-1) == '\r') red++;
        handle_line(ctx, make_string_slice(point, 0, new_point-point-red));
        point = new_point;
    } while(point);
    
}