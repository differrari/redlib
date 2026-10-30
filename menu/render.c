#include "render.h"
#include "draw/draw.h"

#ifdef CROSS
extern system_module menu_mod;
#endif
extern int print(const char *fmt, ...);

typedef union {
    struct {
        u8 quit: 1;
        u8 clicked: 1;
        u8 inside: 1;
    };
    u8 info;
} menu_info;

color menu_color;

void set_menu_color(color color){
    menu_color = color;
}

menu_info draw_submenu(draw_ctx *ctx, gpu_point origin, menu_item_t *parent, bool did_click, gpu_point mouse_click){
    int width = 256;
    fb_fill_rect(ctx, origin.x, origin.y, width, parent->num_children*MENU_HEIGHT, menu_color+0x181818);
    fb_outline_rect(ctx, origin.x, origin.y, width, parent->num_children*MENU_HEIGHT, 2, 0x44000000);
    
    menu_item_t *menu = parent->child;
    
    int y = (MENU_HEIGHT-fb_line_height(2))/2;
    int y_top = 0;
    menu_info info;
    while (menu){
        int x = 10;
        gpu_rect bounds = {{origin.x,origin.y + y_top },{width,MENU_HEIGHT}};
        bool clicked_inside = mouse_in_rect(bounds, mouse_click);
        info.inside |= clicked_inside;
        if (did_click && clicked_inside){
            if (menu->is_submenu){
                if (!menu->child) load_menu_level(menu);
                else unload_menu_children(menu);
            } else {                
                print("Opening %s",menu->filepath);
#ifdef CROSS
                menu_mod.transform(menu->filepath, 0, 0);
#else
                if (menu->module)
                    transform_file(menu->module, menu->filepath, 0, 0);
#endif
                info.quit = true;
            }
            info.clicked = true;
        }
        if (menu->child){
            draw_submenu(ctx, (gpu_point){ origin.x + width, origin.y + y_top }, menu, did_click, mouse_click);
            fb_fill_rect(ctx, origin.x + 3, origin.y + y_top + 3, width - 6, MENU_HEIGHT-6, 0x44000000);
        }
        fb_draw_slice(ctx, (string_slice){menu->name,menu->name_len}, origin.x + x, origin.y + y, 2, 0xFFcccccc);
        if (menu->is_submenu) fb_draw_slice(ctx, SLICE(">"), origin.x + width-fb_get_char_size(2)-10, origin.y + y, 2, 0xFFcccccc);
        y += MENU_HEIGHT;
        y_top += MENU_HEIGHT;
        menu = menu->sibling;
    }
    return info;
}

bool mouse_can_click = true;
bool menu_dirty = true;

bool draw_menu(draw_ctx *screen_ctx, gpu_point mouse_pos, bool mouse_pressed){
    if (menu_dirty){
        menu_reset();
        load_menu_level(0);
        menu_dirty = false;
    }
    fb_fill_rect(screen_ctx, 0, 0, screen_ctx->width, MENU_HEIGHT, menu_color+0x111111);
    fb_fill_rect(screen_ctx, 0, MENU_HEIGHT-BORDER_SIZE, screen_ctx->width, BORDER_SIZE, 0x44000000);
    
    menu_item_t *menu = first_menu_item;
    
    int x = 10;
    bool did_click = false, did_click_inside = false;
    gpu_point mouse_click = {};
    
    bool mouse_in = mouse_pos.y < MENU_HEIGHT;
    
    if (mouse_pressed){
        // print("Mouse is pressed");
        if (mouse_can_click)
            did_click = true;
        mouse_can_click = false;
    } else mouse_can_click = true;
    mouse_click = mouse_pos;
    while (menu){
        int y = (MENU_HEIGHT-fb_line_height(2))/2;
        fb_draw_slice(screen_ctx, (string_slice){menu->name,menu->name_len}, x, y, 2, 0xFFcccccc);
        gpu_rect bounds = {{x,y},{(menu->name_len * fb_get_char_size(2)),fb_line_height(2)}};
        if (did_click && mouse_in_rect(bounds, mouse_click)){
            did_click_inside = true;
            if (!menu->child){
                unload_menu_children(0);
                load_menu_level(menu);
            } else unload_menu_children(menu);
        }
        if (menu->child){
            menu_info info = draw_submenu(screen_ctx, (gpu_point){ x, MENU_HEIGHT-BORDER_SIZE }, menu, did_click, mouse_click);
            did_click_inside |= info.clicked;
            mouse_in |= info.inside;
            if (info.quit) {
                unload_menu_children(0);
                return mouse_in;
            }
        }
        x += bounds.size.width + 10;
        menu = menu->sibling;
    }
    
    if (did_click && !did_click_inside) {
        unload_menu_children(0);
    }
    
    return mouse_in;
}