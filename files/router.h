#pragma once

#include "syscalls/syscalls.h"
#include "data/struct/hashmap.h"
#include "dir_list.h"

typedef struct {
    module_file *file;
    hash_map_t *params;
} path_resolution;

static bool match_param(string_slice slice, string_slice param);

void print_args(void *key, uint64_t keylen, void *val){
    print("Argument %v>%s",(string_slice){key,keylen},val);
}

static path_resolution parse_path(arr_stack_t *entries, const char *path, bool allow_partial, void (*multiple)(path_resolution file)){
    path_resolution res = {
        .file = 0,
        .params = hash_map_create(16)
    };
    size_t arr_size = stack_count(entries);
    for (size_t i = 0; i < arr_size; i++){
        module_file *file = stack_get(entries,i);
        char *pattern = file->name.data;
        string_splitter template_split = make_string_splitter(pattern, '/', false);
        string_splitter path_splitter = make_string_splitter(path, '/', false);
        bool found = true;
        // print("Checking path %s",pattern);
        while (string_splitter_advance(&template_split)){
            if (!string_splitter_advance(&path_splitter)){
                if (!allow_partial) found &= false;
                if (template_split.pointer < template_split.length){
                    found &= false;
                    // print("Failed as the string contains a / after the current partial");
                } else if (template_split.current.length && *template_split.current.data == ':'){
                    hash_map_put(res.params, template_split.current.data + 1, template_split.current.length - 1, string_from_literal("").data);
                    found &= true;
                } 
            } else {
                if (slices_equal(template_split.current, path_splitter.current, true)){
                    found &= true;
                } else if (template_split.current.length && *template_split.current.data == ':' && match_param(path_splitter.current, template_split.current)){
                    found &= true;
                    hash_map_put(res.params, template_split.current.data + 1, template_split.current.length - 1, string_from_slice(path_splitter.current).data);
                } else {
                    found &= false;
                    break;
                }
            }
        }
        if (found && !string_splitter_advance(&path_splitter)){
            // print("Result ?%i",found);
            res.file = file;
            // print("Path %s matched pattern %s",path, pattern);
            // print("***ARGS***");
            // hash_map_for_each(res.params, print_args);
            // print("***____***");
            if (multiple){
                multiple(res);
            } else 
                return res;
        } 
        if (!multiple)
            hash_map_destroy(res.params);
        res.params = hash_map_create(16);
    }
    hash_map_destroy(res.params);
    return res;
}

static fs_dir_list_helper *router_fs_dir_helper;

static void emit_route_contents(path_resolution resolution);

static size_t list_route_directory_contents(arr_stack_t *entries, const char *subdirectory, fs_dir_list_helper *helper){
    if (!helper) return 0;
    router_fs_dir_helper = helper;
    parse_path(entries, subdirectory, true, emit_route_contents);
    return dir_buf_size(helper);
}