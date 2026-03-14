#pragma once

#include "data/struct/stack.h"
#include "files/dir_list.h"
#include "data/struct/hashmap.h"
#include "data_signatures.h"

#define DIR_AS_FILE "#"

static arr_stack_t *entries;

static u64 (*hashing_func)(const char *path);

static inline module_file* make_entry(const char *name, fs_backing_type back_type, fs_entry_type ent_type, data_signature data_type, buffer buf){
    if (!entries) entries = stack_create(sizeof(module_file),32);
    stack_push(entries,&(module_file){
        .name = string_from_literal(name),
        .backing_type = back_type,
        .entry_type = ent_type,
        .file_buffer = buf,
        .references = 0,
        .read_only = false,
        .data_type = data_type,
        .fid = hashing_func ? hashing_func(name): hash_map_fnv1a64(name, strlen(name)),
        .serial = hashing_func ? hashing_func(name): hash_map_fnv1a64(name, strlen(name)),
    });
    return stack_get(entries, stack_count(entries)-1);
}

static inline bool make_cmd_entry(const char *name, fs_backing_type back_type, fs_entry_type ent_type, cmd_fn func){
    if (!entries) entries = stack_create(sizeof(module_file),32);
    stack_push(entries,&(module_file){
        .name = string_from_literal(name),
        .backing_type = back_type,
        .entry_type = ent_type,
        .function = func,
        .references = 0,
        .read_only = false,
        .data_type = DATA_SIG_CMD,
        .fid = hashing_func ? hashing_func(name): hash_map_fnv1a64(name, strlen(name)),
    });
    return true;
}

static inline module_file* eval_entry(const char *path){
    for (u32 i = 0; i < stack_count(entries); i++){
        module_file *file = stack_get(entries,i);
        if (strcmp(path, file->name.data) == 0) return file;
    }
    return 0;
}

static inline module_file* find_entry(u64 fid){
    for (u32 i = 0; i < stack_count(entries); i++){
        module_file *file = stack_get(entries,i);
        if (file->fid == fid) return file;
    }
    return 0;
}

static inline size_t list_entries(u64 *offset, fs_dir_list_helper *helper){
    for (u64 i = offset ? *offset : 0; i < stack_count(entries); i++){
        if (!dir_list_fill(helper, STACK_GET(module_file,entries,i).name.data)){
            if (offset) *offset = i;
            return dir_buf_size(helper);
        }
    }
    return dir_buf_size(helper);
}

static inline FS_RESULT vfs_open(const char *path, file *fd){
    module_file *mfile = eval_entry(!path || !strlen(path) ? DIR_AS_FILE : path+1);
    if (!mfile) return FS_RESULT_NOTFOUND;
    mfile->references++;
    fd->id = mfile->fid;
    return FS_RESULT_SUCCESS;
}

static inline size_t vfs_read(file *fd, char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->function) return 0;
    return buffer_read(&mfile->file_buffer, buf, size, offset);
}

static inline size_t vfs_write(file *fd, const char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->function) return mfile->function((void*)buf, size);
    return buffer_write_to(&mfile->file_buffer, buf, size, fd->cursor);
}

static inline bool vfs_stat(const char *path, fs_stat *out_stat){
    if (!out_stat) return false;
    path = seek_to(path,'/');
    if (!strlen(path))
        return stat_dir(out_stat);
    module_file *file = eval_entry(path);
    if (file){
        out_stat->size = file->file_buffer.buffer_size;
        out_stat->type = file->entry_type;
        out_stat->data_type = file->data_type;
        return true;
    }
    return false;
}

static inline size_t vfs_readdir(const char *path, void *buf, size_t size, file_offset *offset){
    fs_dir_list_helper helper = create_dir_list_helper(buf, size);
    
    return list_entries(offset, &helper);
}

static inline size_t vfs_list(const char *path, void *buf, size_t size, file_offset *offset){
    return vfs_readdir(path, buf, size, offset);
}