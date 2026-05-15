#include "dir_list.h"
#include "string/string.h"
#include "memory/memory.h"

fs_dir_list_helper create_dir_list_helper(void *buf, size_t limit){
    return (fs_dir_list_helper){
        .limit = limit,
        .list = buf,
    };
}

bool dir_list_fill(fs_dir_list_helper *helper, const char *name){
    if (!helper || !helper->list || !helper->limit) return false;
    
    size_t len = strlen(name) + 1;
    
    if (helper->offset + len >= helper->limit) return false;
    
    memcpy((void*)((uptr)helper->list + sizeof(fs_dir_list) + helper->offset), name, len);
    
    helper->offset += len;
    helper->list->count++;
    
    return true;
}

bool stat_dir(fs_stat *out_stat){
    if (!out_stat) return false;
    out_stat->size = 0;
    out_stat->type = entry_directory;
    return true;
}