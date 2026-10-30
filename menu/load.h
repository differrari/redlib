#pragma once

#include "types.h"
#include "files/module_loader.h"
#include "data/struct/stack.h"

#define MENU_MAX_NAME 64
#define MENU_MAX_TOOLTIP 256
#define MENU_MAX_FILEPATH 256

typedef struct menu_item_t {
    u8 name_len;
    char name[MENU_MAX_NAME];
    u8 tooltip_len;
    char tooltip[MENU_MAX_TOOLTIP];
    u8 filepath_len;
    char filepath[MENU_MAX_TOOLTIP];
    size_t num_children;
    bool is_submenu;
    module_root *module;
    struct menu_item_t *parent;
    struct menu_item_t *child;
    struct menu_item_t *sibling;
} menu_item_t;

menu_item_t* menu_add_entry(char *name, menu_item_t *parent);
string menu_create_pathname(menu_item_t *parent);
void load_menu_level(menu_item_t *parent);
void unload_menu_children(menu_item_t *parent);
void menu_reset();

extern arr_stack_t *menu_items;
extern menu_item_t *first_menu_item, *last_menu_item;