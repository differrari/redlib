#pragma once

#include "files/vfs.h"

system_module menu_mod = {
    .name = "demo menu",
    .mount = "menu",
    .version = VERSION_NUM(0, 1, 0, 0),
    .open = vfs_open,
    .read = vfs_read,
    .write = vfs_write,
    .getstat = vfs_stat,
    .readdir = vfs_readdir,
    .transform = vfs_transform,
};

static inline string make_menu_entry(const char *parent, const char *name, fs_entry_type ent_type, file_transform_fn trigger){
    if (parent && *parent == '/') parent++;
    if (name && *name == '/') name++;
    if (!entries) entries = stack_create(sizeof(module_file),32);
    module_file *file = stack_new_item(entries,module_file);
    *file = (module_file){
        .backing_type = backing_virtual,
        .entry_type = ent_type,
        .file_buffer = (buffer){},
        .references = 0,
        .read_only = true,
        .data_type = 0,
        .actions = {
            .transform = trigger
        },
        .fid = hash_filename(name),
        .serial = hash_filename(name),
    };
    if (parent) file->name = string_format("%s/%s",parent, name);
    else file->name = string_from_literal(name);
    return file->name;
}

#define MAKE_MENU(contents) void setup_menu(){ char *parent_dir = 0; contents; };

#define MENU_SUBMENU(name, items)\
    {\
        char *local_dir = make_menu_entry(parent_dir, name, entry_directory, 0).data;\
        char *old_dir = parent_dir;\
        parent_dir = local_dir;\
        items;\
        parent_dir = old_dir;\
    }
    
#define MENU_ITEM(name, function)\
    make_menu_entry(parent_dir, name, entry_file, function);
    
extern void setup_menu();
    
void menu_init(){
    setup_menu();
    
    load_fsmodule(&menu_mod, false);
    
    swritef("/environment/menu", 0, 0, false);
}