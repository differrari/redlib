#pragma once

#include "types.h"
#include "fs.h"

#define VERSION_NUM(major,minor,patch,build) (uint64_t)((((uint64_t)major) << 48) | (((uint64_t)minor) << 32) | (((uint64_t)patch) << 16) | ((uint64_t)build))

typedef enum { backing_physical, backing_virtual } fs_backing_type;
typedef enum { entry_invalid, entry_file, entry_directory } fs_entry_type;

typedef struct {
    size_t size;
    fs_entry_type type;
} fs_stat;

typedef bool   (*getstat)(const char*, fs_stat*);
typedef size_t (*sread)(const char*, void*, size_t, file_offset*);
typedef size_t (*swrite)(const char*, const void*, size_t);

typedef struct {
    const char* name;
    fs_backing_type backing_type;
    fs_entry_type entry_type;
    getstat stat_func;
    sread read_func;
    swrite write_func;
} fs_entry;

typedef enum { fs_action_invalid, fs_action_stat, fs_action_read, fs_action_write, } fs_action;

typedef struct system_module {
    const char* name;
    const char* mount;
    uint64_t version;

    bool (*init)();
    bool (*fini)();

    FS_RESULT (*open)(const char*, file*);
    size_t (*read)(file*, char*, size_t, file_offset);
    size_t (*write)(file*, const char *, size_t, file_offset);
    void (*close)(file *descriptor);
    
    getstat get_stat;

    sread sread;
    swrite swrite;

    size_t (*readdir)(const char*, void*, size_t, file_offset*);

} system_module;