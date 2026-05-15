#pragma once

#include "types.h"
#include "system_module.h"

typedef struct {
    u32 count;
    //Followed by null-terminated strings
} fs_dir_list;

typedef struct {
    u32 offset;
    size_t limit;
    fs_dir_list *list;
} fs_dir_list_helper;

#ifdef __cplusplus
extern "C" {
#endif

fs_dir_list_helper create_dir_list_helper(void* buf,size_t limit);
bool dir_list_fill(fs_dir_list_helper *helper, const char *name);

bool stat_dir(fs_stat *out_stat);

static inline size_t dir_buf_size(fs_dir_list_helper *helper){
    return helper->offset + sizeof(fs_dir_list);
}

#ifdef __cplusplus
}
#endif