#include "load.h"
#include "data/struct/stack.h"
#include "memory/memory.h"
#include "math/math.h"

arr_stack_t *menu_items = 0;

menu_item_t *first_menu_item = 0, *last_menu_item = 0;

extern int print(const char *fmt, ...);

menu_item_t* menu_add_entry(char *name, menu_item_t *parent){
    print("Added entry %s",name);
    menu_item_t *item = stack_new_item(menu_items,menu_item_t);
    string_slice sl = slice_from_literal(name);
    item->name_len = min(MENU_MAX_NAME,sl.length);
    memcpy(item->name, sl.data, item->name_len);
    item->parent = parent;
    if (last_menu_item){
        if (last_menu_item->parent == parent) last_menu_item->sibling = item;
        else if (parent) parent->child = item;
        if (parent) parent->num_children++;
    } else first_menu_item = item;
    last_menu_item = item;
    return item;
}

string menu_create_pathname(menu_item_t *parent){
    string prec = {};
    if (parent && parent->parent){
        prec = menu_create_pathname(parent->parent);
#ifdef CROSS
    } else prec = string_from_literal("");
#else
    } else prec = string_from_literal("/menu");
#endif
    if (parent){
        string new_p = string_format("%S/%v",prec,(string_slice){parent->name,parent->name_len});
        string_free(prec);
        prec = new_p;
    }
    return prec;
}

#ifdef CROSS
extern system_module menu_mod;
#endif

void load_menu_level(menu_item_t *parent){
    string path = menu_create_pathname(parent);
#ifndef CROSS
    module_root *localfs = get_fs_for_id(menu_proc->permissions.fs_id);
    if (!localfs) return;
#endif
    void *buf = zalloc(0x1000);
    u64 off = 0;
#ifdef CROSS
    size_t s = menu_mod.readdir(path.data, buf, 0x1000, &off);
#else
    size_t s = list_directory_contents(localfs, path.data, buf, 0x1000, &off);
#endif
    if (!s) return;
    string_list *list = (string_list*)buf;
    if (list){
        char* reader = (char*)list->array;
        for (uint32_t i = 0; i < list->count; i++){
            char *file = reader;
            if (*file){
                menu_item_t *item = menu_add_entry(file,parent);
#ifndef CROSS
                item->module = localfs;
#endif
                string_format_buf(item->filepath, MENU_MAX_FILEPATH, "%S/%s",path,file);
                fs_stat stat = {};
#ifdef CROSS
                menu_mod.getstat(item->filepath, &stat);
#else
                get_stat(localfs, item->filepath, &stat);
#endif
                item->is_submenu = stat.type == entry_directory;
            }
            while (*reader) reader++;
            reader++;
        }
    }
    string_free(path);
}

void menu_reset(){
    if (!menu_items) menu_items = stack_create(sizeof(menu_item_t), 64);
    else stack_reset(menu_items);
    
    last_menu_item = 0;
    first_menu_item = 0;
}

size_t calc_children(menu_item_t *parent){
    if (!parent) return 0;
    size_t acc = parent->num_children;
    menu_item_t *child = parent->child;
    while (child){
        acc += calc_children(child);
        child = child->sibling;
    }
    return acc;
}

void unload_menu_children(menu_item_t *parent){
    if (!parent) {
        menu_item_t *child = first_menu_item;
        size_t num_menu_items = 0;
        while (child){
            if (child->parent) break;
            num_menu_items++;
            child->child = 0;
            child->num_children = 0;
            child = child->sibling;
        }
        stack_remove(menu_items,stack_count(menu_items)-num_menu_items);
        last_menu_item = stack_get(menu_items,stack_count(menu_items)-1);
        return;
    }
    size_t num_to_delete = calc_children(parent);
    stack_remove(menu_items, num_to_delete);
    parent->child = 0;
    parent->num_children = 0;
    last_menu_item = stack_get(menu_items,stack_count(menu_items)-1);
}