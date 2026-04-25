#pragma once

#include "data/struct/stack.h"
#include "files/dir_list.h"
#include "data/struct/hashmap.h"
#include "data_signatures.h"
#include "memory/memory.h"
#include "data/struct/hashmap.h"
#include "syscalls/syscalls.h"

#define DIR_AS_FILE "#"

static arr_stack_t *entries;

static hash_map_t *alias_map;

static u64 (*hashing_func)(const char *path);

static inline u64 hash_filename(const char *name){
    return hashing_func ? hashing_func(name): hash_map_fnv1a64(name, strlen(name));
}

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
        .fid = hash_filename(name),
        .serial = hash_filename(name),
    });
    return stack_get(entries, stack_count(entries)-1);
}

static inline bool make_complex_entry(const char *name, fs_backing_type back_type, fs_entry_type ent_type, data_signature data_type, file_actions actions, string alias){
    if (!entries) entries = stack_create(sizeof(module_file),32);
    stack_push(entries,&(module_file){
        .name = string_from_literal(name),
        .alias_info = {
            .alias_path = alias,
        },
        .backing_type = back_type,
        .entry_type = ent_type,
        .actions = actions,
        .references = 0,
        .read_only = false,
        .data_type = data_type,
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

static inline file* find_alias(u64 fid){
    if (!alias_map) return 0;
    return hash_map_get(alias_map, &fid, sizeof(u64));
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
    
    if (mfile->alias_info.alias_path.length){
        FS_RESULT result = openf(mfile->alias_info.alias_path.data, &mfile->alias_info.alias_fd);
        if (result != FS_RESULT_SUCCESS){
            return result;
        }
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        //TODO: where do transforms fit in this
        return FS_RESULT_SUCCESS;
    }
    
    if (mfile->actions.open) mfile->actions.open(path,fd);
    //TODO: bindings for aliases
    fd->id = mfile->fid;
    return FS_RESULT_SUCCESS;
}

static inline size_t vfs_read(file *fd, char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->alias_info.alias_fd.id){
        size_t res = readf(&mfile->alias_info.alias_fd, buf, size);
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        return res;
    } 
    if (mfile->actions.read) return mfile->actions.read(fd, buf, size, offset);
    return buffer_read(&mfile->file_buffer, buf, size, offset);
}

static inline size_t vfs_write(file *fd, const char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->alias_info.alias_fd.id){
        size_t res = writef(&mfile->alias_info.alias_fd, buf, size);
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        return res;
    } 
    if (mfile->actions.write) return mfile->actions.write(fd, (void*)buf, size, offset);
    return buffer_write_to(&mfile->file_buffer, buf, size, fd->cursor);
}

static inline bool vfs_stat(const char *path, fs_stat *out_stat){
    if (!out_stat) return false;
    path = seek_to(path,'/');
    if (!strlen(path))
        return stat_dir(out_stat);
    module_file *file = eval_entry(path);
    if (!file) return false;
    if (file->alias_info.alias_path.length){
        return statf(file->alias_info.alias_path.data, out_stat);
    } 
    if (file->actions.getstat) file->actions.getstat(path, out_stat);
    out_stat->size = file->file_buffer.buffer_size;
    out_stat->type = file->entry_type;
    out_stat->data_type = file->data_type;
    return true;
}

static inline void vfs_close(file *fd){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return;
    if (mfile->alias_info.alias_fd.id){
        return closef(&mfile->alias_info.alias_fd);
    }
    if (mfile->actions.close){
        mfile->actions.close(fd);
        return;
    }
} 

static inline size_t vfs_readdir(const char *path, void *buf, size_t size, file_offset *offset){
    fs_dir_list_helper helper = create_dir_list_helper(buf, size);
    
    return list_entries(offset, &helper);
}

static inline size_t vfs_list(const char *path, void *buf, size_t size, file_offset *offset){
    return vfs_readdir(path, buf, size, offset);
}