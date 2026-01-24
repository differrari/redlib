#ifdef CROSS

#include "ui/draw/draw.h"
#include "keyboard_input.h"
#include "mouse_input.h"
#include "raylib.h"
#include "math/math.h"
#include "syscalls/syscalls.h"
#include "keycode_convert.h"
#include "std/memory.h"
#define __USE_POSIX199309
#include <time.h>
#include <stdio.h>

extern void free(void *ptr);
extern int puts(const char *str);
extern void exit(int status);

void printl(const char *str){
    puts(str);
}

char log_buf[1024];

char *read_full_file(const char *path, size_t *out_size){
    if (strstart_case(path,"/resources",true) == 10)
        path++;
    FILE *fd = fopen(path,"r");
    if (!fd) return 0;
    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    if (out_size) *out_size = fsize;
    rewind(fd);

    char *fcontent = (char*)zalloc(sizeof(char) * (fsize+1));
    fread(fcontent, fsize, 1, fd);

    fclose(fd);
    
    return fcontent;
}

bool write_full_file(const char *path, void* buf, size_t size){
    FILE *fd = fopen(path,"w");
    
    size_t res = fwrite(buf, size, 1, fd);
    
    fclose(fd);
    
    return res > 0;
}

void free_sized(void* ptr, size_t size)
{
    free(ptr);
}

void halt(int reason){
    exit(reason);
}

bool read_key(keypress *kp){
    return false;
}

uint64_t get_time(){
    return floor(GetTime() * 1000);
}

bool keypresses[512];

bool read_event(kbd_event *event){
    //TODO: modifiers
    int key;
    // do {
        key = GetKeyPressed();
        if (key){
            event->type = KEY_PRESS;
            event->key = raylib_to_redacted[key];
            keypresses[key] = true;
            return true;
        }
    // } while (key);
    for (int i = 0; i < 512; i++)
        if (keypresses[i] && IsKeyUp(i)){
            event->type = KEY_RELEASE;
            event->key = raylib_to_redacted[i];
            keypresses[i] = false;
            return true;
        }
    return false;
}

void get_mouse_status(mouse_data *in){
    in->raw.x = (int8_t)(GetMouseDelta().x * 128);
    in->raw.y = (int8_t)(GetMouseDelta().y * 128);
    in->raw.scroll = ((int)GetMouseWheelMove() & 0xFF);
    in->raw.buttons = 0;
    for (int i = 0; i < 3; i++)
        in->raw.buttons |= (IsMouseButtonDown(i) & 1) << i;
    in->position.x = GetMouseX();
    in->position.y = GetMouseY();
}

FS_RESULT openf(const char* path, file* descriptor){
    printf("Opening file %s",path);
    return FS_RESULT_DRIVER_ERROR;
}

size_t readf(file *descriptor, char* buf, size_t size){
    return 0;
}

size_t writef(file *descriptor, const char* buf, size_t size){
    return 0;
}

void closef(file *descriptor){

}

void* zalloc(size_t size){
    void *p = malloc(size);
    memset(p,0,size);
    return p;
}

void msleep(uint64_t msec){
    struct timespec rqtp;
    rqtp.tv_sec = msec / 1000;
    rqtp.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&rqtp, NULL);
}

#endif