#pragma once

#include "types.h"
#include "fs.h"
#include "files/buffer.h"
#include "data_signatures.h"

typedef enum { backing_physical, backing_virtual, backing_command } fs_backing_type;
typedef enum { entry_invalid, entry_file, entry_directory } fs_entry_type;

#define VERSION_NUM(major,minor,patch,build) (uint64_t)((((uint64_t)major) << 48) | (((uint64_t)minor) << 32) | (((uint64_t)patch) << 16) | ((uint64_t)build))

typedef u64 (*cmd_fn)(void* ctx, size_t len);

typedef struct module_file {
    string name;
    fs_backing_type backing_type;
    fs_entry_type entry_type;
    uint64_t fid;
    uint64_t serial;
    uptr buf;
    void *private_data;
    bool ignore_cursor;
    bool read_only;
    data_signature data_type;
    size_t file_size;
    buffer file_buffer;
    uint64_t references;
    cmd_fn function;
} module_file;

typedef struct {
    size_t size;
    fs_entry_type type;
    data_signature data_type;
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
    
    bool (*truncate)(file*, size_t);
    
    bool   (*getstat)(const char*, fs_stat*);

    size_t (*readdir)(const char*, void*, size_t, file_offset*);

} system_module;