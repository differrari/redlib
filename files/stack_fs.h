#pragma once

#include "vfs.h"

static u64 static_entries = 0;

static inline module_file* stackfs_resolve_path(const char *path){
    if (path && *path == '/') path = seek_to(path,'/');
    if (!path || !strlen(path) || strcmp(DIR_AS_FILE,path) == 0){
        string fmt = string_format("%i", stack_count(entries)-static_entries-1);
        return eval_entry(fmt.data);
    } else {
        return eval_entry(path);
    }
}

static inline bool stackfs_init(){
    static_entries += make_entry(DIR_AS_FILE, backing_virtual, entry_file, 0, (buffer){}) != 0;
    static_entries += make_entry("latest", backing_virtual, entry_directory, 0, (buffer){}) != 0;
    return true;
}

static inline size_t stackfs_read(file *fd, char *buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        string fmt = string_format("%i", stack_count(entries)-static_entries-1);
        module_file *file = eval_entry(fmt.data);
        if (!file) return 0;
        return buffer_read(&file->file_buffer, buf, size, fd->cursor);
    }
    return buffer_read(&mfile->file_buffer, buf, size, fd->cursor);
}

static inline size_t stackfs_write(file *fd, const char *buf, size_t size, file_offset offset){
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        string fmt = string_format("%i", stack_count(entries)-static_entries);
        module_file *file = make_entry(fmt.data, backing_virtual, entry_file, fd->data_type, buffer_create(size, buffer_can_grow));
        if (!file) return 0;
        size_t written = buffer_write_to(&file->file_buffer, buf, size, fd->cursor);
        return written;
    }
    
    return vfs_write(fd, buf, size, offset);
}

static inline FS_RESULT stackfs_open_root(const char *path, file *fd){
    module_file *mfile = eval_entry(!path || strcmp(DIR_AS_FILE,path) == 0 || !strlen(path) ? DIR_AS_FILE : path+1);
    if (!mfile) return FS_RESULT_NOTFOUND;
    mfile->references++;
    fd->id = mfile->fid;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        string fmt = string_format("%i", stack_count(entries)-static_entries-1);
        module_file *file = eval_entry(fmt.data);
        if (file)
            fd->size = file->file_buffer.buffer_size;
    } else 
        fd->size = mfile->file_buffer.buffer_size;
    return FS_RESULT_SUCCESS;
}

static inline string reverse_filename(const char *path){
    i64 parsed_name = parse_int64(path, strlen(path));
    parsed_name = stack_count(entries)-static_entries-parsed_name-1;
    return string_format("/%i",parsed_name);
}

static inline FS_RESULT stackfs_open(const char *path, file *fd){
    if (strstart("/latest", path) == sizeof("/latest")-1){
        path += sizeof("/latest");
        if (*path == 0) return FS_RESULT_NOTFOUND;
        string new_name = reverse_filename(path);
        FS_RESULT res = stackfs_open_root(new_name.data, fd);
        string_free(new_name);
        return res;
    }
    return stackfs_open_root(path, fd);
}

static inline size_t list_reverse_entries(u64 *offset, fs_dir_list_helper *helper){
    if (offset && *offset < static_entries) *offset = static_entries;
    for (u64 i = offset ? *offset : static_entries; i < stack_count(entries); i++){
        string old_name = STACK_GET(module_file,entries,i).name;
        string new_name = reverse_filename(old_name.data);
        if (!dir_list_fill(helper, new_name.length ? new_name.data+1 : 0)){
            string_free(new_name);
            if (offset) *offset = i;
            return dir_buf_size(helper);
        }
        string_free(new_name);
    }
    return dir_buf_size(helper);
}

static inline size_t stackfs_readdir(const char *path, void *buf, size_t size, file_offset *offset){
    fs_dir_list_helper helper = create_dir_list_helper(buf, size);
    
    if (!strlen(path)){
        return list_entries(offset, &helper);
    }
    
    if (strcmp(path,"/latest") == 0){
        return list_reverse_entries(offset, &helper);
    }
    
    return 0;
}

static inline bool stackfs_stat_root(const char *path, fs_stat *out_stat){
    module_file *mfile = stackfs_resolve_path(path);
    if (mfile){
        out_stat->size = mfile->file_buffer.buffer_size;
        out_stat->type = mfile->entry_type;
        out_stat->data_type = mfile->data_type;
        return true;
    }
    return false;
}

static inline bool stackfs_stat(const char *path, fs_stat *out_stat){
    if (strstart("/latest", path) == sizeof("/latest")-1){
        path += sizeof("/latest");
        if (*path == 0) return stat_dir(out_stat);
        string new_name = reverse_filename(path);
        bool res = stackfs_stat_root(new_name.data, out_stat);
        string_free(new_name);
        return res;
    }
    path = seek_to(path,'/');
    if (!strlen(path))
        return stat_dir(out_stat);
    return stackfs_stat_root(path, out_stat);
}