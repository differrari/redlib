#ifdef CROSS

#include "files/helpers.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include "string/string.h"
#include "syscalls/syscalls.h"

static char cwd[128];
static char *homedir; 

#ifndef DT_DIR
#define DT_DIR 4
#endif

void traverse_directory(const char *directory, bool recursive, dir_traverse func){
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory)) == 0) {
        printf("Failed to open path %s",directory);
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

#endif