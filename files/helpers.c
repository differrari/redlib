#include "helpers.h"
#include "syscalls/syscalls.h"

#ifndef CROSS

void traverse_directory(const char *directory, bool recursive, dir_traverse func){
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
                if (recursive && !strstart(file,".")){
                    string s = string_format("%s/%s",directory,file);
                    fs_stat fsstat = {};
                    statf(s.data, &fsstat);
                    if (fsstat.type == entry_directory)
                        traverse_directory(s.data, recursive, func);
                    string_free(s);
                }
                while (*reader) reader++;
                reader++;
            }
        }
    }
    release(listptr);
}

char* get_current_dir(){
    if (current_shell && current_shell->common_ctx)
        return current_shell->common_ctx->current_directory.data;
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
    
    truncatef(&fd, size);
    
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

void replace_directory(shell_ctx *ctx, string s){
    string_free(ctx->current_directory);
    ctx->current_directory = s;
    print("New directory now %S",ctx->current_directory);
}

void append_directory(shell_ctx *ctx, string s){
    replace_directory(ctx, string_concat(ctx->current_directory, s));
}

void change_current_dir(const char *path){
    if (!current_shell || !current_shell->common_ctx) return;
    if (strlen(path) && *path == '/'){
        replace_directory(current_shell->common_ctx, string_from_literal(path));
        return;
    }
    if (!strlen(path) || *path == '~'){
        replace_directory(current_shell->common_ctx, string_from_literal("/home"));
        if (strlen(path) > 1){
            path++;
            string tmp = string_from_literal(path);
            append_directory(current_shell->common_ctx, tmp);
            string_free(tmp);
        }
        return;
    }
    print("Appending %S/%s",current_shell->common_ctx->current_directory,path);
    string tmp = string_format("/%s",path);
    append_directory(current_shell->common_ctx, tmp);
    string_free(tmp);
}