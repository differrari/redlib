#ifdef CROSS

#include "ui/draw/draw.h"
#include "keyboard_input.h"
#include "mouse_input.h"
#include "math/math.h"
#include "syscalls/syscalls.h"
#include "std/memory.h"
#define __USE_POSIX199309
#include <time.h>
#include <stdio.h>
#include "alloc/allocate.h"

extern void free(void *ptr);
extern int puts(const char *str);
extern void exit(int status);

#define ALIGN 0x1000
void* page_alloc(size_t size) {
    void *mem = malloc(size+ALIGN+sizeof(void*));
    void **ptr = (void**)((uintptr_t)(mem+ALIGN+sizeof(void*)) & ~(ALIGN-1));
    ptr[-1] = mem;
    memset(ptr,0,size);
    return ptr;
}

void page_free(void* ptr){
    free(((void**)ptr)[-1]);
}

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

__attribute__((noreturn)) void halt(int reason){
    exit(reason);
}

bool read_key(keypress *kp){
    return false;
}

u64 get_time(){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)(ts.tv_sec * 1000LL) + (ts.tv_nsec/1000000LL);
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

void msleep(uint64_t msec){
    struct timespec rqtp;
    rqtp.tv_sec = msec / 1000;
    rqtp.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&rqtp, NULL);
}

void in_case_of_js_break_glass(){
    print("Shame on you\r\n\
Don't ever do that again\r\n\
....................../´¯/) \r\n\
....................,/¯../ \r\n\
.................../..../ \r\n\
............./´¯/'...'/´¯¯`·¸ \r\n\
........../'/.../..../......./¨¯\\\r\n\
........('(...´...´.... ¯~/'...') \r\n\
.........\\.................'...../\r\n\
..........''...\\.......... _.·´\r\n\
............\\..............(\r\n\
..............\\.............\\...");
    halt(-1);
}

#endif