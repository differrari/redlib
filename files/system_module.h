#pragma once

#include "types.h"
#include "fs.h"

typedef enum { entry_invalid, entry_file, entry_directory } fs_entry_type;

typedef struct {
    size_t size;
    fs_entry_type type;
} fs_stat;

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

    size_t (*sread)(const char*, void*, size_t);
    size_t (*swrite)(const char*, const void*, size_t);
    
    bool   (*getstat)(const char*, fs_stat*);

    size_t (*readdir)(const char*, void*, size_t, file_offset*);

} system_module;