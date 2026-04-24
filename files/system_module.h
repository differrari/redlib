#pragma once

#include "types.h"
#include "fs.h"
#include "files/buffer.h"
#include "data_signatures.h"

typedef enum { backing_physical, backing_virtual, backing_command, backing_transform } fs_backing_type;
typedef enum { entry_invalid, entry_file, entry_directory } fs_entry_type;

typedef struct {
    size_t size;
    fs_entry_type type;
    data_signature data_type;
} fs_stat;

typedef FS_RESULT (*file_open_fn)(const char*, file*);
typedef size_t (*file_read_fn)(file*, char*, size_t, file_offset);
typedef size_t (*file_write_fn)(file*, const char *, size_t, file_offset);
typedef void (*file_close_fn)(file *descriptor);
typedef bool   (*file_getstat_fn)(const char*, fs_stat*);
typedef size_t (*file_readdir_fn)(const char*, void*, size_t, file_offset*);
typedef bool (*file_truncate_fn)(file*, size_t);

#define VERSION_NUM(major,minor,patch,build) (uint64_t)((((uint64_t)major) << 48) | (((uint64_t)minor) << 32) | (((uint64_t)patch) << 16) | ((uint64_t)build))

typedef u64 (*cmd_fn)(void* ctx, size_t len);

typedef struct {
    file_open_fn open;
    file_read_fn read;
    file_write_fn write;
    file_close_fn close;
    file_truncate_fn truncate;
    file_getstat_fn getstat;
    file_readdir_fn readdir;
} file_actions;

typedef struct module_file {
    string name;
    fs_backing_type backing_type;
    fs_entry_type entry_type;
    uint64_t fid;
    uint64_t serial;
    uptr buf;//DEPRECATED
    void *private_data;
    bool ignore_cursor;//DEPRECATED
    bool read_only;//DEPRECATED
    data_signature data_type;
    size_t file_size;//DEPRECATED
    buffer file_buffer;
    uint64_t references;
    file_actions actions;
} module_file;

typedef struct system_module {
    const char* name;
    const char* mount;
    uint64_t version;

    bool (*init)();
    bool (*fini)();

    file_open_fn open;
    file_read_fn read;
    file_write_fn write;
    file_close_fn close;
    
    file_truncate_fn truncate;
    
    file_getstat_fn getstat;

    file_readdir_fn readdir;

} system_module;