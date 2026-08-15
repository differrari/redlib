#ifdef CROSS

#include "string/string.h"
#include "ui/draw/draw.h"
#include "alloc/allocate.h"
#include "raylib.h"
#include "keyboard_input.h"
#include "mouse_input.h"
#include "keycode_convert.h"
#include "environment/window_info.h"
#include "syscalls/syscalls.h"

extern void free(void*ptr);

#define CONVERT_COLOR(color) ((color & 0xFF00FF00) | ((color & 0xFF) << 16) | ((color >> 16) & 0xFF))

Texture2D _screen_tex = {};

void begin_drawing(draw_ctx *ctx){

}

void destroy_draw_ctx(draw_ctx *ctx){
    CloseWindow();
}

void commit_draw_ctx(draw_ctx *ctx){
    BeginDrawing();
#if _WIN32
    for (int i = 0; i < ctx->width * ctx->height; i++){
        ctx->fb[i] = CONVERT_COLOR(ctx->fb[i]);
    }
#endif
    UpdateTexture(_screen_tex, ctx->fb);
    DrawTexture(_screen_tex,0,0,WHITE);
    EndDrawing();
}

void resize_draw_ctx(draw_ctx *ctx, uint32_t width, uint32_t height){
    release(ctx->fb);
    UnloadTexture(_screen_tex);
    ctx->width = width;
    ctx->height = height;
    ctx->fb = zalloc(width*height*sizeof(color));
    ctx->stride = 4 * width;
    _screen_tex = LoadTextureFromImage((Image){
       .data = ctx->fb,
       .width = width,
       .height = height,
       .mipmaps = 1,
#if _WIN32
       .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 
#else
        .format = PIXELFORMAT_UNCOMPRESSED_B8G8R8A8 
#endif
    });
    SetWindowSize(width, height);
}

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ? ctx->width : 600;
    uint32_t h = ctx->height ? ctx->height : 300;
    ctx->fb = zalloc(w*h*sizeof(color));
    ctx->width = w;
    ctx->height = h;
    ctx->stride = sizeof(color) * w;

    InitWindow(w,h,"RedXLib");
    
    SetWindowMonitor(0);
    
    if (FileExists("icon.png")){
        SetWindowIcon(LoadImage("icon.png"));
    }

    _screen_tex = LoadTextureFromImage((Image){
        .data = ctx->fb,
        .width = w,
        .height = h,
        .mipmaps = 1,
#if _WIN32
       .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 
#else
        .format = PIXELFORMAT_UNCOMPRESSED_B8G8R8A8 
#endif
    });
    SetExitKey(0);

    if (TextFindIndex(GetApplicationDirectory(), ".app") > -1) 
        ChangeDirectory(TextFormat("%s/..",GetApplicationDirectory()));
    
}

#define is_mod(key) (key >= 340/*LSHIFT */ && key <= 347/*RMETA */)

bool keypresses[512] = {};
u64 keypress_repeat[512] = {};
bool read_event(kbd_event *event){
    //TODO: modifiers
    int key = 0;
    key = GetKeyPressed();
    bool mod = is_mod(key);
    uint64_t now = get_time();
    if (key){
        if (mod) event->type = MOD_PRESS;
        else {
            if (IsKeyPressed(key)) event->type = KEY_PRESS;
            else if (IsKeyDown(key)) event->type = KEY_CONTINUE;
        }
        if (mod)
            event->modifier = cross_to_redacted[key];
        else 
            event->key = cross_to_redacted[key];
        keypress_repeat[key] = now + 500;
        // print("P: KEY %i(%i). MOD %i",cross_to_redacted[key],key,mod);
        keypresses[key] = true;
        return true;
    }
    for (int i = 0; i < 512; i++)
        if (keypresses[i]){
            bool mrelease = is_mod(i);
            if (IsKeyUp(i)){
                event->type = mrelease ? MOD_RELEASE : KEY_RELEASE;
                if (mrelease)
                    event->modifier = cross_to_redacted[i];
                else
                    event->key = cross_to_redacted[i];
                // print("R: KEY %i(%i). MOD %i",cross_to_redacted[i],i,mod);
                keypresses[i] = false;
                keypress_repeat[i] = 0;
                return true;
            } else if (!mrelease && keypress_repeat[i] > 0 && keypress_repeat[i] <= now) {
                event->type = KEY_CONTINUE;
                event->key = cross_to_redacted[i];
                keypress_repeat[i] = now + 30;
                return true;
            }
        }
    return false;
}

int old_x = 0;
int old_y = 0;

void get_mouse_status(mouse_data *in){
    in->raw.scroll = ((int)GetMouseWheelMove() & 0xff);
    in->raw.buttons = 0;
    for (int i = 0; i < 3; i++)
        in->raw.buttons |= (IsMouseButtonDown(i) & 1) << i;
    int x_pos = GetMouseX();
    int y_pos = GetMouseY();
    in->raw.x = x_pos - old_x;
    in->raw.y = y_pos - old_y;
    in->position.x = x_pos;
    in->position.y = y_pos;
    old_x = x_pos;
    old_y = y_pos;
}

bool should_close_ctx(){
    return WindowShouldClose();
}

void env_set_window_info(window_info_t *info){
    string s = string_from_literal_length(info->name, info->name_length);
    SetWindowTitle(s.data);
    string_free(s);
}

void clipboard_copy(void *buf, size_t size, data_signature type){
    if (type != DATA_SIG_UNKNOWN && type != DATA_SIG_TEXT) return;
    SetClipboardText(buf);
}

void* clipboard_paste(data_signature expected_sig, size_t *out_size){
    if (expected_sig != DATA_SIG_UNKNOWN && expected_sig != DATA_SIG_TEXT) return 0;
    return (void*)GetClipboardText();
}

#endif