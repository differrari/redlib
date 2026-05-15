#define SKIP_ROUTER_FNS
#include "vfs.h"
#include "router.h"

static u64 static_entries = 0;

static arr_stack_t *data;

static bool match_param(string_slice slice, string_slice param){
    if (slice_lit_match(param, ":id", true)){
        for (size_t i = 0; i < slice.length; i++) if (!is_digit(slice.data[i])) return false;
        return true;
    }
    if (slice_lit_match(param, ":sort", true)){
        return slice_lit_match(slice, "latest", true);
    }
    return false;
}

static inline module_file* stackfs_resolve_path(const char *path){
    if (path && *path == '/') path = seek_to(path,'/');
    if (!path || !strlen(path) || strcmp(DIR_AS_FILE,path) == 0){
        string fmt = string_format("%i", stack_count(entries)-static_entries-1);
        return eval_entry(slice_from_string(fmt));
    } else {
        return eval_entry(slice_from_literal(path));
    }
}

static inline bool stackfs_init(){
    static_entries += make_entry(DIR_AS_FILE, backing_virtual, entry_file, 0, (buffer){}) != 0;
    static_entries += make_entry(":sort", backing_virtual, entry_directory, 0, (buffer){}) != 0;
    static_entries += make_entry(":sort/:id", backing_virtual, entry_file, 0, (buffer){}) != 0;
    static_entries += make_entry(":id", backing_virtual, entry_file, 0, (buffer){}) != 0;
    return true;
}

static inline size_t stackfs_read(file *fd, char *buf, size_t size, file_offset offset){
    if (fd->id < stack_count(data)){
        buffer *b = stack_get(data,fd->id);
        return buffer_read(b, buf, size, fd->cursor);
    }
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        if (!data || !stack_count(data)) return 0;
        buffer *file_buf = stack_get(data, stack_count(data)-1);
        size_t read = buffer_read(file_buf, buf, size, fd->cursor);
        return read;
    }
    return buffer_read(&mfile->file_buffer, buf, size, fd->cursor);
}

static inline size_t stackfs_write(file *fd, const char *buf, size_t size, file_offset offset){
    if (fd->id < stack_count(data)){
        buffer *b = stack_get(data,fd->id);
        return buffer_write_to(b, buf, size, fd->cursor);
    }
    module_file *mfile = find_entry(fd->id);
    if (!mfile) return 0;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        if (!data) return 0;
        buffer new_buf = buffer_create(size, buffer_can_grow);
        new_buf.data_type = fd->data_type;
        return buffer_write_to(stack_get(data, stack_push(data,&new_buf)), buf, size, fd->cursor);
    }
    
    return 0;
}

static inline FS_RESULT stackfs_open_root(const char *path, file *fd){
    string_slice first = first_path_component(!path || strcmp(DIR_AS_FILE,path) == 0 || !strlen(path) ? DIR_AS_FILE : path+1);
    module_file *mfile = eval_entry(first);
    if (!mfile) return FS_RESULT_NOTFOUND;
    mfile->references++;
    fd->id = mfile->fid;
    if (strcmp(mfile->name.data, DIR_AS_FILE) == 0){
        if (!data) data = stack_create(sizeof(buffer), 32);
        else if (stack_count(data)) {
            buffer *b = stack_get(data, stack_count(data)-1);
            fd->data_type = 0;
            fd->size = b->buffer_size;
        }
    } else {
        fd->data_type = mfile->data_type;
        fd->size = mfile->file_buffer.buffer_size;
    }
    return FS_RESULT_SUCCESS;
}

static inline string reverse_filename(const char *path){
    i64 parsed_name = parse_int64(path, strlen(path));
    parsed_name = stack_count(entries)-static_entries-parsed_name-1;
    return string_format("/%i",parsed_name);
}

