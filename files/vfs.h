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

static inline module_file* eval_entry(string_slice path){
    for (u32 i = 0; i < stack_count(entries); i++){
        module_file *file = stack_get(entries,i);
        if (slice_lit_match(path, file->name.data, true)) return file;
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

static inline size_t list_entries(u64 *offset, const char *prefix, fs_dir_list_helper *helper){
    size_t pref_len = strlen(prefix);
    if (pref_len && *prefix == '/'){
        prefix++;
        pref_len--;
    }
    for (u64 i = offset ? *offset : 0; i < stack_count(entries); i++){
        char *entry_name = STACK_GET(module_file,entries,i).name.data;
        if ((size_t)strstart(prefix, entry_name) == pref_len){
            entry_name += pref_len;
            if (strlen(entry_name) && *entry_name == '/') entry_name++;
            if (!strlen(entry_name)) continue;
            if (str_has_char(entry_name, strlen(entry_name), '/') >= 0) continue;
            if (!dir_list_fill(helper, entry_name)){
                if (offset) *offset = i;
                return dir_buf_size(helper);
            }
        }
    }
    return dir_buf_size(helper);
}

static inline string_slice first_path_component(const char *path){
    const char *next = seek_to(path, '/');
    return (string_slice){ .data = (char*)path, .length = next - path - (*(next-1) == '/' ? 1 : 0)};
}

static inline FS_RESULT vfs_open(const char *path, file *fd){
    string_slice first = first_path_component(!path || !strlen(path) ? DIR_AS_FILE : path+1);
    module_file *mfile = eval_entry(first);
    if (!mfile) return FS_RESULT_NOTFOUND;
    if (mfile->actions.open) return mfile->actions.open(path,fd);
    mfile->references++;
    
    if (mfile->alias_info.alias_path.length){
        string fullpath = mfile->alias_info.alias_path;
        string additional = (string){};
        if (mfile->entry_type == entry_directory){
            string_slice next = slice_from_literal(first.data + first.length);
            additional = string_format("%S/%s",fullpath,next);
            fullpath = additional;
        }
        FS_RESULT result = openf(fullpath.data, &mfile->alias_info.alias_fd);
        string_free(additional);
        if (result != FS_RESULT_SUCCESS){
            return result;
        }
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        return FS_RESULT_SUCCESS;
    }
    
    if (mfile->entry_type == entry_directory){
        print("Subdirectory contents not yet supported by vanilla vfs");
        return FS_RESULT_DRIVER_ERROR;
    }
    
    //TODO: bindings for aliases
    fd->id = mfile->fid;
    return FS_RESULT_SUCCESS;
}

static inline size_t vfs_read(file *fd, char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->actions.read) return mfile->actions.read(fd, buf, size, offset);
    if (mfile->alias_info.alias_fd.id){
        size_t res = readf(&mfile->alias_info.alias_fd, buf, size);
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        return res;
    } 
    return buffer_read(&mfile->file_buffer, buf, size, offset);
}

static inline size_t vfs_write(file *fd, const char* buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (mfile->actions.write) return mfile->actions.write(fd, (void*)buf, size, offset);
    if (mfile->alias_info.alias_fd.id){
        size_t res = writef(&mfile->alias_info.alias_fd, buf, size);
        memcpy(fd, &mfile->alias_info.alias_fd, sizeof(file));
        fd->id = mfile->fid;
        return res;
    } 
    return buffer_write_to(&mfile->file_buffer, buf, size, fd->cursor);
}

static inline bool vfs_stat(const char *path, fs_stat *out_stat){
    if (!out_stat) return false;
    string_slice first = first_path_component(!path || !strlen(path) ? DIR_AS_FILE : path+1);
    string_slice next = slice_from_literal(first.data + first.length);
    module_file *file = eval_entry(first);
    if (!file){
        if (next.length == 0)
            return stat_dir(out_stat);
        return 0;
    }
    if (file->actions.getstat) file->actions.getstat(path, out_stat);
    if (file->alias_info.alias_path.length){
        string fullpath = file->alias_info.alias_path;
        string additional = (string){};
        if (file->entry_type == entry_directory){
            string_slice next = slice_from_literal(first.data + first.length);
            additional = string_format("%S/%s",fullpath,next);
            fullpath = additional;
        }
        size_t res = statf(fullpath.data, out_stat);
        string_free(additional);
        return res;
    } 
    out_stat->size = file->file_buffer.buffer_size;
    out_stat->type = file->entry_type;
    out_stat->data_type = file->data_type;
    return true;
}

static inline void vfs_close(file *fd){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return;
    if (mfile->actions.close){
        mfile->actions.close(fd);
        return;
    }
    if (mfile->alias_info.alias_fd.id){
        return closef(&mfile->alias_info.alias_fd);
    }
} 

static inline size_t vfs_readdir(const char *path, void *buf, size_t size, file_offset *offset){
    fs_dir_list_helper helper = create_dir_list_helper(buf, size);
    string_slice first = first_path_component(!path || !strlen(path) ? DIR_AS_FILE : path+1);
    if (*(path + first.length) == 0)
        return list_entries(offset, (char*)(path + first.length), &helper);
    module_file *file = eval_entry(first);
    if (!file) return 0;
    if (file->actions.readdir) return file->actions.readdir(path, buf, size, offset);
    if (file->entry_type != entry_directory){
        print("%s is not a directory",path);
        return 0;
    }
    if (file->alias_info.alias_path.data){
        string fullpath = file->alias_info.alias_path;
        string additional = (string){};
        if (file->entry_type == entry_directory){
            string_slice next = slice_from_literal(first.data + first.length);
            additional = string_format("%S/%s",fullpath,next);
            fullpath = additional;
        }
        size_t res = dir_list(fullpath.data, buf, size, offset);
        string_free(additional);
        return res;
    }
    print("Subdirectory listing not yet supported by vanilla VFS");
    return 0;
}

static inline size_t vfs_list(const char *path, void *buf, size_t size, file_offset *offset){
    return vfs_readdir(path, buf, size, offset);
}