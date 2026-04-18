#pragma once

#include "types.h"
#include "data/struct/hashmap.h"
#include "string/string.h"
#include "data_signatures.h"

#define FD_INVALID 0
#define FD_IN 1
#define FD_OUT 2

//TODO: implement these for multiple writes, but we need a way to do the syncing for follow
typedef enum { 
    cursor_sync_none, //Cursor won't move automatically
    cursor_sync_follow, //External file writes will move the cursor accordingly
    cursor_sync_head, //Cursor will always stay at the beginning of the file
    cursor_sync_tail //Cursor will always stay at the end of the file
} cursor_sync;

typedef struct file {
    u64 id;
    size_t size;
    u64 cursor;
    data_signature data_type;
    // cursor_sync sync_type;
} file;

typedef uint64_t file_offset;

typedef enum SEEK_TYPE {
    SEEK_ABSOLUTE,
    SEEK_RELATIVE
} SEEK_TYPE;

typedef enum FS_RESULT {
    FS_RESULT_DRIVER_ERROR,
    FS_RESULT_SUCCESS,
    FS_RESULT_NOTFOUND,
} FS_RESULT;

#ifdef __cplusplus
extern "C" {
#endif
static inline uint64_t reserve_fd_gid(const char *path){
    return hash_map_fnv1a64(path, strlen(path));
}
#ifdef __cplusplus
}
#endif