static inline FS_RESULT stackfs_open(const char *path, file *fd){
    if (!path || !strlen(path)) path = DIR_AS_FILE;
    path_resolution resolution = parse_path(entries, path, false, 0);
    if (!resolution.file) return FS_RESULT_NOTFOUND;
    if (!hash_map_size(resolution.params)){
        hash_map_destroy(resolution.params);
        if (strcmp(resolution.file->name.data, DIR_AS_FILE) == 0){
            if (!data) data = stack_create(sizeof(buffer), 32);
            if (stack_count(data)) {
                buffer *b = stack_get(data, stack_count(data)-1);
                fd->data_type = b->data_type;
                fd->size = b->buffer_size;
            }
            fd->id = resolution.file->fid;
            return FS_RESULT_SUCCESS;
        }
        return FS_RESULT_NOTFOUND;
    }
    bool latest = hash_map_get_dictionary(resolution.params, "sort") != 0;
    string id_str = string_from_literal(hash_map_get_dictionary(resolution.params, "id"));
    hash_map_destroy(resolution.params);
    if (!id_str.length){
        return FS_RESULT_NOTFOUND;
    } 
    i64 id = parse_int64(id_str.data, id_str.length);
    string_free(id_str);
    if (!data || !stack_count(data)) return false;
    if (id >= (i64)stack_count(data)) return false;
    if (latest) id = stack_count(data) - id - 1;
    fd->id = id;
    buffer *b = stack_get(data, id);
    fd->size = b->buffer_size;
    fd->data_type = b->data_type;
    fd->cursor = 0;
    return FS_RESULT_SUCCESS;
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

size_t list_argument(string_slice prefix, string_slice slice, fs_dir_list_helper *helper){
    if (slice_lit_match(slice, ":id", true)){
        size_t count = stack_count(data);
        for (u64 i = 0; i < count; i++){//TODO: i should start at offset, but offset is no longer just for files
            string entry_name = string_format("%i",slice_lit_match(prefix, "latest", true) ? count - i - 1 : i);
            if (!dir_list_fill(helper, entry_name.data)){
                // if (offset) *offset = i;
                return dir_buf_size(helper);
            }
            string_free(entry_name);
        }
    } else if (slice_lit_match(slice, ":sort", true)){
        if (!dir_list_fill(helper, "latest")){
            // if (offset) *offset = i;
            return dir_buf_size(helper);
        }
    }
        
    return 0;
}

bool argument_match(const char *value, const char *temp){
    if (strstart(temp, ":sort") == 5){
        return strcmp(value, "latest") == 0;
    }
    return false;
}

static file_offset *out_offset;
static file_offset current_offset;

static void emit_route_contents(path_resolution resolution){
    if (!out_offset || *out_offset <= current_offset){
        if (!hash_map_size(resolution.params)){
            if (!dir_list_fill(router_fs_dir_helper, resolution.file->name.data)){
                if (out_offset) *out_offset = current_offset;
            }
        } else {
            char *latest_str = hash_map_get_dictionary(resolution.params, "sort");
            if (latest_str && !strlen(latest_str)){
                if (!dir_list_fill(router_fs_dir_helper, "latest")){
                    if (out_offset) *out_offset = current_offset;
                }
            } else if (data && hash_map_get_dictionary(resolution.params, "id")) {
                bool latest = latest_str != 0;
                size_t data_size = stack_count(data);
                for (size_t i = 0; i < data_size; i++){
                    string s = string_format("%i",latest ? data_size-i-1 : i);
                    if (!dir_list_fill(router_fs_dir_helper, s.data)){
                        string_free(s);
                        if (out_offset) *out_offset = current_offset;
                    }
                    string_free(s);
                }
            }
        }
    }
    current_offset++;
}

static inline size_t stackfs_readdir(const char *path, void *buf, size_t size, file_offset *offset){
    fs_dir_list_helper helper = create_dir_list_helper(buf, size);
    out_offset = offset;
    return list_route_directory_contents(entries, path, &helper);
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

static inline bool stackfs_stat_data(i64 id, fs_stat *out_stat){
    buffer *b = stack_get(data, id);
    if (!b) return false;
    out_stat->data_type = b->data_type;
    out_stat->type = entry_file;
    out_stat->size = b->buffer_size;
    return true;
}

static inline bool stackfs_stat(const char *path, fs_stat *out_stat){
    if (strlen(path) && *path == '/') path++;
    if (!strlen(path)){
        stat_dir(out_stat);
        return true;
    }
    path_resolution resolution = parse_path(entries, path, false, 0);
    if (!resolution.file) return false;
    if (!hash_map_size(resolution.params)){
        hash_map_destroy(resolution.params);
        if (strcmp(resolution.file->name.data, DIR_AS_FILE) == 0 && stack_count(data)){
            return stackfs_stat_data(stack_count(data)-1, out_stat);
        }
        out_stat->type = resolution.file->entry_type;
        out_stat->size = resolution.file->file_buffer.buffer_size;
        out_stat->data_type = resolution.file->data_type;
        return true;
    }
    bool latest = hash_map_get_dictionary(resolution.params, "sort") != 0;
    string id_str = string_from_literal(hash_map_get_dictionary(resolution.params, "id"));
    hash_map_destroy(resolution.params);
    if (!id_str.length){
        if (latest){
            stat_dir(out_stat);
            return true;
        }
        return false;
    } 
    i64 id = parse_int64(id_str.data, id_str.length);
    string_free(id_str);
    if (!data || !stack_count(data)) return false;
    if (id >= (i64)stack_count(data)) return false;
    if (latest) id = stack_count(data) - id - 1;
    return stackfs_stat_data(id, out_stat);
}