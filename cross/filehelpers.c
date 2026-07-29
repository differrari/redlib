#ifdef CROSS

#include "files/helpers.h"
#include <unistd.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <stdlib.h>
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include "string/string.h"
#include "syscalls/syscalls.h"
#include <stdio.h>
#include "memory/memory.h"

static char cwd[128];
static char *homedir; 

#ifndef DT_DIR
#define DT_DIR 4
#endif

extern size_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

void traverse_directory(const char *directory, bool recursive, dir_traverse func){
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory)) == 0) {
        print("Failed to open path %s",directory);
        return;
    }
    
    while ((ent = readdir (dir)) != 0) {
        if (recursive && ent->d_type == DT_DIR && !strstart(ent->d_name, ".")){
            string s = string_format("%s/%s",directory,ent->d_name);
            traverse_directory(s.data, true, func);
            string_free(s);
        } else func(directory, ent->d_name);
    }
    closedir (dir);
}

char* get_current_dir(){
    if (!strlen(cwd)) getcwd(cwd, sizeof(cwd));
    return cwd;
}

char* gethomedir(){
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}

size_t sreadf(const char* path, void* buf, size_t size){
    FILE *fd = fopen(path,"r");
    if (!fd) return 0;
    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    rewind(fd);
    if (fsize < size) size = fsize;
    fread(buf, size, 1, fd);

    fclose(fd);
}

size_t swritef(const char* path, const void* buf, size_t size, bool append){
    FILE *fd = fopen(path,"w");
    if (!fd) return 0;
    if (append) fseek(fd, 0, SEEK_END);
    fwrite(buf, size, 1, fd);
    fclose(fd);
}

buffer get_input_line(){
    buffer buf = {};
    char *s = 0;
    size_t l = 0;
    if (getline(&s,&l, stdin) < 0){
        free(s);
        return buf;
    }
    if (!s) return buf;
    buf = buffer_create(l, buffer_read_only);
    memcpy(buf.buffer, s, l);
    buf.buffer_size = l;
    free(s);
    print("reffub %s",buf.buffer);
    return buf;
}

#endif